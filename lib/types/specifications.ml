open Falcon_core


module DeviceCharacteristic = struct
  type t = {
    mutable name : string;
    mutable indexes : string list;
    mutable hash : string;
    mutable time : int;
    mutable state : DeviceVoltageStates.t;
    mutable unit_type : string;
    mutable characteristic : Yojson.Safe.t;
    mutable uncertainty : float;
  }

  let make ?(state) ?(characteristic=`Null) ?(hash="") ?(time=0) ?(unit_type="") ?(uncertainty= -1.0) name indexes =
    let state = match state with
      | Some s -> s
      | None -> DeviceVoltageStates.new_empty ()
    in
    {
      name;
      indexes;
      hash;
      time;
      state;
      unit_type;
      characteristic;
      uncertainty;
    }

  let copy t =
    { t with state = DeviceVoltageStates.from_json_string (DeviceVoltageStates.to_json_string t.state#raw) }

  let to_json t =
    `Assoc [
      ("name", `String t.name);
      ("indexes", `List (Stdlib.List.map (fun s -> `String s) t.indexes));
      ("hash", `String t.hash);
      ("time", `Int t.time);
      ("state", Yojson.Safe.from_string (Acu_api.string_to_ocaml (DeviceVoltageStates.to_json_string t.state#raw)));
      ("unit_type", `String t.unit_type);
      ("characteristic", t.characteristic);
      ("uncertainty", `Float t.uncertainty);
    ]
end
