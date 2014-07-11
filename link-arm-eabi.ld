/* link-arm-eabi.ld - linker script for arm eabi */
/* Copyright (C) 2013 Goswin von Brederlow <goswin-v-b@web.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

ENTRY(Start)

VIRT_BASE = 0xC0000000;

SECTIONS
{
    /* Starts at LOADER_ADDR. */
    /* . = 0xC0010000; */
    . = 0xC0008000;
    _start = .;
    _text_start = .;
    .text : AT(ADDR(.text) - VIRT_BASE) {
        KEEP(*(.text.boot))
        *(.text)
        *(.text.*)
    }
    . = ALIGN(4096); /* align to page size */
    _text_end = .;
    /* .ARM.exidx is sorted, so has to go in its own output section. */
    __exidx_start = .;
    .ARM.exidx : AT(ADDR(.ARM.exidx) - VIRT_BASE) {
    	 *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    }
    __exidx_end = .;
    _rodata_start = .;
    .rodata : AT(ADDR(.rodata) - VIRT_BASE) {
	*(SORT_BY_ALIGNMENT(.rodata))
	*(SORT_BY_ALIGNMENT(.rodata.*))
    }
    .init_array ALIGN(4) : AT(ADDR(.init_array) - VIRT_BASE) {
    _init_array_start = .;
        *(.init_array)
        *(SORT_BY_INIT_PRIORITY(.init_array.*))
    _init_array_end = .;
    }
    . = ALIGN(4096); /* align to page size */
    _rodata_end = .;
    _data_start = .;
    .data : AT(ADDR(.data) - VIRT_BASE) {
        *(SORT_BY_ALIGNMENT(.data))
        *(SORT_BY_ALIGNMENT(.data.*))
    }
    .got : AT(ADDR(.got) - VIRT_BASE) { *(.got.plt) *(.got) }
    .dynamic : AT(ADDR(.dynamic) - VIRT_BASE) { *(.dynamic) }
    _data_end = .;
    .bss ALIGN(4) : AT(ADDR(.bss) - VIRT_BASE) {
        KEEP(*(.bss.prebss.*))
        . = ALIGN(4);
    	_bss_start = .;
	*(SORT_BY_ALIGNMENT(.bss))
	*(SORT_BY_ALIGNMENT(.bss.*))
        *(.gnu.linkonce.b.*)
        *(COMMON)
	. = ALIGN(4096); /* align to page size */
	_bss_end = .;
	_stack_start = .;
	. = _stack_start + 1024*1024;
	_stack_end = .;
	_fiq_stack_start = .;
	. = _fiq_stack_start + 16384;
	_fiq_stack_end = .;
	_irq_stack_start = .;
	. = _irq_stack_start + 16384;
	_irq_stack_end = .;
	_svc_stack_start = .;
	. = _svc_stack_start + 16384;
	_svc_stack_end = .;
	_abt_stack_start = .;
	. = _abt_stack_start + 16384;
	_abt_stack_end = .;
	_und_stack_start = .;
	. = _und_stack_start + 16384;
	_und_stack_end = .;
    }
    
    _end = .;
    /* Strip unnecessary stuff */
    /DISCARD/ : { *(*) }
}
