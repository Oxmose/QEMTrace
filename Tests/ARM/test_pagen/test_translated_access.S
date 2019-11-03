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
mov r0, #0x8020
lsl r0, #16
ldr r1, [r0]

@ 0x8011_0000 : 0x8040_0000
mov r0, #0x8011
lsl r0, #16
ldr r1, [r0]

@ 0x8028_0000 : 0x80B0_0000
mov r0, #0x8028
lsl r0, #16
ldr r1, [r0]

@ 0x8030_0000 : 0x8030_1000
mov r0, #0x8030
lsl r0, #16
ldr r1, [r0]

@ Restore registers
pop {r0, r1}

