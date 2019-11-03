@ unaligned_data_access.asm
@
@ Author  Alexy Torres Aurora Dugo
@ Version 1.0
@
@ MiniKernel test: Check behaviours on unaligned data access.
@ An unaligned data access can overlapp two pages.

@ Unaligned data access on two pages
push {r0-r1}

mov r0, #0x8021
lsl r0, r0, #16
mov r1, #0xFFF
lsl r1, r1, #4
add r1, r1, #0xE
add r0, r0, r1

ldr r1, [r0]
str r1, [r0]

pop {r0-r1}
