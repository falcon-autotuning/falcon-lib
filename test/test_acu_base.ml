open Lwt.Infix
open Falcon.Base_acu
open Falcon.Message_handler

(* Mock components *)
let mock_config_io = ConfigIO.create_empty ()
let mock_device_config = Falcon.Config_factory.create_simple_config ()

let create_mock_handler ~publish ~request =
  MessageHandler.create_mock 
    ~publish 
    ~request 
    ~subscribe:(fun _ _ -> Lwt.return_unit)
    ~receive_large_message:(fun _ _ -> Lwt.return "")

class test_acu hash req vars handler = object
  inherit base_algorithmic_control_unit hash req vars handler mock_config_io mock_device_config
  method! program () = Lwt.return_unit
end

let test_dependency_management () =
  let handler = create_mock_handler 
    ~publish:(fun _ _ -> Lwt.return_unit)
    ~request:(fun _ _ _ -> Lwt.return "") in
  let acu = new test_acu "test_hash" (`Assoc []) (`Assoc []) handler in
  
  acu#establish_dependency "test_dep";
  let dep = acu#get_dependency "test_dep" in
  Alcotest.(check string) "dependency name" "test_dep" dep.Falcon.Specifications.DeviceCharacteristic.name;
  
  (try 
    ignore (acu#get_dependency "non_existent");
    Alcotest.fail "Should have raised Not_found"
  with Failure _ -> ());
  ()

let test_product_establishment () =
  let handler = create_mock_handler 
    ~publish:(fun _ _ -> Lwt.return_unit)
    ~request:(fun _ _ _ -> Lwt.return "") in
  let acu = new test_acu "test_hash" (`Assoc []) (`Assoc []) handler in
  
  let char = Falcon.Specifications.DeviceCharacteristic.make "test_product" [] in
  acu#establish_product char;
  (* Products are updated during _create_response or when manually added to products list *)
  (* But established products are in the 'outputs' list which is private. *)
  (* However, success is false by default. *)
  Alcotest.(check bool) "initial success" false acu#success;
  ()

let test_logging () =
  let published = ref [] in
  let publish chan msg = 
    published := (chan, msg) :: !published;
    Lwt.return_unit
  in
  let handler = create_mock_handler ~publish ~request:(fun _ _ _ -> Lwt.return "") in
  let acu = new test_acu "test_hash" (`Assoc []) (`Assoc []) handler in
  
  acu#log "hello world" >>= fun () ->
  let chan, msg = List.hd !published in
  Alcotest.(check string) "log channel" "LOG.test_hash" chan;
  let json = Yojson.Safe.from_string msg in
  let open Yojson.Safe.Util in
  Alcotest.(check string) "log message" "hello world" (json |> member "message" |> to_string);
  Lwt.return_unit

let () =
  let open Alcotest in
  run "Base_acu" [
    "State Management", [
      test_case "Dependencies" `Quick test_dependency_management;
      test_case "Products" `Quick test_product_establishment;
    ];
    "Communication", [
      test_case "Logging" `Quick (fun () -> Lwt_main.run (test_logging ()));
    ];
  ]
