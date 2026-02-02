(** Wraps and unwraps Falcon_core strings/JSON for ACU consumption *)

let wrap_response (raw : string) = Falcon_core.String.wrap raw

(** Simple threshold check *)
let exceeds_threshold value threshold = value > threshold

(** Basic signal preprocessing Helpers (delegated to Python for complex logic) *)
let calculate_offset data =
  (* Simple mean for now, complex logic remains in Python scripts *)
  if Array.length data = 0 then 0.0
  else (Array.fold_left (+.) 0.0 data) /. (float_of_int (Array.length data))

let apply_offset data offset =
  Array.map (fun v -> v -. offset) data
