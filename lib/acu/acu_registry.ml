
open Lwt.Infix
open Acu_types

type acu_module = (module ACU_INSTANCE)

let registry : (string, acu_module) Hashtbl.t = Hashtbl.create 16

let register (unit_type : string) (m : acu_module) =
  Printf.eprintf "[INFO] Registering ACU module type: %s\n" unit_type;
  Hashtbl.replace registry unit_type m

let list_registered_types () =
  let types = Hashtbl.fold (fun key _ acc -> key :: acc) registry [] in
  Printf.eprintf "[DEBUG] Registered types count: %d\n" (List.length types);
  types

let find (unit_type : string) =
  Hashtbl.find_opt registry unit_type

let create_unit unit_type hash request instance_vars _globals message_handler message_config device_config =
  match find unit_type with
  | Some (module M) -> 
      M.initialize ~hash ~request ~instance_vars ~handler:message_handler ~config_io:message_config ~device_config () >>= fun () ->
      Lwt.return (module M : ACU_INSTANCE)
  | None -> 
      Lwt.fail_with (Printf.sprintf "Unknown unit type: %s" unit_type)

let load_plugin (path : string) =
  Printf.eprintf "[INFO] Loading plugin: %s\n" path;
  try
    Dynlink.loadfile path;
    Printf.eprintf "[INFO] Successfully loaded plugin %s\n" path
  with
  | Dynlink.Error err ->
      Printf.eprintf "[ERROR] Failed to load plugin %s: %s\n" path (Dynlink.error_message err)
  | exn ->
      Printf.eprintf "[ERROR] Unexpected error loading plugin %s: %s\n" path (Printexc.to_string exn)

let load_all_from_directory (dir : string) =
  let load_dir d =
    try
      if Sys.is_directory d then
        Sys.readdir d
        |> Array.iter (fun file ->
            if Filename.check_suffix file ".cmxs" then
              load_plugin (Filename.concat d file)
          )
    with _ -> ()
  in
  load_dir dir;
  load_dir (Filename.concat dir "bin")
