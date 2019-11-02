@ test_cache_pagdis.asm
@
@ Author  Alexy Torres Aurora Dugo
@ Version 1.0
@
@ MiniKernel test: Check the ability of Qemu to
@ trace if the access is cached or not

@ Save registers
push {r0, r1}

@ Compute address
mov r0, #0x8020
lsl r0, r0, #16
mov r1, #0xDEAD

str r1, [r0, #0]
str r1, [r0, #4]
str r1, [r0, #8]

@ Enable cache
mrc p15, 0, r0, c1, c0, 0 ; Read System Control Register
or r0, r0, #0x1004
mcr p15, 0, r0, c1, c0, 0 ; Write System Control Register

@ Compute address 
mov r0, #0x8020
lsl r0, r0, #16
mov r1, #0xDEAD

str r1, [r0, #0]
str r1, [r0, #4]
str r1, [r0, #8]

@ Restore registers
pop {r0, r1}

