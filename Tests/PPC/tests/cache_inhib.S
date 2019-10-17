/*******************************************************************************
 * cache_inhib.S
 *
 * Author  Alexy Torres Aurora Dugo
 * Version 1.0
 *
 * MiniKernel test: Check if Qemu correctly trace cache inhibited state.
 ******************************************************************************/
#define SPR_Exxx_L1CSR0       (0x3F2)
#define SPR_Exxx_L1CSR1       (0x3F3)

/* Save cache state */
mfspr	r8,SPR_Exxx_L1CSR0
mfspr	r9,SPR_Exxx_L1CSR1

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

li r3, 0x7000
li r4, 0x1234

/* Non inhib */
stw r4, 0(r3)

li r3, 0xF00
li r4, 0x4
slw r3, r3, r4

/* Inhib */
stw r4, 0(r3)

/* Disable instruction cache */
mfspr	r5,SPR_Exxx_L1CSR1
li      r6, 0x00000001
andc    r5, r5, r6
mtspr	SPR_Exxx_L1CSR1,r5
isync

/* Inhib (intruction) */
li r3, 0x7000
li r4, 0x1234

/* Non Inhib */
stw r4, 0(r3)

li r3, 0xF00
li r4, 0x4
slw r3, r3, r4

/* Inhib */
stw r4, 0(r3)

/* Disable data cache */
mfspr	r5,SPR_Exxx_L1CSR0
li      r6, 0x00000001
andc    r5, r5, r6
mtspr	SPR_Exxx_L1CSR0,r5
isync

/* Enable instruction cache */
mfspr	r5,SPR_Exxx_L1CSR1
ori     r5, r5, 0x0001
mtspr	SPR_Exxx_L1CSR1,r5
isync

/* Non inhib (intruction) */
li r3, 0x7000
li r4, 0x1234

/* Inhib */
stw r4, 0(r3)

/* Restore cache state */
mtspr	SPR_Exxx_L1CSR0,r8
mtspr	SPR_Exxx_L1CSR1,r9
isync
