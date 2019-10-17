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
 * wt_state_trace.S
 *
 * Author  Alexy Torres Aurora Dugo
 * Version 1.0
 *
 * MiniKernel test: Check the tracing on WT enabled addresses.
 ******************************************************************************/
 
lis r3, 0x0001
addi r3, r3, 0x1000

/* WT enabled */
lwz r4, 0(r3)

lis r3, 0x0001
/* WT disabled */
lwz r4, 0(r3)
/* EP */
/* Disable tracing: 0xFFFFFFF1 */
.long 0xFFFFFFF1
addi sp, sp, 16
lwz r0, 8(sp)
mtlr r0
blr
