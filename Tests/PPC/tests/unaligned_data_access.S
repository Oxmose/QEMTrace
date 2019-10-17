/*******************************************************************************
 * unaligned_data_access.S
 *
 * Author  Alexy Torres Aurora Dugo
 * Version 1.0
 *
 * MiniKernel test: Check behaviours on unaligned data access.
 * An unaligned data access can overlapp two pages or two cache lines.
 ******************************************************************************/
 
 /* Unaligned data access on two pages */
 li r3, 0x00007FFE
 li r4, 0x00001234

 stw r4, 0(r3)
 lwz r4, 0(r3)

 addi r3, r3, 1
 sth r4, 0(r3)
 lhz r4, 0(r3)

 /* Unaligned data access on two cache lines (32B/64B/128B) */
 li r3, 0x0000707E
 li r4, 0x00001234

 stw r4, 0(r3)
 lwz r4, 0(r3)

 addi r3, r3, 1
 sth r4, 0(r3)
 lhz r4, 0(r3)
