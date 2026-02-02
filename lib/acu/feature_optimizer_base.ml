open Lwt.Infix
open Acu_api
open Optimizer_jobs

module type OPTIMIZER = sig
  class type t = object
    inherit Base_acu.acu_unit
    method set_range : float -> float -> int -> unit
    method add_end_condition : string -> (UnitResponse.t -> bool) -> unit
  end
end

module MakeOptimizer (J : JOB) = struct
  class t hash request instance_vars handler config_io device_config = object(self)
    inherit Base_acu.base_algorithmic_control_unit hash request instance_vars handler config_io device_config

    val mutable range_start = 0.0
    val mutable range_stop = 0.0
    val mutable max_iters = 1
    val mutable end_condition_groups : (string, (UnitResponse.t -> bool) list) Hashtbl.t = Hashtbl.create 4
    val mutable job_params_factory : (float -> J.params) option = None

    method set_range start stop iters =
      range_start <- start;
      range_stop <- stop;
      max_iters <- iters

    method set_job_params_factory factory = job_params_factory <- Some factory

    method add_end_condition (group_id : string) (cond : UnitResponse.t -> bool) =
      let existing = try Hashtbl.find end_condition_groups group_id with Not_found -> [] in
      Hashtbl.replace end_condition_groups group_id (cond :: existing)

    method! program () =
      match job_params_factory with
      | None -> Lwt.fail_with "Job parameters factory not set"
      | Some factory ->
          self#log (Printf.sprintf "Starting Job-based Optimization from %f to %f (max_iters %d)" range_start range_stop max_iters) >>= fun () ->
          self#optimization_loop factory 0

    method private optimization_loop factory iter =
      if iter >= max_iters then
        begin
          self#log (Printf.sprintf "max_iter reached at %d" max_iters) >>= fun () ->
          success <- false;
          result <- "Internal Error: Max iterations reached without meeting end condition";
          Lwt.return_unit
        end
      else
        begin
          let current_val = 
            if max_iters <= 1 then range_start
            else range_start +. (range_stop -. range_start) *. (float_of_int iter /. float_of_int (max_iters - 1))
          in
          let params = factory current_val in
          
          self#log (Printf.sprintf "Optimization: Step %d/%d (val %f)" (iter + 1) max_iters current_val) >>= fun () ->
          
          J.run (self :> Base_acu.base_algorithmic_control_unit) params >>= fun response ->
          
          let group_result = ref None in
          Hashtbl.iter (fun group_id conds ->
            if !group_result = None then
              if List.for_all (fun f -> f response) conds then
                group_result := Some group_id
          ) end_condition_groups;

          match !group_result with
          | Some gid ->
              self#log (Printf.sprintf "Condition %s established" gid) >>= fun () ->
              success <- true;
              result <- Printf.sprintf "Optimization successful at %f" current_val;
              Lwt.return_unit
          | None ->
              self#optimization_loop factory (iter + 1)
        end
  end
end

(** Pre-instantiated standard optimizer for backward compatibility and ease of use *)
module LinearOptimizer = MakeOptimizer(LinearSearchJob)

class base_feature_optimizer hash request instance_vars handler config_io device_config = 
  let opt = new LinearOptimizer.t hash request instance_vars handler config_io device_config in
  object
    inherit Base_acu.base_algorithmic_control_unit hash request instance_vars handler config_io device_config
    method! setup kwargs = opt#setup kwargs
    method! program () = opt#program ()
    method! main () = opt#main ()
    method set_range s e n = opt#set_range s e n
    method set_job_params_factory f = opt#set_job_params_factory f
    method add_end_condition gid f = opt#add_end_condition gid f
  end
