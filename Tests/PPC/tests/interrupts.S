/*******************************************************************************
 * interrupts.S
 *
 * Author  Alexy Torres Aurora Dugo
 * Version 1.0
 *
 * MiniKernel test: Check qemu tracing behaviour during interrupts, traps
 * and exceptions.
 ******************************************************************************/

/* Copy interrupt vector */
li r3, interrupt_table_start
li r4, interrupt_table_end

li r5, 0x1000
slwi r5, r5, 4

bl copy_loop

/* Load Interrupt vector */
lis r0, 0x1
mtivpr r0

li r0, 96
mtivor6 r0

li r20, 0
tweqi r20, 0
b code_end

copy_loop:

    /* Copy */
    lwz r6, 0(r3)
    stw r6, 0(r5)

    addi r5, r5, 4
    addi r3, r3, 4
    cmp cr0, 0, r3, r4 /* Compare r3 with r4, cr0 bit 2 is set on equality */
    bne cr0, copy_loop

blr

.equ SIXTEEN_BYTES, 4 # 16 byte alignment required for table entries

interrupt_table_start:
.align SIXTEEN_BYTES
IVOR0trap: b IVOR0trap # IVOR 0 interrupt handler
.align SIXTEEN_BYTES
IVOR1trap: b IVOR1trap # IVOR 1 interrupt handler
.align SIXTEEN_BYTES
IVOR2trap: b IVOR2trap # IVOR 2 interrupt handler
.align SIXTEEN_BYTES
IVOR3trap: b IVOR3trap # IVOR 3 interrupt handler
.align SIXTEEN_BYTES
IVOR4trap: b IVOR4trap # IVOR 4 interrupt handler
.align SIXTEEN_BYTES
IVOR5trap: b IVOR5trap # IVOR 5 interrupt handler
.align SIXTEEN_BYTES
IVOR6trap: ba trap_handler # IVOR 6 interrupt handler
.align SIXTEEN_BYTES
IVOR7trap: b IVOR7trap # IVOR 7 interrupt handler
.align SIXTEEN_BYTES
IVOR8trap: b IVOR8trap # IVOR 8 interrupt handler
.align SIXTEEN_BYTES
IVOR9trap: b IVOR9trap # IVOR 9 interrupt handler
.align SIXTEEN_BYTES
IVOR10trap: b IVOR10trap # IVOR 10 interrupt handler
.align SIXTEEN_BYTES
IVOR11trap: b IVOR11trap # IVOR 11 interrupt handler
.align SIXTEEN_BYTES
IVOR12trap: b IVOR12trap # IVOR 12 interrupt handler
.align SIXTEEN_BYTES
IVOR13trap: b IVOR13trap # IVOR 13 interrupt handler
.align SIXTEEN_BYTES
IVOR14trap: b IVOR14trap # IVOR 14 interrupt handler
.align SIXTEEN_BYTES
IVOR15trap: b IVOR15trap # IVOR15 interrupt handler
interrupt_table_end:


trap_handler:

li r3, 0x7000
li r4, 0x1234

stw r4, 0(r3)
lwz r4, 0(r3)

sth r4, 0(r3)
lhz r4, 0(r3)

stb r4, 0(r3)
lbz r4, 0(r3)

li r20, 5

rfi

code_end:
nop
