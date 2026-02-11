(** UI Packet Subjects & Lifecycle Events *)

(** Primary NATS subject for UI-bound packets *)
let ui_packets = "UI_PACKETS"

(** ACU Lifecycle Events *)
let setup_start = "SETUP_START"
let setup_finished = "SETUP_FINISHED"
let program_start = "PROGRAM_START"
let program_finished = "PROGRAM_FINISHED"
let iteration_start = "ITERATION_START"
let iteration_finished = "ITERATION_FINISHED"

(** General Purpose Events *)
let custom_event = "CUSTOM_EVENT"
let error_event = "ERROR_EVENT"
let log_event = "LOG_EVENT"
