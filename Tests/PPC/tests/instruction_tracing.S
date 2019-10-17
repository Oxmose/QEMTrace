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
