(* Framebuffer.ml - ocaml framebuffer interface
 * Copyright (C) 2013 Goswin von Brederlow <goswin-v-b@web.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --
 *
 * Access to the Raspberry Pi framebuffer
 *)

type result =
| SUCCESS
(* Error codes *)
| FAIL_GET_RESOLUTION
| FAIL_GOT_INVALID_RESOLUTION
| FAIL_SETUP_FRAMEBUFFER
| FAIL_INVALID_TAGS
| FAIL_INVALID_TAG_RESPONSE
| FAIL_INVALID_TAG_DATA
| FAIL_INVALID_PITCH_RESPONSE
| FAIL_INVALID_PITCH_DATA

let to_string = function
  | SUCCESS -> "SUCCESS"
  (* Error codes *)
  | FAIL_GET_RESOLUTION -> "FAIL_GET_RESOLUTION"
  | FAIL_GOT_INVALID_RESOLUTION -> "FAIL_GOT_INVALID_RESOLUTION"
  | FAIL_SETUP_FRAMEBUFFER -> "FAIL_SETUP_FRAMEBUFFER"
  | FAIL_INVALID_TAGS -> "FAIL_INVALID_TAGS"
  | FAIL_INVALID_TAG_RESPONSE -> "FAIL_INVALID_TAG_RESPONSE"
  | FAIL_INVALID_TAG_DATA -> "FAIL_INVALID_TAG_DATA"
  | FAIL_INVALID_PITCH_RESPONSE -> "FAIL_INVALID_PITCH_RESPONSE"
  | FAIL_INVALID_PITCH_DATA -> "FAIL_INVALID_PITCH_DATA"

external init : unit -> result = "ocaml_rpi__fb_init"

let () =
  let res = init ()
  in
  Printf.printf "Framebuffer.init () -> %s\n%!" (to_string res)
