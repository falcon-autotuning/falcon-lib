open Lwt.Infix
open Acu_types

let parse_args () =
  if Array.length Sys.argv = 3 && Sys.argv.(1) = "--list" then
    let package_paths = if Sys.argv.(2) = "" then [] else String.split_on_char ',' Sys.argv.(2) in
    `List package_paths
  else if Array.length Sys.argv < 8 then
    (Printf.eprintf "Usage: %s <hash> <unit_type> <request> <globals> <instance_variables> <message_config> <package_paths>\n" Sys.argv.(0);
     Printf.eprintf "       %s --list <package_paths>\n" Sys.argv.(0);
     exit 1)
  else
    let hash = Sys.argv.(1) in
    let unit_type = Sys.argv.(2) in
    let request = Yojson.Safe.from_string Sys.argv.(3) in
    let globals = Yojson.Safe.from_string Sys.argv.(4) in
    let instances = Yojson.Safe.from_string Sys.argv.(5) in
    let message_config = Message_handler.ConfigIO.from_json (Yojson.Safe.from_string Sys.argv.(6)) in
    let package_paths = if Sys.argv.(7) = "" then [] else String.split_on_char ',' Sys.argv.(7) in
    `Run (hash, unit_type, request, globals, instances, message_config, package_paths)

let create_unit = Acu_registry.create_unit

let run () =
  match parse_args () with
  | `List package_paths ->
      List.iter Acu_registry.load_all_from_directory package_paths;
      let types = Acu_registry.list_registered_types () in
      Printf.printf "%s\n" (String.concat "," types);
      Lwt.return_unit
  | `Run (hash, unit_type, request_json, _globals, instance_json, message_config, package_paths) ->
      (* Load plugins from package paths *)
      List.iter Acu_registry.load_all_from_directory package_paths;

      Message_handler.MessageHandler.connect message_config.url >>= fun message_handler ->
      let device_config = Falcon_core.Config.from_json_string (Falcon_core.String.wrap (Yojson.Safe.to_string _globals)) in
      
      create_unit unit_type hash request_json instance_json _globals message_handler message_config device_config >>= fun (module M : ACU_INSTANCE) ->
      
      M.main () >>= fun () ->
      
      let res = M.get_response () in
      let response = `Assoc [
        ("hash", `String hash);
        ("response", `String res.message);
        ("success", `Bool res.success);
        ("products", `Assoc res.products);
      ] in
      print_endline (Yojson.Safe.to_string response);
      
      Message_handler.MessageHandler.close message_handler
