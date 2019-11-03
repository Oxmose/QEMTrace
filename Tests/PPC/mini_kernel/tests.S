#include "ppc-asm.h"
.file	"tests.S"
.text
  /* Define kernel entry point */
.globl	test_entry
.type	test_entry,@function
test_entry:
/* PR */
mflr r0
stw r0, 8(sp)
stwu sp, -16(sp)
/* Enable tracing: 0xFFFFFFF0 */
.long 0xFFFFFFF0
/* Tests will be cat here */
/*******************************************************************************
 * instruction_tracing.S
 *
 * Author  Alexy Torres Aurora Dugo
 * Version 1.0
 *
 * MiniKernel test: Check if all instruction are correctly traced even when
 * repeated.
 ******************************************************************************/

/* 4 + 4 * 500 + 1 = 2005 */
 li r3, 5
 li r3, 5
 li r3, 5
 li r3, 500
 inst_tracing_loop:
     addi r3, r3, -1
     li r4, 5
     cmpi 0,0,r3,0 /* Compare r3 with 0, cr0 bit 2 is set on equality */
     bne cr0, inst_tracing_loop
/* EP */
/* Disable tracing: 0xFFFFFFF1 */
.long 0xFFFFFFF1
addi sp, sp, 16
lwz r0, 8(sp)
mtlr r0
blr
