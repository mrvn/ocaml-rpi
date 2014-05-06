type t

external init : unit -> unit = "ocaml_thread_init"

let () = init ()

external create : (unit -> unit) -> t = "caml_thread_create"
external yield : unit -> unit = "schedule"
external signal : int -> unit = "caml_thread_signal"
