/*******************************************************************************
 * cache_coherent.S
 *
 * Author  Alexy Torres Aurora Dugo
 * Version 1.0
 *
 * MiniKernel test: Check if Qemu correctly trace cache coherency state.
 ******************************************************************************/

li r3, 0x7000
li r4, 0x1234

/* Non coherent */
stw r4, 0(r3)
addi r3, r3, 0x2000


/* Coherent */
stw r4, 0(r3)
