open Falcon.Acu_api

let test_instance_variables_json () =
  let vars : InstanceVariables.t = [("key1", `String "val1"); ("key2", `Int 42)] in
  let json = InstanceVariables.to_json vars in
  let vars' = InstanceVariables.from_json json in
  Alcotest.(check (list (pair string (of_pp Yojson.Safe.pp)))) "instance variables roundtrip" vars vars'

let test_unit_request_json () =
  let req : UnitRequest.t = { message = "hello"; demands = [("d1", "v1")] } in
  let json = UnitRequest.to_json req in
  let req' = UnitRequest.from_json json in
  Alcotest.(check string) "request message" req.message req'.message;
  Alcotest.(check (list (pair string string))) "request demands" req.demands req'.demands

let test_unit_response_json () =
  let resp : UnitResponse.t = { message = "done"; products = [("p1", `Float 1.0)]; success = true } in
  let json = UnitResponse.to_json resp in
  let resp' = UnitResponse.from_json json in
  Alcotest.(check string) "response message" resp.message resp'.message;
  Alcotest.(check (list (pair string (of_pp Yojson.Safe.pp)))) "response products" resp.products resp'.products;
  Alcotest.(check bool) "response success" resp.success resp'.success

let () =
  let open Alcotest in
  run "Acu_api" [
    "JSON handling", [
      test_case "InstanceVariables" `Quick test_instance_variables_json;
      test_case "UnitRequest" `Quick test_unit_request_json;
      test_case "UnitResponse" `Quick test_unit_response_json;
    ];
  ]
