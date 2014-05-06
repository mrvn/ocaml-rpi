type t = { tv_sec:int; tv_usec:int; }

external init : unit -> unit = "caml_time_init"
external time : unit -> t = "caml_time_time"

let () = init ()

let print time = Printf.printf "%d.%06d" time.tv_sec time.tv_usec
let to_string time = Printf.sprintf "%d.%06d" time.tv_sec time.tv_usec
