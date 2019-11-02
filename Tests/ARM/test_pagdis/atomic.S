@ atomic.asm
@
@ Author  Alexy Torres Aurora Dugo
@ Version 1.0
@
@ MiniKernel test: Check behaviours on atomic data access.

push {r0-r3}

mov r0, #0x8020
lsl r0, r0, #16


ldrex r1, [r0]
strex r1, r2, [r0]

ldrexb r1, [r0]
strexb r1, r2, [r0]

ldrexh r1, [r0]
strexh r1, r2, [r0]

ldrexd r2, r3, [r0]
strexd r1, r2, [r0]

pop {r0-r3}
