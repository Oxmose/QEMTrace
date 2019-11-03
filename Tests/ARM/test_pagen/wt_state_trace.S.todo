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
