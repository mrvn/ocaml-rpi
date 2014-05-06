let rec fac = function
  | 1 -> 1
  | n -> n * fac (n-1)

let () = ignore (fac 10)
let () = Printf.printf "Hello World\n%!"
let handler num =
  (* Printf.printf "Signal number %d\n%!" num;
  *)
   Thread.yield();
  ()
    
let () = Sys.set_signal 0 (Sys.Signal_handle handler)
  
let rec fib lst = function
  | 0 -> (1, "0", lst)
  | 1 -> (1, "1", lst)
  | n ->
    let s = Printf.sprintf "%d" n in
    let (n1, _, lst) = fib (s::lst) (n - 1) in
    let (n2, _, lst) = fib (s::lst) (n - 2) in
    (n1 + n2, s, lst)
  
let rec loop n =
  let (res, s, _) = fib [] n
  in
  Printf.printf "[%s] fib(%s) = %d\n%!" (Time.to_string (Time.time ())) s res;
(*
  Thread.yield();
  Thread.signal 0;
*)
  loop (n + 1)

let rec loop2 n =
  let (res, s, _) = fib [] n
  in
  Printf.printf "[%s] fib2(%s) = %d\n%!" (Time.to_string (Time.time ())) s res;
(*
  Thread.yield();
  Thread.signal 0;
*)
  loop2 (n + 1)

let t = Thread.create (fun () -> loop2 1)
let t = Thread.create (fun () -> loop 1)

let rec loop3 n =
  Printf.printf "[%s] loop3 %d\n%!" (Time.to_string (Time.time ())) n;
(*
  Thread.yield ();
  Gc.major ();
  Gc.compact ();
*)  let stat = Gc.stat ()
  in
  Printf.printf "live_words = %d\n%!" stat.Gc.live_words;
  loop3 (n+1)

let () =
  Gc.set { (Gc.get()) with Gc.verbose = 0x00d };
  loop3 1
