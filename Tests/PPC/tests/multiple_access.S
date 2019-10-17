/*******************************************************************************
 * multiple_access.S
 *
 * Author  Alexy Torres Aurora Dugo
 * Version 1.0
 *
 * MiniKernel test: Check if one instruction loading/storing multiple data is
 * correctly traced
 ******************************************************************************/

/* Compute address (two pages wide multiple word access) */
li r3, 0x00000AFF
li r4, 0x4
slw r3, r3, r4
addi r3, r3, 0xC

li r25, 0x1234
li r26, 0x5670
li r27, 0x1234
li r28, 0x5670
li r29, 0x1234
li r30, 0x5670
li r31, 0x1234

/* Store multiple words */
li r24, 7
stmw r24, 0(r3)

/* Load multiple words */
li r24, 7
lmw r24, 0(r3)


li r5, 31
