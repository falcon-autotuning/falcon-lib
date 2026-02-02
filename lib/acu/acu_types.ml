open Acu_api

(** The standard interface that every ACU module must satisfy for runtime execution. *)
module type ACU_INSTANCE = sig
  val initialize : 
    hash:string -> 
    request:Yojson.Safe.t -> 
    instance_vars:Yojson.Safe.t -> 
    handler:Message_handler.MessageHandler.t -> 
    config_io:Message_handler.ConfigIO.t -> 
    device_config:Falcon_core.Config.t -> 
    unit -> unit Lwt.t

  val setup : (string * Yojson.Safe.t) list -> unit Lwt.t
  val main : unit -> unit Lwt.t

  val get_response : unit -> UnitResponse.t
end

(** The general signature for any Template (functor) input. 
    Specific templates (like Optimizer) will extend this. *)
module type ACU_TEMPLATE_INPUT = sig
  val program : (module ACU_INSTANCE) -> unit Lwt.t
end
