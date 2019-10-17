/*******************************************************************************
 * cache_op_trace.S
 *
 * Author  Alexy Torres Aurora Dugo
 * Version 1.0
 *
 * MiniKernel test: Check if Qemu correctly traces cache operations.
 ******************************************************************************/

/********************************** DCBA **************************************/
/* Coherent */
li r3, 0x900
slwi r3, r3, 4
li r4, 0
dcba r3, r4
/* Non coherent */
li r3, 0x7000
dcba r3, r4
/* Cache Inhibited */
li r3, 0xF00
slwi r3, r3, 4
dcba r3, r4
/* Non cache inhibited */
li r3, 0xB00
slwi r3, r3, 4
dcba r3, r4

/********************************** DCBF **************************************/
/* Coherent */
li r3, 0x900
slwi r3, r3, 4
li r4, 0
dcbf r3, r4
/* Non coherent */
li r3, 0x7000
dcbf r3, r4
/* Cache Inhibited */
li r3, 0xF00
slwi r3, r3, 4
dcbf r3, r4
/* Non cache inhibited */
li r3, 0xB00
slwi r3, r3, 4
dcbf r3, r4

/********************************** DCBI **************************************/
/* Coherent */
li r3, 0x900
slwi r3, r3, 4
li r4, 0
dcbi r3, r4
/* Non coherent */
li r3, 0x7000
dcbi r3, r4
/* Cache Inhibited */
li r3, 0xF00
slwi r3, r3, 4
dcbi r3, r4
/* Non cache inhibited */
li r3, 0xB00
slwi r3, r3, 4
dcbi r3, r4

/********************************** DCBLC *************************************/
li r3, 0x900
slwi r3, r3, 4
li r4, 0

dcblc r3, r4
dcblc 0, r3, r4
dcblc 1, r3, r4
dcblc 2, r3, r4
dcblc 3, r3, r4
dcblc 4, r3, r4

/* Coherent */
li r3, 0x900
slwi r3, r3, 4
li r4, 0
dcblc r3, r4
/* Non coherent */
li r3, 0x7000
dcblc r3, r4
/* Cache Inhibited */
li r3, 0xF00
slwi r3, r3, 4
dcblc r3, r4
/* Non cache inhibited */
li r3, 0xB00
slwi r3, r3, 4
dcblc r3, r4

/********************************** DCBST *************************************/
/* Coherent */
li r3, 0x900
slwi r3, r3, 4
li r4, 0
dcbst r3, r4
/* Non coherent */
li r3, 0x7000
dcbst r3, r4
/* Cache Inhibited */
li r3, 0xF00
slwi r3, r3, 4
dcbst r3, r4
/* Non cache inhibited */
li r3, 0xB00
slwi r3, r3, 4
dcbst r3, r4

/********************************** DCBT **************************************/
/* Coherent */
li r3, 0x900
slwi r3, r3, 4
li r4, 0
dcbt r3, r4
/* Non coherent */
li r3, 0x7000
dcbt r3, r4
/* Cache Inhibited */
li r3, 0xF00
slwi r3, r3, 4
dcbt r3, r4
/* Non cache inhibited */
li r3, 0xB00
slwi r3, r3, 4
dcbt r3, r4

/********************************** DCBTLS ************************************/
/* Levels */
li r3, 0x900
slwi r3, r3, 4
li r4, 0
dcbtls r3, r4
dcbtls 0, r3, r4
dcbtls 1, r3, r4
dcbtls 2, r3, r4
dcbtls 3, r3, r4
dcbtls 4, r3, r4

/* Coherent */
li r3, 0x900
slwi r3, r3, 4
li r4, 0
dcbtls r3, r4
/* Non coherent */
li r3, 0x7000
dcbtls r3, r4
/* Cache Inhibited */
li r3, 0xF00
slwi r3, r3, 4
dcbtls r3, r4
/* Non cache inhibited */
li r3, 0xB00
slwi r3, r3, 4
dcbtls r3, r4

/********************************** DCBTST ************************************/
/* Coherent */
li r3, 0x900
slwi r3, r3, 4
li r4, 0
dcbtst r3, r4
/* Non coherent */
li r3, 0x7000
dcbtst r3, r4
/* Cache Inhibited */
li r3, 0xF00
slwi r3, r3, 4
dcbtst r3, r4
/* Non cache inhibited */
li r3, 0xB00
slwi r3, r3, 4
dcbtst r3, r4

/********************************** DCBTSTLS **********************************/

/* Levels */
li r3, 0x900
slwi r3, r3, 4
li r4, 0
dcbtstls r3, r4
dcbtstls 0, r3, r4
dcbtstls 1, r3, r4
dcbtstls 2, r3, r4
dcbtstls 3, r3, r4
dcbtstls 4, r3, r4

/* Coherent */
li r3, 0x900
slwi r3, r3, 4
li r4, 0
dcbtstls r3, r4
/* Non coherent */
li r3, 0x7000
dcbtstls r3, r4
/* Cache Inhibited */
li r3, 0xF00
slwi r3, r3, 4
dcbtstls r3, r4
/* Non cache inhibited */
li r3, 0xB00
slwi r3, r3, 4
dcbtstls r3, r4

/********************************** DCBZ **************************************/
/* Coherent */
li r3, 0x900
slwi r3, r3, 4
li r4, 0
dcbz r3, r4
/* Non coherent */
li r3, 0x7000
dcbz r3, r4
/* Cache Inhibited */
li r3, 0xF00
slwi r3, r3, 4
dcbz r3, r4
/* Non cache inhibited */
li r3, 0xB00
slwi r3, r3, 4
dcbz r3, r4

/********************************** ICBLC *************************************/
li r3, 0x300
slwi r3, r3, 4
li r4, 0

icblc r3, r4
icblc 0, r3, r4
icblc 1, r3, r4
icblc 2, r3, r4
icblc 3, r3, r4
icblc 4, r3, r4

/* Coherent */
li r3, 0x1000
slwi r3, r3, 4
li r4, 0
icblc r3, r4
/* Non coherent */
li r3, 0x300
icblc r3, r4
/* Cache Inhibited */
li r3, 0x1000
slwi r3, r3, 4
icblc r3, r4
/* Non cache inhibited */
li r3, 0x300
slwi r3, r3, 4
icblc r3, r4

/********************************** ICBT *************************************/
li r3, 0x300
slwi r3, r3, 4
li r4, 0

icbt r3, r4
icbt 0, r3, r4
icbt 1, r3, r4
icbt 2, r3, r4
icbt 3, r3, r4
icbt 4, r3, r4

/* Coherent */
li r3, 0x1000
slwi r3, r3, 4
li r4, 0
icbt r3, r4
/* Non coherent */
li r3, 0x300
icbt r3, r4
/* Cache Inhibited */
li r3, 0x1000
slwi r3, r3, 4
icbt r3, r4
/* Non cache inhibited */
li r3, 0x300
slwi r3, r3, 4
icbt r3, r4

/********************************** ICBTLS ************************************/
li r3, 0x300
slwi r3, r3, 4
li r4, 0

icbtls r3, r4
icbtls 0, r3, r4
icbtls 1, r3, r4
icbtls 2, r3, r4
icbtls 3, r3, r4
icbtls 4, r3, r4

/* Coherent */
li r3, 0x1000
slwi r3, r3, 4
li r4, 0
icbtls r3, r4
/* Non coherent */
li r3, 0x300
icbtls r3, r4
/* Cache Inhibited */
li r3, 0x1000
slwi r3, r3, 4
icbtls r3, r4
/* Non cache inhibited */
li r3, 0x300
slwi r3, r3, 4
icbtls r3, r4

/********************************** ICBI **************************************/
/* Coherent */
li r3, 0x900
slwi r3, r3, 4
li r4, 0
icbi r3, r4
/* Non coherent */
li r3, 0x7000
icbi r3, r4
/* Cache Inhibited */
li r3, 0xF00
slwi r3, r3, 4
icbi r3, r4
/* Non cache inhibited */
li r3, 0xB00
slwi r3, r3, 4
icbi r3, r4
