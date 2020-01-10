/*
 * boot.S -- startup file for PowerPC systems.
 *
 * Copyright (c) 1995 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include "ppc-asm.h"
#include "common.h"

	.file	"boot.S"
	.text

    /* Define kernel entry point */
	.globl	_entry
	.type	_entry,@function

    .extern _kernel_bss_start
    .extern _kernel_bss_end

_entry:
    /* Start tracing and timer*/
    .long 0xFFFFFFF6
    .long 0xFFFFFFF2

	bl	.addr_trick /* Jump to addr_trick to get the current inst address */

.addr_trick:
	mflr r4 /* Get the current instruction address */

    /* Set the stack */
	lwz	 r0, (.kernel_stack - .addr_trick)(r4) /* Get the stack base address */
	mr	 sp, r0			                       /* Set the stack base */

	addi sp, sp, -4	  /* Just add one byte unused */
	lis	 r0, 0        /* Clear R0 */
	stw	 r0, 0(sp)	  /* Clear back chain */
	stwu sp, -64(sp)  /* Create a new stack frame */

l1_invalidate:
    li      r3,2
    msync
    isync
    mtspr	SPR_Exxx_L1CSR0,r3	/* invalidate d-cache */
    mtspr	SPR_Exxx_L1CSR1,r3	/* invalidate i-cache */
    isync

l1_d_enable:
    /* Enable data cache */
    msync
    isync
    mfspr   r3,SPR_Exxx_L1CSR0
    ori     r3, r3, 0x0001
    mtspr   SPR_Exxx_L1CSR0,r3
    isync

l1_i_enable:
    /* Enable instruction cache */
    mfspr   r3,SPR_Exxx_L1CSR1
    ori     r3, r3, 0x0001
    mtspr   SPR_Exxx_L1CSR1,r3
    isync

bss_init:
    lis     r3, _kernel_bss_start@h
    ori     r3, r3, _kernel_bss_start@l
    lis     r4, _kernel_bss_end@h
    ori     r4, r4, _kernel_bss_end@l
    li      r0, 0
    bss_blank:
        cmp  0, 0, r3, r4
        beq  bss_blank_end
        stw  r0, 0(r3)
        addi r3, r3, 1
        b bss_blank
    bss_blank_end:

bootstrap: 
    /* Call the kernel_kickstart function, we should never return from here */
	bl	FUNC_NAME(kernel_kickstart)

    /* Stop tracing and timer */
    .long 0xFFFFFFF3
    .long 0xFFFFFFF7

endloop:
    b endloop

	trap

.kernel_stack:
	.long	kernel_stack_base
