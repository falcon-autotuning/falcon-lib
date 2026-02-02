open Ctypes

type string_struct
let string_struct : string_struct structure typ = structure "string"
let string_raw = field string_struct "raw" (ptr char)
let string_length = field string_struct "length" size_t
let () = seal string_struct

let string_to_ocaml handle =
  if handle = null then ""
  else
    let s = !@ (coerce (ptr void) (ptr string_struct) handle) in
    let r = getf s string_raw in
    let l = getf s string_length in
    string_from_ptr r ~length:(Unsigned.Size_t.to_int l)

let to_number_compat v =
  let open Yojson.Safe.Util in
  match v with
  | `Int i -> float_of_int i
  | `Float f -> f
  | _ -> to_float v

module RUNTIME_COMMANDS = struct
  (* ... (rest of the RUNTIME_COMMANDS content) ... *)
  module BULK_DATABASE_ACCESS = struct
    let comm_channel = "BULK_DATABASE_ACCESS"
    let timestamp = "timestamp"
    let hash = "hash"
  end

  module BULK_DATABASE_RECEIVE = struct
    let comm_channel = "BULK_DATABASE_RECEIVE"
    let collection = "collection"
    let timestamp = "timestamp"
    let hash = "hash"
  end

  module ERASE_DATABASE = struct
    let comm_channel = "ERASE_DATABASE"
    let timestamp = "timestamp"
    let hash = "hash"
  end

  module LOAD = struct
    let comm_channel = "LOAD"
    let device_characteristic = "device_characteristic"
    let hash = "hash"
    let keys = "keys"
    let uncertainty = "uncertainty"
    let timestamp = "timestamp"
    let name = "name"
    let unitname = "unitname"
    let device_state = "device_state"
    let dbtime = "dbtime"
    let dbhash = "dbhash"
  end

  module LOG = struct
    let comm_channel = "LOG"
    let timestamp = "timestamp"
    let hash = "hash"
    let message = "message"
  end

  module MEASURE_COMMAND = struct
    let comm_channel = "MEASURE_COMMAND"
    let hash = "hash"
    let request = "request"
    let timestamp = "timestamp"
  end

  module MEASURE_RESPONSE = struct
    let comm_channel = "MEASURE_RESPONSE"
    let channel = "channel"
    let stream = "stream"
    let timestamp = "timestamp"
    let hash = "hash"
  end

  module PRINT_ANALYZER = struct
    let comm_channel = "PRINT_ANALYZER"
    let timestamp = "timestamp"
    let contents = "contents"
    let hash = "hash"
  end

  module PRINT_AUTOTUNER = struct
    let comm_channel = "PRINT_AUTOTUNER"
    let timestamp = "timestamp"
    let contents = "contents"
    let hash = "hash"
  end

  module PRINT_OPTIMIZER = struct
    let comm_channel = "PRINT_OPTIMIZER"
    let timestamp = "timestamp"
    let contents = "contents"
    let hash = "hash"
  end

  module READ_COMMAND = struct
    let comm_channel = "READ_COMMAND"
    let unitname = "unitname"
    let device_state = "device_state"
    let dbtime = "dbtime"
    let name = "name"
    let keys = "keys"
    let uncertainty = "uncertainty"
    let dbhash = "dbhash"
    let timestamp = "timestamp"
    let hash = "hash"
    let history_index = "history_index"
  end

  module READ_RESPONSE = struct
    let comm_channel = "READ_RESPONSE"
    let name = "name"
    let keys = "keys"
    let device_state = "device_state"
    let device_characteristic = "device_characteristic"
    let timestamp = "timestamp"
    let unitname = "unitname"
    let uncertainty = "uncertainty"
    let dbtime = "dbtime"
    let dbhash = "dbhash"
    let hash = "hash"
  end

  module SPAWN_COMMAND = struct
    let comm_channel = "SPAWN_COMMAND"
    let timestamp = "timestamp"
    let hash = "hash"
    let unit_type = "unit_type"
    let request = "request"
    let instance_variables = "instance_variables"
  end

  module SPAWN_RESPONSE = struct
    let comm_channel = "SPAWN_RESPONSE"
    let error = "error"
    let timestamp = "timestamp"
    let hash = "hash"
    let response = "response"
  end

  module STATE_REQUEST = struct
    let comm_channel = "STATE_REQUEST"
    let timestamp = "timestamp"
    let hash = "hash"
  end

  module STATE_RESPONSE = struct
    let comm_channel = "STATE_RESPONSE"
    let response = "response"
    let timestamp = "timestamp"
    let hash = "hash"
  end

  module UI_PACKETS = struct
    module ANALYZER_PKT = struct
      let comm_channel = "ANALYZER_PKT"
      let products = "products"
      let extra_products = "extra_products"
      let graphical_element = "graphical_element"
      let message = "message"
    end

    module AUTOTUNER_PKT = struct
      let comm_channel = "AUTOTUNER_PKT"
      let products = "products"
      let extra_products = "extra_products"
      let graphical_element = "graphical_element"
      let message = "message"
    end

    module COLLECTOR_PKT = struct
      let comm_channel = "COLLECTOR_PKT"
      let message = "message"
      let products = "products"
      let extra_products = "extra_products"
      let graphical_element = "graphical_element"
    end

    module OPTIMIZER_PKT = struct
      let comm_channel = "OPTIMIZER_PKT"
      let extra_products = "extra_products"
      let outputs = "outputs"
      let graphical_element = "graphical_element"
      let message = "message"
      let products = "products"
    end

    module LOAD_PKT = struct
      let comm_channel = "LOAD_PKT"
      let dbhash = "dbhash"
      let name = "name"
      let keys = "keys"
      let device_state = "device_state"
      let dbtime = "dbtime"
      let device_characteristic = "device_characteristic"
      let hash = "hash"
      let timestamp = "timestamp"
      let unitname = "unitname"
      let uncertainty = "uncertainty"
    end

    module SPAWN_CMD_PKT = struct
      let comm_channel = "SPAWN_CMD_PKT"
      let request = "request"
      let instance_variables = "instance_variables"
      let timestamp = "timestamp"
      let hash = "hash"
      let unit_type = "unit_type"
    end

    module SPAWN_RESP_PKT = struct
      let comm_channel = "SPAWN_RESP_PKT"
      let response = "response"
      let error = "error"
      let timestamp = "timestamp"
      let hash = "hash"
    end

    module STATE_PKT = struct
      let comm_channel = "STATE_PKT"
      let hash = "hash"
      let response = "response"
      let timestamp = "timestamp"
    end
  end
end

module InstanceVariables = struct
  type t = (string * Yojson.Safe.t) list

  let to_json t = `Assoc t
  let from_json json = match json with `Assoc l -> l | _ -> []
end

module UnitRequest = struct
  type t = {
    message : string;
    demands : (string * string) list;
  }

  let to_json t =
    `Assoc [
      ("message", `String t.message);
      ("demands", `Assoc (Stdlib.List.map (fun (k, v) -> (k, `String v)) t.demands));
    ]

  let from_json json =
    let open Yojson.Safe.Util in
    let to_string_option v = match v with `String s -> Some s | _ -> None in
    let to_assoc_option v = match v with `Assoc l -> Some l | _ -> None in
    {
      message = json |> member "message" |> to_string_option |> Option.value ~default:"";
      demands = json |> member "demands" |> to_assoc_option |> Option.value ~default:[] |> Stdlib.List.map (fun (k, v) -> (k, to_string v));
    }
end

module UnitResponse = struct
  type t = {
    message : string;
    products : (string * Yojson.Safe.t) list;
    success : bool;
  }

  let to_json t =
    `Assoc [
      ("message", `String t.message);
      ("products", `Assoc t.products);
      ("success", `Bool t.success);
    ]

  let from_json json =
    let open Yojson.Safe.Util in
    let to_string_option v = match v with `String s -> Some s | _ -> None in
    let to_assoc_option v = match v with `Assoc l -> Some l | _ -> None in
    {
      message = json |> member "message" |> to_string_option |> Option.value ~default:"";
      products = json |> member "products" |> to_assoc_option |> Option.value ~default:[];
      success = json |> member "success" |> (function `Bool b -> Some b | _ -> None) |> Option.value ~default:false;
    }
end
