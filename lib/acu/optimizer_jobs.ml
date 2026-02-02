open Acu_api
open Base_acu

module type JOB = sig
  type params
  val run : base_algorithmic_control_unit -> params -> UnitResponse.t Lwt.t
end

module LinearSearchJob = struct
  type params = {
    unit_type : string;
    instance_variables : (string * Yojson.Safe.t) list;
    current_value : float;
  }

  let run (acu : base_algorithmic_control_unit) params =
    let req = { 
      UnitRequest.message = Printf.sprintf "Linear Search step at %f" params.current_value; 
      demands = [] 
    } in
    acu#request_subunit req params.unit_type params.instance_variables
end
