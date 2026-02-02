open Ctypes
open Foreign

(* Robust library loading *)
let lib_handle =
  let paths = [
    "/usr/local/lib/libnats.so";
    "/usr/lib/libnats.so";
    "libnats.so"
  ] in
  let rec try_load = function
    | [] -> None
    | p :: rest ->
        try 
          (* Using RTLD_LAZY to be safer *)
          let h = Dl.dlopen ~filename:p ~flags:[Dl.RTLD_LAZY] in
          Some h
        with _ -> try_load rest
  in
  try_load paths

let is_available () = Option.is_some lib_handle

(* Helper to define foreign functions that fail gracefully if library is missing *)
let safe_foreign name signature =
  match lib_handle with
  | Some h -> 
      (try foreign ~from:h name signature
       with _ -> 
         (* Return a dummy that fails when called *)
         fun _ -> failwith (Printf.sprintf "NATS symbol %s not found in library" name))
  | None -> 
      (* Return a dummy that fails when called *)
      fun _ -> failwith (Printf.sprintf "NATS function %s called but libnats.so not found" name)

(* Types *)
type nats_connection = unit ptr
let nats_connection : nats_connection typ = ptr void

type nats_subscription = unit ptr
let nats_subscription : nats_subscription typ = ptr void

type nats_msg = unit ptr
let nats_msg : nats_msg typ = ptr void

type nats_status = int
let nats_status : nats_status typ = int

let nats_ok = 0

(* natsMsg functions *)
let natsMsg_GetData = safe_foreign "natsMsg_GetData" (nats_msg @-> returning string)
let natsMsg_GetDataLength = safe_foreign "natsMsg_GetDataLength" (nats_msg @-> returning int)
let natsMsg_GetSubject = safe_foreign "natsMsg_GetSubject" (nats_msg @-> returning string)
let natsMsg_GetReply = safe_foreign "natsMsg_GetReply" (nats_msg @-> returning string)
let natsMsg_Destroy = safe_foreign "natsMsg_Destroy" (nats_msg @-> returning void)

(* Callback type for subscriptions *)
type nats_msg_handler = nats_connection -> nats_subscription -> nats_msg -> unit ptr -> unit
let nats_msg_handler = funptr (nats_connection @-> nats_subscription @-> nats_msg @-> ptr void @-> returning void)

(* Connection functions *)
let natsConnection_ConnectTo = 
  match lib_handle with
  | Some h -> (try foreign ~from:h "natsConnection_ConnectTo" (ptr nats_connection @-> string @-> returning nats_status) with _ -> fun _ _ -> failwith "natsConnection_ConnectTo missing")
  | None -> fun _ _ -> failwith "natsConnection_ConnectTo: libnats not found"

let natsConnection_Publish = 
  match lib_handle with
  | Some h -> (try foreign ~from:h "natsConnection_Publish" (nats_connection @-> string @-> ptr void @-> int @-> returning nats_status) with _ -> fun _ _ _ _ -> failwith "natsConnection_Publish missing")
  | None -> fun _ _ _ _ -> failwith "natsConnection_Publish: libnats not found"

let natsConnection_PublishString = 
  match lib_handle with
  | Some h -> (try foreign ~from:h "natsConnection_PublishString" (nats_connection @-> string @-> string @-> returning nats_status) with _ -> fun _ _ _ -> failwith "natsConnection_PublishString missing")
  | None -> fun _ _ _ -> failwith "natsConnection_PublishString: libnats not found"

let natsConnection_Subscribe = 
  match lib_handle with
  | Some h -> (try foreign ~from:h "natsConnection_Subscribe" (ptr nats_subscription @-> nats_connection @-> string @-> nats_msg_handler @-> ptr void @-> returning nats_status) with _ -> fun _ _ _ _ _ -> failwith "natsConnection_Subscribe missing")
  | None -> fun _ _ _ _ _ -> failwith "natsConnection_Subscribe: libnats not found"

let natsConnection_SubscribeSync = 
  match lib_handle with
  | Some h -> (try foreign ~from:h "natsConnection_SubscribeSync" (ptr nats_subscription @-> nats_connection @-> string @-> returning nats_status) with _ -> fun _ _ _ -> failwith "natsConnection_SubscribeSync missing")
  | None -> fun _ _ _ -> failwith "natsConnection_SubscribeSync: libnats not found"

let natsConnection_RequestString = 
  match lib_handle with
  | Some h -> (try foreign ~from:h "natsConnection_RequestString" (ptr nats_msg @-> nats_connection @-> string @-> string @-> int64_t @-> returning nats_status) with _ -> fun _ _ _ _ _ -> failwith "natsConnection_RequestString missing")
  | None -> fun _ _ _ _ _ -> failwith "natsConnection_RequestString: libnats not found"

let natsConnection_Close = 
  match lib_handle with
  | Some h -> (try foreign ~from:h "natsConnection_Close" (nats_connection @-> returning void) with _ -> fun _ -> failwith "natsConnection_Close missing")
  | None -> fun _ -> failwith "natsConnection_Close: libnats not found"

let natsConnection_Destroy = 
  match lib_handle with
  | Some h -> (try foreign ~from:h "natsConnection_Destroy" (nats_connection @-> returning void) with _ -> fun _ -> failwith "natsConnection_Destroy missing")
  | None -> fun _ -> failwith "natsConnection_Destroy: libnats not found"

(* Subscription functions *)
let natsSubscription_NextMsg = 
  match lib_handle with
  | Some h -> (try foreign ~from:h "natsSubscription_NextMsg" (ptr nats_msg @-> nats_subscription @-> int64_t @-> returning nats_status) with _ -> fun _ _ _ -> failwith "natsSubscription_NextMsg missing")
  | None -> fun _ _ _ -> failwith "natsSubscription_NextMsg: libnats not found"

let natsSubscription_Unsubscribe = 
  match lib_handle with
  | Some h -> (try foreign ~from:h "natsSubscription_Unsubscribe" (nats_subscription @-> returning nats_status) with _ -> fun _ -> failwith "natsSubscription_Unsubscribe missing")
  | None -> fun _ -> failwith "natsSubscription_Unsubscribe: libnats not found"

let natsSubscription_Destroy = 
  match lib_handle with
  | Some h -> (try foreign ~from:h "natsSubscription_Destroy" (nats_subscription @-> returning void) with _ -> fun _ -> failwith "natsSubscription_Destroy missing")
  | None -> fun _ -> failwith "natsSubscription_Destroy: libnats not found"
