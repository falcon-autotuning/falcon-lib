open Lwt.Infix

module History = struct
  type 'a t = {
    name : string;
    mutable data : 'a list;
  }

  let make name = { name; data = [] }
  let add t v = t.data <- v :: t.data
  let latest t = match t.data with [] -> None | h :: _ -> Some h
  let all t = t.data
end

module Counter = struct
  type t = {
    name : string;
    mutable count : int;
    max_count : int;
  }

  let make name max = { name; count = 0; max_count = max }
  let increment t = if t.count < t.max_count then t.count <- t.count + 1
  let decrement t = if t.count > 0 then t.count <- t.count - 1
  let set t v = t.count <- v
  let get t = t.count
end

type transition = {
  on_success : string;
  on_failure : string;
}

type state_node = {
  unit_type : string;
  instance_variables : (string * Yojson.Safe.t) list;
  next : transition;
}

type autotuner_graph = (string, state_node) Hashtbl.t

class virtual base_autotuner hash request instance_vars handler config_io device_config = object(self)
  inherit Base_acu.base_algorithmic_control_unit hash request instance_vars handler config_io device_config

  val graph = Hashtbl.create 16
  val histories = Hashtbl.create 16
  val counters = Hashtbl.create 16
  val mutable current_state_name = ""

  method add_state name node =
    Hashtbl.replace graph name node

  method get_history name =
    match Hashtbl.find_opt histories name with
    | Some h -> h
    | None ->
        let h = History.make name in
        Hashtbl.add histories name h;
        h

  method get_counter name =
    match Hashtbl.find_opt counters name with
    | Some c -> c
    | None ->
        failwith (Printf.sprintf "Counter %s not found" name)

  method add_counter name max =
    Hashtbl.replace counters name (Counter.make name max)

  method virtual initial_state : string

  method get_graph = graph

  method! program () =
    current_state_name <- self#initial_state;
    self#execution_loop ()

  method private execution_loop () =
    match Hashtbl.find_opt graph current_state_name with
    | None ->
        if current_state_name = "SUCCESS" then
          begin
            success <- true;
            result <- "Autotuning completed successfully";
            Lwt.return_unit
          end
        else if current_state_name = "FAILURE" then
          begin
            success <- false;
            result <- "Autotuning failed";
            Lwt.return_unit
          end
        else
          let msg = Printf.sprintf "State %s not found in Autotuner graph" current_state_name in
          self#log msg >>= fun () ->
          success <- false;
          result <- msg;
          Lwt.return_unit
    | Some node ->
        let req = { Acu_api.UnitRequest.message = (Printf.sprintf "Autotuner state: %s" current_state_name); demands = [] } in
        let inst_vars = node.instance_variables in
        
        self#log (Printf.sprintf "Autotuner: Entering state %s, spawning %s" current_state_name node.unit_type) >>= fun () ->
        
        self#request_subunits [req] [node.unit_type] [inst_vars] >>= fun responses ->
        let response = List.hd responses in
        
        (* Record history *)
        let h = self#get_history current_state_name in
        History.add h response;
        
        let next_state = if response.success then node.next.on_success else node.next.on_failure in
        current_state_name <- next_state;
        self#execution_loop ()
end
