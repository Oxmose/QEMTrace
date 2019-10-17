/*******************************************************************************
 * atomic_op.S
 *
 * Author  Alexy Torres Aurora Dugo
 * Version 1.0
 *
 * MiniKernel test: Check if all atomic operations are correctly traced
 ******************************************************************************/

li r3, 0x7000
la r4, 0(r3)
li r4, 0

lwarx r5, r3, r4
stwcx. r5, r3, r4 /* Should store */
stwcx. r5, r3, r4 /* Should not store */

/* THE FOLLOWING ARE NOT DEFINED ON E500 */
#if 0
lharx r5, r3, r4

sthcx. r5, r3, r4 /* Should store */
sthcx. r5, r3, r4 /* Should not store */

lbarx r5, r3, r4

stbcx. r5, r3, r4 /* Should store */
stbcx. r5, r3, r4 /* Should not store */
#endif
