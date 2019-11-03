@ access_size.asm
@
@ Author  Alexy Torres Aurora Dugo
@ Version 1.0
@
@ MiniKernel test: Check if the acess size is correctly traced

push {r0-r1}

mov r0, #0x8020
lsl r0, r0, #16

ldr r1, [r0]
ldrh r1, [r0]
ldrb r1, [r0]

str r1, [r0]
strh r1, [r0]
strb r1, [r0]

pop {r0-r1}
