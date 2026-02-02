open Lwt.Infix
open Falcon.Message_handler

let test_mock_transport () =
  let published = ref [] in
  let publish chan msg = 
    published := (chan, msg) :: !published;
    Lwt.return_unit
  in
  let request _ msg _ =
    Lwt.return (Printf.sprintf "response to %s" msg)
  in
  let handler = MessageHandler.create_mock 
    ~publish 
    ~request 
    ~subscribe:(fun _ _ -> Lwt.return_unit)
    ~receive_large_message:(fun _ _ -> Lwt.return "")
  in
  
  MessageHandler.publish handler "test_chan" "hello" >>= fun () ->
  Alcotest.(check (list (pair string string))) "published" [("test_chan", "hello")] !published;
  
  MessageHandler.request handler "test_chan" "ping" 1000 >>= fun resp ->
  Alcotest.(check string) "request response" "response to ping" resp;
  Lwt.return_unit

let test_config_io_json () =
  let json = `Assoc [
    ("url", `String "nats://localhost:4222");
    ("map", `Assoc [
      ("cmd1", `Assoc [
        ("receive_channel", `String "resp1");
        ("receive_timeout", `Float 5.0)
      ])
    ])
  ] in
  let config = ConfigIO.from_json json in
  Alcotest.(check string) "url" "nats://localhost:4222" config.url;
  let endpoint = match ConfigIO.get_endpoint config "cmd1" with Some e -> e | None -> Alcotest.fail "Endpoint not found" in
  Alcotest.(check (option string)) "receive_channel" (Some "resp1") endpoint.receive_channel;
  Alcotest.(check (float 0.1)) "receive_timeout" 5.0 endpoint.receive_timeout;
  ()

let () =
  let open Alcotest in
  run "Message_handler" [
    "Transport", [
      test_case "Mock" `Quick (fun () -> Lwt_main.run (test_mock_transport ()));
    ];
    "ConfigIO", [
      test_case "JSON parsing" `Quick test_config_io_json;
    ];
  ]
