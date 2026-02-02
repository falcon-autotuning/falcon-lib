open Acu_api
open Acu_types

module Simple = struct
  module type INPUT = sig
    val program : Base_acu.base_algorithmic_control_unit -> unit Lwt.t
  end

  module Template (I : INPUT) : ACU_INSTANCE = struct
    let state = ref None

    let initialize ~hash ~request ~instance_vars ~handler ~config_io ~device_config () =
      let acu = new Base_acu.base_algorithmic_control_unit hash request instance_vars handler config_io device_config in
      state := Some acu;
      acu#setup (Acu_api.InstanceVariables.from_json instance_vars)

    let setup kwargs =
      match !state with
      | Some acu -> acu#setup kwargs
      | None -> Lwt.fail_with "ACU not initialized"

    let main () =
      match !state with
      | Some acu -> I.program acu
      | None -> Lwt.fail_with "ACU not initialized"

    let get_response () =
      match !state with
      | Some acu -> 
          { UnitResponse.message = acu#result; 
            success = acu#success; 
            products = acu#products }
      | None -> failwith "ACU not initialized"
  end
end

module Optimizer = struct
  module type INPUT = sig
    module J : Optimizer_jobs.JOB
    val range_start : float
    val range_stop : float
    val max_iters : int
    val job_params_factory : float -> J.params
    val end_conditions : (string * (UnitResponse.t -> bool)) list
  end

  module Template (I : INPUT) : ACU_INSTANCE = struct
    module Opt = Feature_optimizer_base.MakeOptimizer(I.J)
    let instance = ref None

    let initialize ~hash ~request ~instance_vars ~handler ~config_io ~device_config () =
      let opt = new Opt.t hash request instance_vars handler config_io device_config in
      opt#set_range I.range_start I.range_stop I.max_iters;
      opt#set_job_params_factory I.job_params_factory;
      List.iter (fun (gid, cond) -> opt#add_end_condition gid cond) I.end_conditions;
      instance := Some opt;
      opt#setup (Acu_api.InstanceVariables.from_json instance_vars)

    let setup kwargs =
      match !instance with
      | Some opt -> opt#setup kwargs
      | None -> Lwt.fail_with "Optimizer not initialized"

    let main () =
      match !instance with
      | Some opt -> opt#program ()
      | None -> Lwt.fail_with "Optimizer not initialized"

    let get_response () =
      match !instance with
      | Some opt -> 
          { UnitResponse.message = opt#result; 
            success = opt#success; 
            products = opt#products }
      | None -> failwith "Optimizer not initialized"
  end
end

module Autotuner = struct
  module type INPUT = sig
    val initial_state : string
    val graph : (string * Autotuner_base.state_node) list
  end

  module Template (I : INPUT) : ACU_INSTANCE = struct
    let instance = ref None

    let initialize ~hash ~request ~instance_vars ~handler ~config_io ~device_config () =
      let tuner = object
        inherit Autotuner_base.base_autotuner hash request instance_vars handler config_io device_config
        method initial_state = I.initial_state
      end in
      List.iter (fun (name, node) -> tuner#add_state name node) I.graph;
      instance := Some tuner;
      tuner#setup (Acu_api.InstanceVariables.from_json instance_vars)

    let setup kwargs =
      match !instance with
      | Some tuner -> tuner#setup kwargs
      | None -> Lwt.fail_with "Autotuner not initialized"

    let main () =
      match !instance with
      | Some tuner -> tuner#program ()
      | None -> Lwt.fail_with "Autotuner not initialized"

    let get_response () =
      match !instance with
      | Some tuner -> 
          { UnitResponse.message = tuner#result; 
            success = tuner#success; 
            products = tuner#products }
      | None -> failwith "Autotuner not initialized"
  end
end

module ClassBridge (C : sig 
  val create : 
    string -> 
    Yojson.Safe.t -> 
    Yojson.Safe.t -> 
    Message_handler.MessageHandler.t -> 
    Message_handler.ConfigIO.t -> 
    Falcon_core.Config.t -> 
    Base_acu.base_algorithmic_control_unit 
end) : ACU_INSTANCE = struct
  let instance = ref None

  let initialize ~hash ~request ~instance_vars ~handler ~config_io ~device_config () =
    let i = C.create hash request instance_vars handler config_io device_config in
    instance := Some i;
    i#setup (Acu_api.InstanceVariables.from_json instance_vars)

  let setup kwargs =
    match !instance with
    | Some i -> i#setup kwargs
    | None -> Lwt.fail_with "ACU not initialized"

  let main () =
    match !instance with
    | Some i -> i#main ()
    | None -> Lwt.fail_with "ACU not initialized"

  let get_response () =
    match !instance with
    | Some i -> 
        { UnitResponse.message = i#result; 
          success = i#success; 
          products = i#products }
    | None -> failwith "ACU not initialized"
end
