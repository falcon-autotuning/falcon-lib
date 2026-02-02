open Lwt.Infix
open Falcon.Autotuner_base
open Falcon.Acu_api

let mock_config_io = Falcon.Message_handler.ConfigIO.create_empty ()
let mock_device_config = Falcon.Config_factory.create_simple_config ()

class test_autotuner hash request instance_vars handler = object
  inherit base_autotuner hash request instance_vars handler mock_config_io mock_device_config
  method initial_state = "START"
end

let test_counter_management () =
  let handler = Falcon.Message_handler.MessageHandler.create_mock 
    ~publish:(fun _ _ -> Lwt.return_unit)
    ~request:(fun _ _ _ -> Lwt.return "")
    ~subscribe:(fun _ _ -> Lwt.return_unit)
    ~receive_large_message:(fun _ _ -> Lwt.return "")
  in
  let tuner = new test_autotuner "hash" (`Assoc []) (`Assoc []) handler in
  tuner#add_counter "test" 10;
  let c = tuner#get_counter "test" in
  Alcotest.(check int) "initial count" 0 (Counter.get c);
  Counter.increment c;
  Alcotest.(check int) "incremented count" 1 (Counter.get c);
  ()

let test_history_management () =
  let handler = Falcon.Message_handler.MessageHandler.create_mock 
    ~publish:(fun _ _ -> Lwt.return_unit)
    ~request:(fun _ _ _ -> Lwt.return "")
    ~subscribe:(fun _ _ -> Lwt.return_unit)
    ~receive_large_message:(fun _ _ -> Lwt.return "")
  in
  let tuner = new test_autotuner "hash" (`Assoc []) (`Assoc []) handler in
  let h = tuner#get_history "state1" in
  let resp = { UnitResponse.message = "ok"; products = []; success = true } in
  History.add h resp;
  Alcotest.(check int) "history size" 1 (List.length (History.all h));
  Alcotest.(check string) "latest message" "ok" (Option.get (History.latest h)).message;
  ()

let test_execution_loop () =
  let subunit_responses = ref [
    (* Response for START state *)
    `Assoc [
      (RUNTIME_COMMANDS.SPAWN_RESPONSE.response, `Assoc [
        ("message", `String "Success in START");
        ("products", `Assoc []);
        ("success", `Bool true)
      ])
    ];
    (* Response for NEXT state *)
    `Assoc [
      (RUNTIME_COMMANDS.SPAWN_RESPONSE.response, `Assoc [
        ("message", `String "Success in NEXT");
        ("products", `Assoc []);
        ("success", `Bool true)
      ])
    ]
  ] in
  
  let request _ _ _ =
    match !subunit_responses with
    | h :: t ->
        subunit_responses := t;
        Lwt.return (Yojson.Safe.to_string h)
    | [] -> Lwt.fail_with "No more mock responses"
  in
  
  let handler = Falcon.Message_handler.MessageHandler.create_mock 
    ~publish:(fun _ _ -> Lwt.return_unit)
    ~request
    ~subscribe:(fun _ _ -> Lwt.return_unit)
    ~receive_large_message:(fun _ _ -> Lwt.return "")
  in
  
  let tuner = new test_autotuner "hash" (`Assoc []) (`Assoc []) handler in
  tuner#add_state "START" { unit_type = "Toy"; instance_variables = []; next = { on_success = "NEXT"; on_failure = "FAILURE" } };
  tuner#add_state "NEXT" { unit_type = "Toy"; instance_variables = []; next = { on_success = "SUCCESS"; on_failure = "FAILURE" } };
  
  tuner#program () >>= fun () ->
  Alcotest.(check bool) "autotuner success" true tuner#success;
  Alcotest.(check string) "autotuner result" "Autotuning completed successfully" tuner#result;
  Lwt.return_unit

let () =
  let open Alcotest in
  run "Autotuner_base" [
    "Management", [
      test_case "Counters" `Quick test_counter_management;
      test_case "History" `Quick test_history_management;
    ];
    "Execution", [
      test_case "Standard Flow" `Quick (fun () -> Lwt_main.run (test_execution_loop ()));
    ];
  ]
