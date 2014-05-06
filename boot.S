/* boot.S - assembly startup code */
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

// Mapping between virtual and physical memory
#define PHYS_TO_VIRT 0xC0000000

// To keep this in the first portion of the binary.
.section ".text.boot"

.globl Start
Start:
// Entry point for the kernel
// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42
// r2 -> 0x00000100 - start of ATAGS
// preserve these registers as argument for kernel_main

	/****************************************************************
	 * Map kernel to 0xC0000000 and enable paging			*
	 ****************************************************************/
	// create kernel pagetable at 0x4000
	mov	r3, #0x4000		// kernel page table
	ldr	r4, =(memory_regions - PHYS_TO_VIRT)	// 5 regions of memory
	mov	r5, #5

.L1: // LOOP: regions of memory
	ldmia	r4!, {r6, r7, r8}	// load val, incr and count

.L2: // LOOP: count
	mov	r9, #16			// repeat entry 16 times

.L3: // LOOP: 16
	stmia	r3!, {r6}
	subs	r9, r9, #1		// LOOP: 16
	bne	.L3
	
	add	r6, r6, r7		// val += incr
	subs	r8, r8, #1		// LOOP: count
	bne	.L2
	
	subs	r5, r5, #1		// LOOP: regions of memory
	bne	.L1

	
	// Translation Table Base Register 0
	mov	r3, #0x4000	// kernel page table
	mcr	p15, 0, r3, c2, c0, 0 // Write TTBR0

	// Translation Table Base Register 1
	mcr	p15, 0, r3, c2, c0, 1 // Write TTBR1

	// c2, Translation Table Base Control Register
	ldr	r4, =2 // 4K Table 0
	mcr	p15, 0, r4, c2, c0, 2 // Write TTBCR

	// setup domains (CP15 c3)
	ldr	r4, =0x55555555 // use access permissions from TLB entry
	mcr	p15, 0, r4, c3, c0, 0 // Write Domain Access Control Register	

	// c1 control register
	// 29: Force AP disabled (0)
	// 28: TEX remap disabled (0)
	// 23: Subpages AP bits disabled (1)
	// 13: vector base address for exceptions selected (0)
	// 12: enable L1 instruction cache (1)
	// 11: enable branch prediction (1)
	//  9: rom protection off (0)
	//  8: S bit off (0)
	//  2: enable data cache (1)
	//  1: strict alignment (1)
	//  0: enable MMU (1)
	ldr	r5, =0xCF7FCBF8
	ldr	r6, =0x00801807
	mrc	p15, 0, r4, c1, c0, 0 // Read Control Register
	and	r4, r4, r5
	orr	r4, r4, r6
	mcr	p15, 0, r4, c1, c0, 0 // Write Control Register

	/****************************************************************
	 * Switch to higher half (0xC0000000)				*
	 ****************************************************************/
	ldr	pc, =higher_half

higher_half:
	// set stack for fiq mode
	mrs	r4, cpsr	// get current mode
	bic	r4, r4, #0x1f	// blank mode
	orr	r5, r4, #0x11	// fiq mode
	msr	cpsr_c, r5
	ldr	sp, =_fiq_stack_end	// top of stack

	// set stack for irq mode
	mrs	r4, cpsr	// get current mode
	bic	r4, r4, #0x1f	// blank mode
	orr	r5, r4, #0x12	// irq mode
	msr	cpsr_c, r5
	ldr	sp, =_irq_stack_end	// top of stack

	// set stack for svc mode
	mrs	r4, cpsr	// get current mode
	bic	r4, r4, #0x1f	// blank mode
	orr	r5, r4, #0x13	// svc mode
	msr	cpsr_c, r5
	ldr	sp, =_svc_stack_end	// top of stack

	// set stack for abort mode
	mrs	r4, cpsr	// get current mode
	bic	r4, r4, #0x1f	// blank mode
	orr	r5, r4, #0x17	// abt mode
	msr	cpsr_c, r5
	ldr	sp, =_abt_stack_end	// top of stack

	// set stack for undefined mode
	mrs	r4, cpsr	// get current mode
	bic	r4, r4, #0x1f	// blank mode
	orr	r5, r4, #0x1b	// und mode
	msr	cpsr_c, r5
	ldr	sp, =_und_stack_end	// top of stack

	// run in system mode from here on
	mrs	r4, cpsr	// get current mode
	bic	r4, r4, #0x1f	// blank mode
	orr	r5, r4, #0x1f	// sys mode
	msr	cpsr_c, r5

	// Setup the stack.
	ldr	sp, =_stack_end	// top of stack

	// Clear out bss.
	ldr	r4, =_bss_start
	ldr	r9, =_bss_end
	mov	r5, #0
	mov	r6, #0
	mov	r7, #0
	mov	r8, #0
1:	// store multiple at r4.
	stmia	r4!, {r5-r8}
	// If we're still below bss_end, loop.
	cmp	r4, r9
	blo	1b

        // enable the FPU
	mov     r5, #0
	mrc     p15, 0, r5, c1, c0, 2
	orr     r5, r5, #0x300000            /* single precision */
	orr     r5, r5, #0xC00000            /* double precision */
	mcr     p15, 0, r5, c1, c0, 2
	mov     r5, #0x40000000
	fmxr    fpexc,r5

	/****************************************************************
	 * Turn on OK LED						*
	 ****************************************************************/
	// Base of GPIO registers
	ldr	r4, =0xE0200000
	// enable pin 16 as output
	ldr	r6, [r4, #4]
	mov	r5, #1
	lsl	r5, #18
	orr	r6, r6, r5
	str	r6, [r4, #4]
	// turn on LED
	mov	r5, #1
	lsl	r5, #16
	str	r5, [r4, #40]

	/****************************************************************
         * Call constructors                                            *
         ****************************************************************/
	push    {r0-r2}
	ldr     r3, =kernel_constructors
	blx     r3
	pop     {r0-r2}

	/****************************************************************
	 * Call kernel_main						*
	 ****************************************************************/
	ldr	r3, =kernel_main
	blx	r3

	/****************************************************************
	 * Turn off OK LED						*
	 ****************************************************************/
	// Base of GPIO registers
	ldr	r4, =0xE0200000
	// enable pin 16 as output
	ldr	r6, [r4, #4]
	mov	r5, #1
	lsl	r5, #18
	orr	r6, r6, r5
	str	r6, [r4, #4]
	// turn off LED
	mov	r5, #1
	lsl	r5, #16
	str	r5, [r4, #28]

	// halt
halt:
#	wfe
#	b	halt


	/****************************************************************
	 * Blink OK LED							*
	 ****************************************************************/
	// Emergency signal in case something goes horribly wrong
	.globl abort
abort:
	// Base of GPIO registers
	ldr	r4, =0xE0200000

	// enable pin 16 as output
	ldr	r6, [r4, #4]
	mov	r5, #1
	lsl	r5, #18
	orr	r6, r6, r5
	str	r6, [r4, #4]

2:
	// turn on LED
	mov	r5, #1
	lsl	r5, #16
	str	r5, [r4, #40]

	// wait
	mov	r6, #0x1000000
1:
	sub	r6, #1
	cmp	r6, #0
	bne	1b
	
	// turn off LED
	mov	r5, #1
	lsl	r5, #16
	str	r5, [r4, #28]

	// wait
	mov	r6, #0x1000000
1:
	sub	r6, #1
	cmp	r6, #0
	bne	1b

	// again
	b	2b


	// switch threads
	.globl switch_thread
switch_thread:
        vpush   {d8-d15}
	push	{r4-r12,r14}
	str	sp, [r0, #0]
	mov	sp, r1
	pop	{r4-r12,r14}
        vpop    {d8-d15}
	bx	lr

	// starter stub
	.globl starter_stub
starter_stub:
	pop	{r0-r3,lr}
//	mov	r5,$0xFFFFFFF0
//	bx	r5
	bx	lr


// store constants
constants:
.ltorg

.section ".data"
	.global memory_regions
memory_regions: // start, incr, count
	.word 0x0004140E	// 0x00000000 - 0x0FFFFFFF
	.word 0x01000000	// Outer and Inner Write-Back
	.word 16		// TEX 001, C 1, B 1 Alloc on Write

	.word 0x00000000	// 0x10000000 - 0xBFFFFFFF
	.word 0x00000000	// unmapped
	.word 176

	.word 0x0004140E	// 0xC0000000 - 0xCFFFFFFF
	.word 0x01000000	// Outer and Inner Write-Back
	.word 16		// TEX 001, C 1, B 1 Alloc on Write

	.word 0x1004040A	// 0xD0000000 - 0xDFFFFFFF
	.word 0x01000000	// Outer and Inner Write-Through
	.word 16		// TEX 000, C 1, B 0 No Alloc on Write

	.word 0x20040402	// 0xE0000000 - 0xFFFFFFFF
	.word 0x01000000	// peripherals (memory = device)
	.word 32		// TEX 000, C 0, B 0    not cached
