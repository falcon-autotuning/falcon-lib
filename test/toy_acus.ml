open Lwt.Infix

class logger_toy hash request instance_vars message_handler config (device_config : Falcon_core.Config.t) = object(self)
  inherit Falcon.Base_acu.base_algorithmic_control_unit hash request instance_vars message_handler config device_config

  method! program () =
    self#log "LoggerToy: Starting execution loop" >>= fun () ->
    let rec loop n =
      if n <= 0 then
        self#log "LoggerToy: Finished execution"
      else
        self#log (Printf.sprintf "LoggerToy: Tick %d" n) >>= fun () ->
        Lwt_unix.sleep 1.0 >>= fun () ->
        loop (n - 1)
    in
    loop 5
end

class spawner_toy hash request instance_vars message_handler config (device_config : Falcon_core.Config.t) = object(self)
  inherit Falcon.Base_acu.base_algorithmic_control_unit hash request instance_vars message_handler config device_config

  method! program () =
    self#log "SpawnerToy: Attempting to spawn LoggerToy" >>= fun () ->
    let request = { Falcon.Acu_api.UnitRequest.message = "Spawned by SpawnerToy"; demands = [] } in
    self#request_subunit request "LoggerToy" [] >>= fun response ->
    self#log (Printf.sprintf "SpawnerToy: Subunit finished with success: %b" response.success)
end

class graph_autotuner_toy hash request instance_vars message_handler config (device_config : Falcon_core.Config.t) = object(self)
  inherit Falcon.Autotuner_base.base_autotuner hash request instance_vars message_handler config device_config

  method initial_state = "START"

  initializer
    self#add_state "START" {
      unit_type = "LoggerToy";
      instance_variables = [];
      next = { on_success = "END"; on_failure = "END" };
    };
    self#add_state "END" {
      unit_type = "LoggerToy";
      instance_variables = [];
      next = { on_success = "SUCCESS"; on_failure = "FAILURE" };
    }
end

class linear_optimizer_toy hash request instance_vars message_handler config (device_config : Falcon_core.Config.t) = object(self)
  inherit Falcon.Feature_optimizer_base.base_feature_optimizer hash request instance_vars message_handler config device_config


  initializer
    self#set_range 0.0 5.0 6;
    self#set_job_params_factory (fun v -> {
      unit_type = "LoggerToy";
      instance_variables = [];
      current_value = v;
    });
    self#add_end_condition "main" (fun resp -> resp.Falcon.Acu_api.UnitResponse.success)
end

class bridge_analyzer_toy hash request instance_vars message_handler config (device_config : Falcon_core.Config.t) = object(self)
  inherit Falcon.Signal_analyzer_base.base_signal_analyzer hash request instance_vars message_handler config device_config

  initializer
    self#set_python_script "/home/daniel/work/wisc/playground/python-port-playground/falcon/ocaml/src/analyzer_toy.py"
end

class formal_dependency_toy hash request instance_vars message_handler config (device_config : Falcon_core.Config.t) = object(self)
  inherit Falcon.Base_acu.base_algorithmic_control_unit hash request instance_vars message_handler config device_config

  initializer
    self#establish_dependency "gate_1_voltage"

  method! program () =
    self#log "FormalDependencyToy: Starting" >>= fun () ->
    let char = self#get_dependency "gate_1_voltage" in
    let current_voltage = match char.characteristic with `Float f -> f | `Int i -> float_of_int i | _ -> 0.0 in
    self#log (Printf.sprintf "FormalDependencyToy: Read current voltage %f" current_voltage) >>= fun () ->
    
    let new_voltage = current_voltage +. 0.1 in
    self#log (Printf.sprintf "FormalDependencyToy: Optimizing to %f" new_voltage) >>= fun () ->
    
    let product_char = Falcon.Specifications.DeviceCharacteristic.copy char in
    product_char.name <- "gate_1_optimized_voltage";
    product_char.characteristic <- `Float new_voltage;
    
    self#establish_product product_char;
    success <- true;
    result <- "Dependency handled and product established";
    Lwt.return_unit
end
