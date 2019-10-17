/*******************************************************************************
 * flash_inval.S
 *
 * Author  Alexy Torres Aurora Dugo
 * Version 1.0
 *
 * MiniKernel test: Check if Qemu correctly traces cache flash inval.
 ******************************************************************************/

/* Icache flash inval */
.long 0xFFFFFFF4

/* Dcache flash inval */
.long 0xFFFFFFF5
