open Lwt.Infix
open Falcon.Signal_analyzer_base

let mock_config_io = Falcon.Message_handler.ConfigIO.create_empty ()
let mock_device_config = Falcon.Config_factory.create_simple_config ()

class test_analyzer hash request instance_vars handler = object
  inherit base_signal_analyzer hash request instance_vars handler mock_config_io mock_device_config
end

let test_python_integration () =
  let script_path = "test_analyzer_tmp.py" in
  let oc = open_out script_path in
  output_string oc "
def analyze(context):
    return {
        'success': True,
        'message': 'Python analysis successful',
        'products': {'val': 42}
    }
";
  close_out oc;

  let handler = Falcon.Message_handler.MessageHandler.create_mock 
    ~publish:(fun _ _ -> Lwt.return_unit)
    ~request:(fun _ _ _ -> Lwt.return "")
    ~subscribe:(fun _ _ -> Lwt.return_unit)
    ~receive_large_message:(fun _ _ -> Lwt.return "")
  in
  
  let analyzer = new test_analyzer "hash" (`Assoc []) (`Assoc []) handler in
  analyzer#set_python_script script_path;
  
  analyzer#setup [] >>= fun () ->
  analyzer#program () >>= fun () ->
  
  Alcotest.(check bool) "analyzer success" true analyzer#success;
  Alcotest.(check string) "analyzer result" "Python analysis successful" analyzer#result;
  let p = List.assoc "val" analyzer#products in
  Alcotest.(check int) "product value" 42 (Yojson.Safe.Util.to_int p);
  
  Sys.remove script_path;
  Lwt.return_unit

let () =
  let open Alcotest in
  run "Signal_analyzer_base" [
    "Python Bridge", [
      test_case "Basic Analysis" `Quick (fun () -> Lwt_main.run (test_python_integration ()));
    ];
  ]
