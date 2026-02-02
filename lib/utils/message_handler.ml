open Ctypes
open Nats_bindings
open Lwt.Infix

module MessageHandler = struct
  type transport =
    | Nats of nats_connection
    | Mock of {
        publish : string -> string -> unit Lwt.t;
        request : string -> string -> int -> string Lwt.t;
        subscribe : string -> (string -> unit Lwt.t) -> unit Lwt.t;
        receive_large_message : string -> string -> string Lwt.t;
      }

  type t = {
    transport : transport;
    url : string;
  }

  let connect url =
    let nc_ptr = Ctypes.allocate nats_connection Ctypes.null in
    let status = natsConnection_ConnectTo nc_ptr url in
    if status = nats_ok then
      Lwt.return { transport = Nats (!@ nc_ptr); url }
    else
      Lwt.fail_with (Printf.sprintf "Failed to connect to NATS at %s (status: %d)" url status)

  let create_mock ~publish ~request ~subscribe ~receive_large_message =
    { transport = Mock { publish; request; subscribe; receive_large_message }; url = "mock://" }

  let publish t channel message =
    match t.transport with
    | Nats nc ->
        let status = natsConnection_PublishString nc channel message in
        if status = nats_ok then Lwt.return_unit
        else Lwt.fail_with (Printf.sprintf "Failed to publish to %s (status: %d)" channel status)
    | Mock m -> m.publish channel message

  let request t channel message timeout_ms =
    match t.transport with
    | Nats nc ->
        let msg_ptr = allocate nats_msg null in
        let status = natsConnection_RequestString msg_ptr nc channel message (Int64.of_int timeout_ms) in
        if status = nats_ok then
          let msg = !@ msg_ptr in
          let data = natsMsg_GetData msg in
          natsMsg_Destroy msg;
          Lwt.return data
        else
          Lwt.fail_with (Printf.sprintf "NATS request to %s failed or timed out (status: %d)" channel status)
    | Mock m -> m.request channel message timeout_ms

  let receive t channel timeout_ms =
    match t.transport with
    | Nats nc ->
        let sub_ptr = allocate nats_subscription null in
        let status = natsConnection_SubscribeSync sub_ptr nc channel in
        if status <> nats_ok then
          Lwt.fail_with (Printf.sprintf "Failed to subscribe to %s (status: %d)" channel status)
        else
          let sub = !@ sub_ptr in
          let msg_ptr = allocate nats_msg null in
          let status = natsSubscription_NextMsg msg_ptr sub (Int64.of_int timeout_ms) in
          if status = nats_ok then
            let msg = !@ msg_ptr in
            let data = natsMsg_GetData msg in
            natsMsg_Destroy msg;
            ignore (natsSubscription_Unsubscribe sub);
            natsSubscription_Destroy sub;
            Lwt.return data
          else
            (ignore (natsSubscription_Unsubscribe sub);
             natsSubscription_Destroy sub;
             Lwt.fail_with (Printf.sprintf "Timed out waiting for message on %s (status: %d)" channel status))
    | Mock _ -> Lwt.fail_with "receive (sync subscribe) not implemented for Mock transport yet"

  let request_with_custom_response t request_channel message response_channel timeout_ms =
    match t.transport with
    | Nats _ ->
        let sub_ptr = allocate nats_subscription null in
        let nc = match t.transport with Nats nc -> nc | _ -> assert false in
        let status = natsConnection_SubscribeSync sub_ptr nc response_channel in
        if status <> nats_ok then
          Lwt.fail_with (Printf.sprintf "Failed to subscribe to %s (status: %d)" response_channel status)
        else
          let sub = !@ sub_ptr in
          publish t request_channel message >>= fun () ->
          let msg_ptr = allocate nats_msg null in
          let status = natsSubscription_NextMsg msg_ptr sub (Int64.of_int timeout_ms) in
          if status = nats_ok then
            let msg = !@ msg_ptr in
            let data = natsMsg_GetData msg in
            natsMsg_Destroy msg;
            ignore (natsSubscription_Unsubscribe sub);
            natsSubscription_Destroy sub;
            Lwt.return data
          else
            (ignore (natsSubscription_Unsubscribe sub);
             natsSubscription_Destroy sub;
             Lwt.fail_with (Printf.sprintf "Timed out waiting for response on %s (status: %d)" response_channel status))
    | Mock m ->
        (* In mock mode, we assume the mock request handler handles the correlation or just returns the response directly *)
        m.request request_channel message timeout_ms

  let close t =
    match t.transport with
    | Nats nc ->
        natsConnection_Close nc;
        natsConnection_Destroy nc;
        Lwt.return_unit
    | Mock _ -> Lwt.return_unit

  let receive_large_message t channel stream =
    match t.transport with
    | Nats _ -> Lwt.fail_with "Jetstream/Large message support not implemented yet in OCaml"
    | Mock m -> m.receive_large_message channel stream

  (* Note: subscription is more complex because it involves callbacks bridging to Lwt.
     We might need a global state or a way to map nats_subscription to Lwt.wait/wakeup. *)
end

module ConfigIO = struct
  type endpoint = {
    receive_channel : string option;
    receive_timeout : float;
  }

  type t = {
    url : string;
    map : (string, endpoint) Hashtbl.t;
  }

  let create_empty () = { url = ""; map = Hashtbl.create 1 }

  let from_json json =
    let open Yojson.Safe.Util in
    let to_string_option v = match v with `String s -> Some s | _ -> None in
    let to_assoc_option v = match v with `Assoc l -> Some l | _ -> None in
    let url = json |> member "url" |> to_string_option |> Option.value ~default:"" in
    let raw_map = json |> member "map" |> to_assoc_option |> Option.value ~default:[] in
    let map = Hashtbl.create (List.length raw_map) in
    let to_float_compat v =
      match v with
      | `Int i -> float_of_int i
      | `Float f -> f
      | _ -> to_float v (* Fallback to default which might raise error *)
    in
    List.iter (fun (k, v) ->
      let endpoint = {
        receive_channel = v |> member "receive_channel" |> to_string_option;
        receive_timeout = (try v |> member "receive_timeout" |> to_float_compat with _ -> 1.0);
      } in
      Hashtbl.add map k endpoint
    ) raw_map;
    { url; map }

  let get_endpoint t key =
    Hashtbl.find_opt t.map key

  let get_receive_timeout t key =
    match get_endpoint t key with
    | Some e -> e.receive_timeout
    | None -> 1.0
end
