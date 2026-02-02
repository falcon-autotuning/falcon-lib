open Falcon

let test_threshold () =
  Alcotest.(check bool) "exceeds" true (Signal_utils.exceeds_threshold 10.0 5.0);
  Alcotest.(check bool) "not exceeds" false (Signal_utils.exceeds_threshold 3.0 5.0);
  ()

let test_offset () =
  let data = [|1.0; 2.0; 3.0|] in
  let offset = Signal_utils.calculate_offset data in
  Alcotest.(check (float 0.1)) "mean" 2.0 offset;
  let corrected = Signal_utils.apply_offset data offset in
  Alcotest.(check (array (float 0.1))) "corrected" [|-1.0; 0.0; 1.0|] corrected;
  ()

let () =
  let open Alcotest in
  run "Signal_utils" [
    "Logic", [
      test_case "Threshold" `Quick test_threshold;
      test_case "Offset" `Quick test_offset;
    ];
  ]
