

module SignalMapper = struct
  (** 
    Utility for resolving semantic signal keys from raw measurement data
    using a provided mapping configuration.
  *)

  let get_signal signals mapping semantic_name =
    let raw_key = 
      match mapping with
      | `Assoc _ -> 
          (match Yojson.Safe.Util.member semantic_name mapping with
          | `String s -> s
          | _ -> semantic_name)
      | _ -> semantic_name
    in
    match signals with
    | `Assoc _ ->
        (match Yojson.Safe.Util.member raw_key signals with
        | `List l ->
            let arr = List.map Yojson.Safe.Util.to_float l |> Array.of_list in
            let n = Array.length arr in
            let mat = Owl_base_dense_ndarray_d.create [|1; n|] 0.0 in
            Array.iteri (fun i v -> Owl_base_dense_ndarray_d.set mat [|0; i|] v) arr;
            Some mat
        | _ -> None)
    | _ -> None

  let get_contextual_value context key default =
    match context with
    | `Assoc _ ->
        (* check dependencies first *)
        let deps = Yojson.Safe.Util.member "dependencies" context in
        let dep = match deps with `Assoc _ -> Yojson.Safe.Util.member key deps | _ -> `Null in
        (match dep with
        | `Assoc _ ->
            (match Yojson.Safe.Util.member "value" dep with
            | `Null -> 
                (* fall back to instance_vars *)
                let ivars = Yojson.Safe.Util.member "instance_vars" context in
                let iv = match ivars with `Assoc _ -> Yojson.Safe.Util.member key ivars | _ -> `Null in
                if iv = `Null then default else iv
            | v -> v)
        | _ -> 
            let ivars = Yojson.Safe.Util.member "instance_vars" context in
            let iv = match ivars with `Assoc _ -> Yojson.Safe.Util.member key ivars | _ -> `Null in
            if iv = `Null then default else iv)
    | _ -> default
end

module OwlUtils = struct
  let to_ndarray json =
    match json with
    | `List l ->
        let arr = List.map Yojson.Safe.Util.to_float l |> Array.of_list in
        let n = Array.length arr in
        let mat = Owl_base_dense_ndarray_d.create [|1; n|] 0.0 in
        Array.iteri (fun i v -> Owl_base_dense_ndarray_d.set mat [|0; i|] v) arr;
        mat
    | _ -> Owl_base_dense_ndarray_d.create [|0; 0|] 0.0

  let of_ndarray mat =
    let n = Owl_base_dense_ndarray_d.numel mat in
    let arr = Array.init n (fun i -> Owl_base_dense_ndarray_d.get mat [|0; i|]) in
    `List (Array.to_list arr |> List.map (fun f -> `Float f))
end
