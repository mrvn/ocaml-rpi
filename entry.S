/* entry.S - exception, interrupt and syscall entry point */
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

.section ".text"

.balign 32
.globl exception_table
exception_table:
	ldr	pc, addr_exception_reset
	ldr	pc, addr_exception_undefined
	ldr	pc, addr_exception_syscall
	ldr	pc, addr_exception_prefetch_abort
	ldr	pc, addr_exception_data_abort
	ldr	pc, addr_exception_reserved
	ldr	pc, addr_exception_irq
	ldr	pc, addr_exception_fiq

addr_exception_reset:		.word exception_reset
addr_exception_undefined:	.word exception_undefined
addr_exception_syscall:		.word exception_syscall
addr_exception_prefetch_abort:	.word exception_prefetch_abort
addr_exception_data_abort:	.word exception_data_abort
addr_exception_reserved:	.word exception_reserved
addr_exception_irq:		.word exception_irq
addr_exception_fiq:		.word exception_fiq

// Mapping between virtual and physical memory
#define PHYS_TO_VIRT 0xC0000000

.macro save, offset
        // Adjust LR and save it
        sub     lr, #\offset
        stmdb	sp!,{lr}
	// save all registers
        stmdb	sp,{r0-r14}^
	sub	sp, #60
	mov	r0, sp
.endm

.macro restore
	// restore all registers and return
        ldmia	sp,{r0-r14}^
	add	sp, #60
        ldmia	sp!,{pc}^
.endm

.balign 4
.globl exception_reset
exception_reset:
        save    4
	bl	exception_reset_handler
	// no way to return from reset
	// restore
	b	abort

.globl exception_undefined
exception_undefined:
        save    4
	bl	exception_undefined_handler
	b	abort

.globl exception_syscall
exception_syscall:
        save    0
	bl	exception_syscall_handler
	b	abort
	
.globl exception_prefetch_abort
exception_prefetch_abort:
        save    4
	bl	exception_prefetch_abort_handler
	b	abort

.globl exception_data_abort
exception_data_abort:
        save    8
	bl	exception_data_abort_handler
	b	abort

.globl	exception_reserved
exception_reserved:
	b	abort

.globl	exception_irq
exception_irq:
        save    4
	bl	exception_irq_handler
	restore

.globl	exception_fiq
exception_fiq:
        save    4
	bl	exception_fiq_handler
	b	abort
