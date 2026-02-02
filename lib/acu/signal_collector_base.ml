open Lwt.Infix
open Acu_api

class virtual base_signal_collector hash request instance_vars handler config_io device_config = object(self)
  inherit Base_acu.base_algorithmic_control_unit hash request instance_vars handler config_io device_config

  val mutable analyzer_units = []
  val mutable measurement_unit = "MeasurementUnit"
  val mutable measurement_config : Yojson.Safe.t = `Null

  method set_analyzers units = analyzer_units <- units
  method set_measurement_unit unit_name = measurement_unit <- unit_name
  method set_measurement_config conf = measurement_config <- conf

  method! program () =
    self#log "SignalCollector: Starting collection" >>= fun () ->
    self#collect_signals () >>= fun products ->
    self#log "SignalCollector: Collection finished, starting analysis" >>= fun () ->
    self#proctor_analysis products >>= fun results ->
    self#interpret_analyses results

  method private collect_signals () =
    let req = { Acu_api.UnitRequest.message = "Performing measurement"; demands = [] } in
    (* Correct order: req, unit, vars *)
    self#request_subunit req measurement_unit [] >>= fun response ->
    Lwt.return response.products

  method private proctor_analysis signals_json =
    let requests = List.map (fun _ ->
      { Acu_api.UnitRequest.message = "Analyzing signals"; demands = [] }
    ) analyzer_units in
    (* Pass signals to analyzers via instance variables *)
    let instance_variables = List.map (fun _ ->
      [("input_signals", `Assoc signals_json)]
    ) analyzer_units in
    
    self#request_subunits requests analyzer_units instance_variables

  method virtual interpret_analyses : UnitResponse.t list -> unit Lwt.t
end
