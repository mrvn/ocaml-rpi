ocaml-rpi
=========

Exokernel to run ocaml baremetal on the Raspberry Pi


To test run make and copy the kerne.img to your SD card in place of
the linux kernel you normaly have on a RPi bootable card.

Or compile qemu with RPi patches [1], adjust the QEMU variable in the
Makefile and run "make && make test".

--
[1] https://github.com/Torlus/qemu.git
