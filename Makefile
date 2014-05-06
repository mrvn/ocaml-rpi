DEPENDFLAGS := -MD -MP
INCLUDES    := -I .
BASEFLAGS   := -O2 -fpic -nostdlib -std=gnu99
BASEFLAGS   += -ffreestanding -fomit-frame-pointer
BASEFLAGS   += -D_FILE_OFFSET_BITS=64
CPUFLAGS    := -mcpu=arm1176jzf-s -marm -mhard-float -mfpu=vfp
WARNFLAGS   := -Wall -Wextra -Wshadow -Wcast-align -Wwrite-strings
WARNFLAGS   += -Wredundant-decls -Winline
WARNFLAGS   += -Wno-endif-labels -Wfloat-equal
WARNFLAGS   += -Wformat=2 -Winit-self
WARNFLAGS   += -Winvalid-pch -Wmissing-format-attribute
WARNFLAGS   += -Wmissing-include-dirs
WARNFLAGS   += -Wredundant-decls -Wshadow
WARNFLAGS   += -Wswitch -Wsystem-headers
WARNFLAGS   += -Wno-pragmas
WARNFLAGS   += -Wno-psabi
WARNFLAGS   += -Wwrite-strings -Wdisabled-optimization -Wpointer-arith
WARNFLAGS   += -Werror -Wno-error=unused-variable -Wno-error=unused-parameter
WARNFLAGS   += -Wno-error=unused-function
ASFLAGS     := $(INCLUDES) $(DEPENDFLAGS) $(CPUFLAGS) -D__ASSEMBLY__
ALLFLAGS    := $(BASEFLAGS) $(WARNFLAGS) $(INCLUDES) $(DEPENDFLAGS)
ALLFLAGS    += $(CPUFLAGS) 

CFLAGS      := $(ALLFLAGS) -Wstrict-prototypes -Wnested-externs -Winline
CXXFLAGS    := $(ALLFLAGS) -fno-exceptions -std=c++11
LDFLAGS     := $(BASEFLAGS)

CC := gcc
OBJCOPY := objcopy
OBJDUMP := objdump

all: kernel.img kernel.symbols tests

# Basic patterns
%.o: %.S
	$(CC) $(ASFLAGS) -MT $@ -MF $@.d -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -MT $@ -MF $@.d -c $< -o $@

ocaml.o: Thread.ml Time.ml Framebuffer.ml foo.ml
#	ocamlopt -output-obj -o $@ -thread unix.cmxa threads.cmxa $+
	ocamlopt -output-obj -o $@ $+

kernel.elf: boot.o entry.o uart.o printf.o string.o memory.o main.o Thread_stubs.o Time_stubs.o Framebuffer_stubs.o ocaml.o
#	$(CC) -nostdlib -ffreestanding -o $@ $+ -L/usr/lib/ocaml -lasmrun
#	$(CC) -o $@ $+ -L/usr/lib/ocaml -lasmrun
	$(CC) $(LDFLAGS) -Tlink-arm-eabi.ld -o $@ $+ -L/usr/lib/ocaml -lasmrun -lunix -L . -lgcc

%.img: %.elf
	$(OBJCOPY) -R .bss $< -O binary $@

%.symbols: %.elf
	$(OBJDUMP) -t $< | sort >$@

clean:
	rm -f *.o *.cmx *.cmi *.elf *.img *.symbols *~
	rm -f test/liist test/memory

# Include depends
include $(wildcard *.d) $(wildcard test/*.d)

QEMU = ../../qemu/install/bin/qemu-system-arm
test:
	$(QEMU) -kernel kernel.elf -cpu arm1176 -m 512 -M raspi -serial stdio -device usb-kbd

tests: test/list test/memory

test/%: test/%.c
	$(CC) -std=gnu99 -O2 -W -Wall -Wextra -Werror -g -MD -MP -MT $@ -MF $@.d -o $@ $<

.PHONY: test
