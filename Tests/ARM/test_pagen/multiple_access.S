@ multiple_access.asm
@
@ Author  Alexy Torres Aurora Dugo
@ Version 1.0
@
@ MiniKernel test: Check if one instruction loading/storing multiple data is
@ correctly traced

push {r0-r11}

mov r0, #0x8021
lsl r0, r0, #16

ldm r0, {r1-r11}
ldmia r0, {r1-r11}
ldmfd r0, {r1-r11}
ldmdb r0, {r1-r11}
ldmea r0, {r1-r11}

stm r0, {r1-r11}
stmia r0, {r1-r11}
stmfd r0, {r1-r11}
stmdb r0, {r1-r11}
stmea r0, {r1-r11}

pop {r0-r11}
