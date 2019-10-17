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

	.file	"boot.S"
	.text

    /* Define kernel entry point */
	.globl	_entry
	.type	_entry,@function

    #define SPR_Exxx_L1CSR0       (0x3F2)
    #define SPR_Exxx_L1CSR1       (0x3F3)

_entry:

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

    /* Enable data cache */
    mfspr	r5,SPR_Exxx_L1CSR0
    ori     r5, r5, 0x0001
    mtspr	SPR_Exxx_L1CSR0,r5
    isync

    /* Enable instruction cache */
    mfspr	r5,SPR_Exxx_L1CSR1
    ori     r5, r5, 0x0001
    mtspr	SPR_Exxx_L1CSR1,r5
    isync

    /* Call the kernel_kickstart function, we should never return from here */
	bl	FUNC_NAME(kernel_kickstart)

	trap

.kernel_stack:
	.long	stack_base
