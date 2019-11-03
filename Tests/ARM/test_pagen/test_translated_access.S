@ test_translated_access.asm
@
@ Author  Alexy Torres Aurora Dugo
@ Version 1.0
@
@ MiniKernel test: Check if QEMU correctly trace the 
@ translated physical addresses

@ Save registers
push {r0, r1}

@ 1:1
ldr r0, =0x80200000
ldr r1, [r0]

@ 0x8011_0000 : 0x8040_0000
ldr r0, =0x80110000
ldr r1, [r0]

@ 0x8028_0000 : 0x80B0_0000
ldr r0, =0x80280000
ldr r1, [r0]

@ 0x8030_0000 : 0x8030_1000
ldr r0, =0x80300000
ldr r1, [r0]

@ Restore registers
pop {r0, r1}

