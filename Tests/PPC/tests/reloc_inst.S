/*******************************************************************************
 * reloc_inst.S
 *
 * Author  Alexy Torres Aurora Dugo
 * Version 1.0
 *
 * MiniKernel test: Check if all instruction are correctly traced even when
 * repeated and relocated.
 ******************************************************************************/

li r3, code_start
li r4, code_end

/* Compute 0x10000 */
li r5, 0x1000
slwi r5, r5, 4

copy_loop:

    /* Copy */
    lwz r6, 0(r3)
    stw r6, 0(r5)

    addi r5, r5, 4
    addi r3, r3, 4
    cmp 0, 0, r3, r4 /* Compare r3 with r4, cr0 bit 2 is set on equality */
    bne cr0, copy_loop

bla 0x10000
b code_end

code_start:

li r3, 0x7000
li r4, 0x1234

stw r4, 0(r3)
lwz r4, 0(r3)

sth r4, 0(r3)
lhz r4, 0(r3)

stb r4, 0(r3)
lbz r4, 0(r3)

blr

code_end:
nop
