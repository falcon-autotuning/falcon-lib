open Lwt.Infix
open Specifications

class virtual base_signal_analyzer hash request instance_vars handler config_io device_config = object(self)
  inherit Base_acu.base_algorithmic_control_unit hash request instance_vars handler config_io device_config as super

  val mutable python_script = ""
  val mutable input_signals = `Null

  method set_python_script path = python_script <- path

  method! setup _kwargs =
    super#setup _kwargs >>= fun () ->
    if python_script <> "" then
      begin
        self#log "WARNING: SignalAnalyzer is using deprecated Python bridge. Move to native OCaml analysis." >>= fun () ->
        if not (Py.is_initialized ()) then Py.initialize ();
        Lwt.return_unit
      end
    else
      Lwt.return_unit

  (** [DEPRECATED] Direct Yojson.Safe.t to Py.Object.t bridge (no intermediate JSON strings) *)
  method private yojson_to_py (j : Yojson.Safe.t) : Py.Object.t =
    match j with
    | `Null -> Py.none
    | `Bool b -> Py.Bool.of_bool b
    | `Int i -> Py.Int.of_int i
    | `Float f -> Py.Float.of_float f
    | `String s -> Py.String.of_string s
    | `Assoc l ->
        let dict = Py.Dict.create () in
        List.iter (fun (k, v) -> Py.Dict.set_item dict (Py.String.of_string k) (self#yojson_to_py v)) l;
        dict
    | `List l ->
        Py.List.of_list (List.map (fun v -> self#yojson_to_py v) l)
    | _ -> Py.none

  (** [DEPRECATED] Convert DeviceCharacteristic.t directly to a Python dict *)
  method private characteristic_to_py (c : DeviceCharacteristic.t) : Py.Object.t =
    let dict = Py.Dict.create () in
    let set k v = Py.Dict.set_item dict (Py.String.of_string k) v in
    set "name" (Py.String.of_string c.name);
    set "indexes" (Py.List.of_list (List.map Py.String.of_string c.indexes));
    set "hash" (Py.String.of_string c.hash);
    set "time" (Py.Int.of_int c.time);
    set "unit_type" (Py.String.of_string c.unit_type);
    set "uncertainty" (Py.Float.of_float c.uncertainty);
    set "value" (self#yojson_to_py c.characteristic);
    dict

  method private build_context () =
    let signals = match List.assoc_opt "input_signals" instance_variables with
      | Some s -> s
      | None -> `Null
    in
    let deps_assoc = List.map (fun (name, char) ->
      let char_json = `Assoc [
        ("name", `String char.DeviceCharacteristic.name);
        ("value", char.DeviceCharacteristic.characteristic);
        ("unit_type", `String char.DeviceCharacteristic.unit_type);
        ("uncertainty", `Float char.DeviceCharacteristic.uncertainty)
      ] in
      (name, char_json)
    ) dependencies in
    `Assoc [
      ("signals", signals);
      ("instance_vars", `Assoc instance_variables);
      ("dependencies", `Assoc deps_assoc)
    ]

  (** 
    The main analysis hook. Subclasses should override this for native OCaml analysis.
    By default, it falls back to the Python script if provided (DEPRECATED).
  *)
  method analyze (context : Yojson.Safe.t) =
    if python_script <> "" then
      begin
        self#log "SignalAnalyzer: Falling back to deprecated Python analysis" >>= fun () ->
        self#run_python_analysis context
      end
    else
      begin
        let msg = "No analysis method implemented (and no Python script provided)" in
        Lwt.return (false, msg, [])
      end

  method! program () =
    let context = self#build_context () in
    self#analyze context >>= fun (s, m, p) ->
    success <- s;
    result <- m;
    products <- p;
    Lwt.return_unit

  (** [DEPRECATED] Internal method to execute user script via pyml *)
  method private run_python_analysis context_json =
    try
      (* Build the enriched Python context object from JSON context *)
      let context = self#yojson_to_py context_json in
      
      (* Execute user script *)
      let script_source = 
        let ic = open_in python_script in
        let s = really_input_string ic (in_channel_length ic) in
        close_in ic; s
      in
      
      let user_mod_name = Printf.sprintf "analyzer_%s" (Filename.basename python_script |> Filename.chop_extension |> String.map (function '-' -> '_' | c -> c)) in
      let user_module = Py.Module.create user_mod_name in
      let globals = Py.Module.get_dict user_module in
      let _ = Py.Run.eval ~globals ~start:Py.File script_source in
      
      let analyze_func = Py.Module.get user_module "analyze" in
      if Py.is_null analyze_func then
        Lwt.return (false, "Python script does not define 'analyze' function", [])
      else
        begin
          let py_result = Py.Callable.to_function analyze_func [| context |] in
          
          (* The result should be a dictionary with {success, message, products} *)
          let get_bool k d = 
            match Py.Dict.get_item d (Py.String.of_string k) with
            | Some v -> Py.Bool.to_bool v
            | None -> false
          in
          let get_string k d =
            match Py.Dict.get_item d (Py.String.of_string k) with
            | Some v -> Py.String.to_string v
            | None -> ""
          in
          
          let s = get_bool "success" py_result in
          let m = get_string "message" py_result in
          
          (* products mapping back to Yojson *)
          let products_assoc = match Py.Dict.get_item py_result (Py.String.of_string "products") with
            | None -> []
            | Some py_products ->
                let py_json = Py.import "json" in
                let dumps = Py.Module.get py_json "dumps" in
                let res_str = Py.String.to_string (Py.Callable.to_function dumps [| py_products |]) in
                match Yojson.Safe.from_string res_str with
                | `Assoc l -> l
                | _ -> []
          in
          
          Lwt.return (s, m, products_assoc)
        end
    with e ->
      let msg = Printf.sprintf "Python bridge error: %s" (Printexc.to_string e) in
      Lwt.return (false, msg, [])
end
