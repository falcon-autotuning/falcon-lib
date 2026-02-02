(lang dune 3.0)
(name falcon)
(generate_opam_files true)
(package
 (name falcon)
 (synopsis "OCaml implementation of Falcon ACUs")
 (depends
  (ocaml (>= 4.14.0))
  (dune (>= 3.0))
  lwt
  lwt_ppx
  yojson
  ctypes
  ctypes-foreign
  pyml
  falcon_core
  (alcotest :with-test)))
