open Lwt.Infix
open Falcon.Feature_optimizer_base
open Falcon.Acu_api

module MockJob = struct
  type params = float
  let run (acu : Falcon.Base_acu.base_algorithmic_control_unit) p =
    acu#log (Printf.sprintf "MockJob running with %f" p) >>= fun () ->
    Lwt.return { UnitResponse.message = "ok"; products = []; success = true }
end

module MockOptimizer = MakeOptimizer(MockJob)

let mock_config_io = Falcon.Message_handler.ConfigIO.create_empty ()
let mock_device_config = Falcon.Config_factory.create_simple_config ()

let test_optimization_loop () =
  let handler = Falcon.Message_handler.MessageHandler.create_mock 
    ~publish:(fun _ _ -> Lwt.return_unit)
    ~request:(fun _ _ _ -> Lwt.return "")
    ~subscribe:(fun _ _ -> Lwt.return_unit)
    ~receive_large_message:(fun _ _ -> Lwt.return "")
  in
  let opt = new MockOptimizer.t "hash" (`Assoc []) (`Assoc []) handler mock_config_io mock_device_config in
  opt#set_range 0.0 1.0 11;
  opt#set_job_params_factory (fun x -> x);
  opt#add_end_condition "target" (fun _ -> true); (* Succeeds immediately *)
  
  opt#program () >>= fun () ->
  Alcotest.(check bool) "optimizer success" true opt#success;
  Lwt.return_unit

let test_max_iters () =
  let handler = Falcon.Message_handler.MessageHandler.create_mock 
    ~publish:(fun _ _ -> Lwt.return_unit)
    ~request:(fun _ _ _ -> Lwt.return "")
    ~subscribe:(fun _ _ -> Lwt.return_unit)
    ~receive_large_message:(fun _ _ -> Lwt.return "")
  in
  let opt = new MockOptimizer.t "hash" (`Assoc []) (`Assoc []) handler mock_config_io mock_device_config in
  opt#set_range 0.0 1.0 3;
  opt#set_job_params_factory (fun x -> x);
  opt#add_end_condition "never" (fun _ -> false);
  
  opt#program () >>= fun () ->
  Alcotest.(check bool) "optimizer failure" false opt#success;
  Alcotest.(check string) "error message" "Internal Error: Max iterations reached without meeting end condition" opt#result;
  Lwt.return_unit

let () =
  let open Alcotest in
  run "Feature_optimizer_base" [
    "Logic", [
      test_case "Immediate Success" `Quick (fun () -> Lwt_main.run (test_optimization_loop ()));
      test_case "Max Iterations" `Quick (fun () -> Lwt_main.run (test_max_iters ()));
    ];
  ]
