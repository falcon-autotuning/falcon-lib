(* Standalone pyml verification *)
let () =
  Py.initialize ();
  print_endline "Python initialized successfully.";
  
  let py_json = Py.import "json" in
  let loads = Py.Module.get py_json "loads" in
  let py_obj = Py.Callable.to_function loads [| Py.String.of_string "{\"test\": 42}" |] in
  
  let dumps = Py.Module.get py_json "dumps" in
  let result_str = Py.String.to_string (Py.Callable.to_function dumps [| py_obj |]) in
  
  Printf.printf "Data roundtrip: %s\n" result_str;
  
  if result_str = "{\"test\": 42}" then
    print_endline "SUCCESS: pyml bridge working."
  else
    (print_endline "FAILURE: data mismatch."; exit 1)
