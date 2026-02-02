open Lwt.Infix
open Message_handler
open Specifications
open Falcon_core

class type acu_unit = object
  method hash : string
  method success : bool
  method result : string
  method products : (string * Yojson.Safe.t) list
  method instance_variables : (string * Yojson.Safe.t) list
  method establish_dependency : string -> unit
  method establish_product : Specifications.DeviceCharacteristic.t -> unit
  method get_dependency : string -> Specifications.DeviceCharacteristic.t
  method setup : (string * Yojson.Safe.t) list -> unit Lwt.t
  method program : unit -> unit Lwt.t
  method main : unit -> unit Lwt.t
  method log : string -> unit Lwt.t
  method process_request : string -> Yojson.Safe.t -> unit Lwt.t
  method process_request_with_response : string -> Yojson.Safe.t -> ?unique_id:string -> unit -> Yojson.Safe.t Lwt.t
  method request_subunits : Acu_api.UnitRequest.t list -> string list -> Acu_api.InstanceVariables.t list -> Acu_api.UnitResponse.t list Lwt.t
  method request_subunit : Acu_api.UnitRequest.t -> string -> Acu_api.InstanceVariables.t -> Acu_api.UnitResponse.t Lwt.t
  method get_attribute : string -> Yojson.Safe.t option
  method _create_response : unit -> Acu_api.UnitResponse.t
  method load : Specifications.DeviceCharacteristic.t -> unit Lwt.t
  method safe_read : Specifications.DeviceCharacteristic.t -> ?history_index:int -> unit -> (bool * Specifications.DeviceCharacteristic.t) Lwt.t
  method read : Specifications.DeviceCharacteristic.t -> ?history_index:int -> unit -> Specifications.DeviceCharacteristic.t Lwt.t
  method exists_with_uncertainty : Specifications.DeviceCharacteristic.t -> ?history_index:int -> unit -> bool Lwt.t
  method read_with_uncertainty : Specifications.DeviceCharacteristic.t -> ?history_index:int -> unit -> bool Lwt.t
  method request_measurement : Falcon_core.MeasurementRequest.t -> Falcon_core.MeasurementResponse.t Lwt.t
  method request_device_state : unit -> Falcon_core.VoltageStatesResponse.t Lwt.t
  method process_receipt : string -> string -> Yojson.Safe.t Lwt.t
  method print_autotuner : Yojson.Safe.t -> unit Lwt.t
  method erase_runtime_database : unit -> unit Lwt.t
end

class base_algorithmic_control_unit (hash_init : string) (request_init : Yojson.Safe.t) (instance_vars_init : Yojson.Safe.t) (handler_init : MessageHandler.t) (config_io_init : ConfigIO.t) (device_config_init : Config.t) = object(self)
  val hash = hash_init
  val request = request_init
  val instance_variables = Acu_api.InstanceVariables.from_json instance_vars_init
  val message_handler = handler_init
  val config_io = config_io_init
  val device_config = device_config_init
  val mutable products : (string * Yojson.Safe.t) list = []
  val mutable result : string = ""
  val mutable success : bool = false
  val mutable spawn_count : int = 0
  val mutable dependencies : (string * Specifications.DeviceCharacteristic.t) list = []
  val mutable outputs : Specifications.DeviceCharacteristic.t list = []
  val parsed_request = Acu_api.UnitRequest.from_json request_init

  method hash = hash
  method success = success
  method result = result
  method products = products
  method instance_variables = instance_variables

  method establish_dependency (name : string) =
    if not (Stdlib.List.mem_assoc name dependencies) then
      let char = Specifications.DeviceCharacteristic.make name [] in
      dependencies <- (name, char) :: dependencies

  method establish_product (char : Specifications.DeviceCharacteristic.t) =
    outputs <- char :: outputs

  method get_dependency (name : string) : Specifications.DeviceCharacteristic.t =
    try Stdlib.List.assoc name dependencies
    with Not_found -> failwith (Printf.sprintf "Dependency %s not found" name)

  method setup (_kwargs : (string * Yojson.Safe.t) list) : unit Lwt.t =
    if Stdlib.List.length dependencies = 0 then Lwt.return_unit
    else
      self#log "Resolving established dependencies from specifications" >>= fun () ->
      Lwt_list.iter_s (fun (name, char) ->
        self#safe_read char () >>= fun (success, resolved) ->
        if success then
          begin
            let updated = Stdlib.List.map (fun (n, c) -> if n = name then (n, resolved) else (n, c)) dependencies in
            dependencies <- updated;
            Lwt.return_unit
          end
        else
          self#log (Printf.sprintf "Warning: Failed to resolve dependency %s" name)
      ) dependencies

  method program () : unit Lwt.t =
    Lwt.fail_with "The program function is not implemented."

  method main () : unit Lwt.t =
    Lwt.catch
      (fun () -> 
        self#program () >>= fun () ->
        if success && Stdlib.List.length outputs > 0 then
          begin
            self#log "Storing established products to specifications" >>= fun () ->
            Lwt_list.iter_s (fun char -> self#load char) outputs
          end
        else Lwt.return_unit
      )
      (fun exn -> self#log (Printf.sprintf "Main program failed to execute: %s" (Printexc.to_string exn)) >>= fun () -> Lwt.fail_with (Printf.sprintf "Main program failed: %s" (Printexc.to_string exn)))

  method log (msg : string) : unit Lwt.t =
    let payload = `Assoc [
      (Acu_api.RUNTIME_COMMANDS.LOG.hash, `String hash);
      (Acu_api.RUNTIME_COMMANDS.LOG.timestamp, `Int (int_of_float (Unix.gettimeofday ())));
      (Acu_api.RUNTIME_COMMANDS.LOG.message, `String msg);
    ] in
    let channel = Acu_api.RUNTIME_COMMANDS.LOG.comm_channel ^ "." ^ hash in
    MessageHandler.publish message_handler channel (Yojson.Safe.to_string payload)

  method process_request (channel : string) (message : Yojson.Safe.t) : unit Lwt.t =
    MessageHandler.publish message_handler channel (Yojson.Safe.to_string message)

  method process_request_with_response (request_channel : string) (message : Yojson.Safe.t) ?unique_id () : Yojson.Safe.t Lwt.t =
    let timeout_ms = 5000 in 
    let response_channel = match unique_id with
      | Some id -> request_channel ^ "." ^ id
      | None -> request_channel ^ ".response"
    in
    MessageHandler.request_with_custom_response message_handler request_channel (Yojson.Safe.to_string message) response_channel timeout_ms
    >|= Yojson.Safe.from_string

  method request_subunits (reqs : Acu_api.UnitRequest.t list) (units : string list) (vars : Acu_api.InstanceVariables.t list) : Acu_api.UnitResponse.t list Lwt.t =
    let len = Stdlib.List.length reqs in
    if len <> Stdlib.List.length units || len <> Stdlib.List.length vars then
      Lwt.fail_with "Requests, units, and instance_variables must be the same length."
    else
      let base_request_channel = Acu_api.RUNTIME_COMMANDS.SPAWN_COMMAND.comm_channel ^ "." ^ hash in
      let base_response_channel = "SPAWN_RESPONSE." ^ hash in 
      
      let min_count = spawn_count in
      spawn_count <- spawn_count + len;
      
      let subunit_hashes = Stdlib.List.init len (fun i -> Printf.sprintf "%s.%d" hash (min_count + i)) in
      let response_channels = Stdlib.List.init len (fun i -> Printf.sprintf "%s.%d" base_response_channel (min_count + i)) in
      let request_channels = Stdlib.List.init len (fun i -> Printf.sprintf "%s.%d" base_request_channel (min_count + i)) in
      
      let messages = Stdlib.List.init len (fun i ->
        `Assoc [
          (Acu_api.RUNTIME_COMMANDS.SPAWN_COMMAND.hash, `String (Stdlib.List.nth subunit_hashes i));
          (Acu_api.RUNTIME_COMMANDS.SPAWN_COMMAND.timestamp, `Int (int_of_float (Unix.gettimeofday ())));
          (Acu_api.RUNTIME_COMMANDS.SPAWN_COMMAND.unit_type, `String (Stdlib.List.nth units i));
          (Acu_api.RUNTIME_COMMANDS.SPAWN_COMMAND.request, `String (Yojson.Safe.to_string (Acu_api.UnitRequest.to_json (Stdlib.List.nth reqs i))));
          (Acu_api.RUNTIME_COMMANDS.SPAWN_COMMAND.instance_variables, `String (Yojson.Safe.to_string (Acu_api.InstanceVariables.to_json (Stdlib.List.nth vars i))));
        ]
      ) in
      
      let ui_cmd_channel = Acu_api.RUNTIME_COMMANDS.UI_PACKETS.SPAWN_CMD_PKT.comm_channel ^ "." ^ hash in
      Lwt_list.iter_p (fun msg -> self#process_request ui_cmd_channel msg) messages >>= fun () ->
      
      let tasks = Stdlib.List.init len (fun i ->
        MessageHandler.request_with_custom_response message_handler (Stdlib.List.nth request_channels i) (Yojson.Safe.to_string (Stdlib.List.nth messages i)) (Stdlib.List.nth response_channels i) 10000
        >|= Yojson.Safe.from_string
      ) in
      
      Lwt.all tasks >>= fun responses ->
      
      let outs = Stdlib.List.map (fun incoming ->
        let open Yojson.Safe.Util in
        Acu_api.UnitResponse.from_json (incoming |> member Acu_api.RUNTIME_COMMANDS.SPAWN_RESPONSE.response)
      ) responses in
      
      let ui_resp_channel = Acu_api.RUNTIME_COMMANDS.UI_PACKETS.SPAWN_RESP_PKT.comm_channel ^ "." ^ hash in
      Lwt_list.iter_p (fun incoming -> self#process_request ui_resp_channel incoming) responses >>= fun () ->
      
      Lwt.return outs

  method request_subunit (req : Acu_api.UnitRequest.t) (unit : string) (vars : Acu_api.InstanceVariables.t) : Acu_api.UnitResponse.t Lwt.t =
    self#request_subunits [req] [unit] [vars] >|= Stdlib.List.hd

  method get_attribute (name : string) : Yojson.Safe.t option =
    match name with
    | "hash" -> Some (`String hash)
    | "result" -> Some (`String result)
    | "success" -> Some (`Bool success)
    | _ -> None

  method private _find_missing_demand_attributes () : string list =
    Stdlib.List.filter (fun (name, _) ->
      match self#get_attribute name with
      | None -> true
      | Some _ -> false
    ) parsed_request.demands |> Stdlib.List.map fst

  method private _store_missing_demand_names () : unit =
    let missing = self#_find_missing_demand_attributes () in
    Stdlib.List.iter (fun name ->
      result <- result ^ (Printf.sprintf "AttributeError: Object does not have attribute %s\n" name)
    ) missing

  method private _generate_product () : unit =
    let existing = Stdlib.List.filter (fun (name, _) ->
      match self#get_attribute name with
      | Some _ -> true
      | None -> false
    ) parsed_request.demands in
    products <- Stdlib.List.map (fun (name, _) ->
      (name, Option.get (self#get_attribute name))
    ) existing

  method private _check_results_exist () : unit =
    if result = "" then success <- true

  method _create_response () : Acu_api.UnitResponse.t =
    self#_check_results_exist ();
    self#_store_missing_demand_names ();
    if Stdlib.List.length products = 0 then self#_generate_product ();
    {
      message = result;
      products = products;
      success = success;
    }

  method load (char : DeviceCharacteristic.t) : unit Lwt.t =
    let payload = `Assoc [
      (Acu_api.RUNTIME_COMMANDS.LOAD.hash, `String hash);
      (Acu_api.RUNTIME_COMMANDS.LOAD.timestamp, `Int (int_of_float (Unix.gettimeofday ())));
      (Acu_api.RUNTIME_COMMANDS.LOAD.device_characteristic, char.characteristic);
      (Acu_api.RUNTIME_COMMANDS.LOAD.keys, `List (Stdlib.List.map (fun s -> `String s) char.indexes));
      (Acu_api.RUNTIME_COMMANDS.LOAD.name, `String char.name);
      (Acu_api.RUNTIME_COMMANDS.LOAD.device_state, `String (Acu_api.string_to_ocaml (DeviceVoltageStates.to_json_string char.state#raw)));
      (Acu_api.RUNTIME_COMMANDS.LOAD.uncertainty, `Float char.uncertainty);
      (Acu_api.RUNTIME_COMMANDS.LOAD.unitname, `String char.unit_type);
      (Acu_api.RUNTIME_COMMANDS.LOAD.dbhash, `String hash);
      (Acu_api.RUNTIME_COMMANDS.LOAD.dbtime, `Int (int_of_float (Unix.gettimeofday ())));
    ] in
    let channel = Acu_api.RUNTIME_COMMANDS.LOAD.comm_channel ^ "." ^ hash in
    self#process_request channel payload >>= fun () ->
    let ui_channel = Acu_api.RUNTIME_COMMANDS.UI_PACKETS.LOAD_PKT.comm_channel ^ "." ^ hash in
    self#process_request ui_channel payload

  method safe_read (char_map : DeviceCharacteristic.t) ?(history_index=0) () : (bool * DeviceCharacteristic.t) Lwt.t =
    let states_str = Acu_api.string_to_ocaml (DeviceVoltageStates.to_json_string char_map.state#raw) in
    let payload = `Assoc [
      (Acu_api.RUNTIME_COMMANDS.READ_COMMAND.hash, `String hash);
      (Acu_api.RUNTIME_COMMANDS.READ_COMMAND.timestamp, `Int (int_of_float (Unix.gettimeofday ())));
      (Acu_api.RUNTIME_COMMANDS.READ_COMMAND.name, `String char_map.name);
      (Acu_api.RUNTIME_COMMANDS.READ_COMMAND.keys, `List (Stdlib.List.map (fun s -> `String s) char_map.indexes));
      (Acu_api.RUNTIME_COMMANDS.READ_COMMAND.unitname, `String char_map.unit_type);
      (Acu_api.RUNTIME_COMMANDS.READ_COMMAND.uncertainty, `Float char_map.uncertainty);
      (Acu_api.RUNTIME_COMMANDS.READ_COMMAND.device_state, `String states_str);
      (Acu_api.RUNTIME_COMMANDS.READ_COMMAND.dbhash, `String char_map.hash);
      (Acu_api.RUNTIME_COMMANDS.READ_COMMAND.dbtime, `Int char_map.time);
      (Acu_api.RUNTIME_COMMANDS.READ_COMMAND.history_index, `Int history_index);
    ] in
    let channel = Acu_api.RUNTIME_COMMANDS.READ_COMMAND.comm_channel ^ "." ^ hash in
    self#process_request_with_response channel payload () >>= fun incoming ->
    let open Yojson.Safe.Util in
    let exists = 
      (incoming |> member Acu_api.RUNTIME_COMMANDS.READ_RESPONSE.keys <> `Null) &&
      (incoming |> member Acu_api.RUNTIME_COMMANDS.READ_RESPONSE.device_characteristic <> `Null)
    in
    if exists then
      let out = DeviceCharacteristic.copy char_map in
      out.characteristic <- incoming |> member Acu_api.RUNTIME_COMMANDS.READ_RESPONSE.device_characteristic;
      out.hash <- incoming |> member Acu_api.RUNTIME_COMMANDS.READ_RESPONSE.dbhash |> to_string;
      out.time <- incoming |> member Acu_api.RUNTIME_COMMANDS.READ_RESPONSE.dbtime |> to_int;
      let resp_state = incoming |> member Acu_api.RUNTIME_COMMANDS.READ_RESPONSE.device_state |> to_string in
      out.state <- DeviceVoltageStates.from_json_string (Falcon_core.String.wrap resp_state);
      out.uncertainty <- incoming |> member Acu_api.RUNTIME_COMMANDS.READ_RESPONSE.uncertainty |> Acu_api.to_number_compat;
      out.unit_type <- incoming |> member Acu_api.RUNTIME_COMMANDS.READ_RESPONSE.unitname |> to_string;
      Lwt.return (true, out)
    else
      Lwt.return (false, char_map)

  method read (char_map : DeviceCharacteristic.t) ?(history_index=0) () : DeviceCharacteristic.t Lwt.t =
    self#safe_read char_map ~history_index () >>= fun (_, res) ->
    Lwt.return res

  method exists_with_uncertainty (char_map : DeviceCharacteristic.t) ?(history_index=0) () : bool Lwt.t =
    self#safe_read char_map ~history_index () >>= fun (safe, out) ->
    Lwt.return (safe && out.uncertainty <= char_map.uncertainty)

  method read_with_uncertainty (char_map : DeviceCharacteristic.t) ?(history_index=0) () : bool Lwt.t =
    self#read char_map ~history_index () >>= fun out ->
    Lwt.return (out.uncertainty <= char_map.uncertainty)

  method request_measurement (req : MeasurementRequest.t) : MeasurementResponse.t Lwt.t =
    let payload = `Assoc [
      (Acu_api.RUNTIME_COMMANDS.MEASURE_COMMAND.hash, `String hash);
      (Acu_api.RUNTIME_COMMANDS.MEASURE_COMMAND.timestamp, `Int (int_of_float (Unix.gettimeofday ())));
      (Acu_api.RUNTIME_COMMANDS.MEASURE_COMMAND.request, Yojson.Safe.from_string (Acu_api.string_to_ocaml (MeasurementRequest.to_json_string req#raw)));
    ] in
    let channel = Acu_api.RUNTIME_COMMANDS.MEASURE_COMMAND.comm_channel ^ "." ^ hash in
    self#process_request_with_response channel payload () >>= fun incoming ->
    let open Yojson.Safe.Util in
    let ch = incoming |> member Acu_api.RUNTIME_COMMANDS.MEASURE_RESPONSE.channel |> to_string in
    let st = incoming |> member Acu_api.RUNTIME_COMMANDS.MEASURE_RESPONSE.stream |> to_string in
    MessageHandler.receive_large_message message_handler ch st >>= fun raw ->
    let wrapped = Falcon_core.String.wrap raw in
    Lwt.return (MeasurementResponse.from_json_string wrapped)

  method request_device_state () : VoltageStatesResponse.t Lwt.t =
    let payload = `Assoc [
      (Acu_api.RUNTIME_COMMANDS.STATE_REQUEST.hash, `String hash);
      (Acu_api.RUNTIME_COMMANDS.STATE_REQUEST.timestamp, `Int (int_of_float (Unix.gettimeofday ())));
    ] in
    let channel = Acu_api.RUNTIME_COMMANDS.STATE_REQUEST.comm_channel ^ "." ^ hash in
    self#process_request_with_response channel payload () >>= fun incoming ->
    let open Yojson.Safe.Util in
    let resp = incoming |> member Acu_api.RUNTIME_COMMANDS.STATE_RESPONSE.response |> to_string in
    Lwt.return (VoltageStatesResponse.from_json_string (Falcon_core.String.wrap resp))

  method process_receipt (listen_on_topic : string) (action_config_key : string) : Yojson.Safe.t Lwt.t =
    let timeout = ConfigIO.get_receive_timeout config_io action_config_key in
    let timeout_ms = int_of_float (timeout *. 1000.0) in
    MessageHandler.receive message_handler listen_on_topic timeout_ms >|= Yojson.Safe.from_string

  method print_autotuner (contents : Yojson.Safe.t) : unit Lwt.t =
    let payload = `Assoc [
      (Acu_api.RUNTIME_COMMANDS.PRINT_AUTOTUNER.hash, `String hash);
      (Acu_api.RUNTIME_COMMANDS.PRINT_AUTOTUNER.timestamp, `Int (int_of_float (Unix.gettimeofday ())));
      (Acu_api.RUNTIME_COMMANDS.PRINT_AUTOTUNER.contents, contents);
    ] in
    let channel = Acu_api.RUNTIME_COMMANDS.PRINT_AUTOTUNER.comm_channel ^ "." ^ hash in
    self#process_request channel payload

  method erase_runtime_database () : unit Lwt.t =
    let payload = `Assoc [
      (Acu_api.RUNTIME_COMMANDS.ERASE_DATABASE.hash, `String hash);
      (Acu_api.RUNTIME_COMMANDS.ERASE_DATABASE.timestamp, `Int (int_of_float (Unix.gettimeofday ())));
    ] in
    let channel = Acu_api.RUNTIME_COMMANDS.ERASE_DATABASE.comm_channel ^ "." ^ hash in
    self#process_request channel payload
end
