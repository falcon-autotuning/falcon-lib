open Falcon_core

let create_simple_config () =
  let screening_gates = Connections.from_list [Connection.new_screening_gate (String.of_string "S1"); Connection.new_screening_gate (String.of_string "S2")] in
  let barrier_gates = Connections.from_list [Connection.new_barrier_gate (String.of_string "B1"); Connection.new_barrier_gate (String.of_string "B2"); Connection.new_barrier_gate (String.of_string "B3")] in
  let reservoir_gates = Connections.from_list [Connection.new_reservoir_gate (String.of_string "R1"); Connection.new_reservoir_gate (String.of_string "R2")] in
  let plunger_gates = Connections.from_list [Connection.new_plunger_gate (String.of_string "P1"); Connection.new_plunger_gate (String.of_string "P2")] in
  let ohmics = Connections.from_list [Connection.new_ohmic (String.of_string "O1"); Connection.new_ohmic (String.of_string "O2")] in

  let order = Connections.from_list [
    Connection.new_ohmic (String.of_string "O1");
    Connection.new_reservoir_gate (String.of_string "R1");
    Connection.new_barrier_gate (String.of_string "B1");
    Connection.new_plunger_gate (String.of_string "P1");
    Connection.new_barrier_gate (String.of_string "B2");
    Connection.new_plunger_gate (String.of_string "P2");
    Connection.new_barrier_gate (String.of_string "B3");
    Connection.new_reservoir_gate (String.of_string "R2");
    Connection.new_ohmic (String.of_string "O2");
  ] in

  let group1 = Group.make 
    (Channel.of_string "group1")#raw 
    2 
    screening_gates#raw 
    reservoir_gates#raw 
    plunger_gates#raw 
    barrier_gates#raw 
    order#raw 
  in

  let groups = MapGnameGroup.new_empty () in
  MapGnameGroup.insert groups#raw (Gname.of_string "group1")#raw group1#raw;

  let wiring_dc = Impedances.new_empty () in
  
  let adjacency = Adjacency.of_arrays ~data:[|0;0;0;0|] ~shape:[|2;2|] screening_gates in
  let bounds = PairDoubleDouble.make (-2.0) 2.0 in
  let constraints = VoltageConstraints.make adjacency#raw 0.5 bounds#raw in

  Config.create
    ~screening_gates ~plunger_gates ~ohmics ~barrier_gates ~reservoir_gates 
    ~groups ~wiring_dc ~constraints
