@ test_opcode_trace.asm
@
@ Author  Alexy Torres Aurora Dugo
@ Version 1.0
@
@ MiniKernel test: Start tracing Qemu custom instruction
@ The Qemu emulator should manage this instruction and strart
@ tracing the system memory access.

@ Save registers
push {r0, r1}

@ Compute address at 1M 
mov r0, #0x8020
lsl r0, r0, #16
mov r1, #0xDEAD

@ Disable tracing: 0xFFFFFFF1 
.long 0xFFFFFFF1

str r1, [r0, #0]
str r1, [r0, #4]
str r1, [r0, #8]

@ Enable tracing: 0xFFFFFFF0 
.long 0xFFFFFFF0

@ Compute address at 1M 
mov r0, #0x8020
lsl r0, r0, #16
mov r1, #0xDEAD

str r1, [r0, #0]
str r1, [r0, #4]
str r1, [r0, #8]

@ Restore registers
pop {r0, r1}

