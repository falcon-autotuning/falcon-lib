open Falcon

let test_create_config () =
  let conf = Config_factory.create_simple_config () in
  Alcotest.(check bool) "config is not null" true (conf#raw <> Ctypes.null);
  let raw_json = Falcon_core.Config.to_json_string conf#raw in
  let s = Falcon.Acu_api.string_to_ocaml raw_json in
  Alcotest.(check bool) "json is not empty" true (String.length s > 0)

let () =
  let open Alcotest in
  run "ConfigFactory" [
    "Creation", [
      test_case "SimpleConfig" `Quick test_create_config;
    ];
  ]
