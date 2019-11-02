@ instruction_tracing.asm
@
@ Author  Alexy Torres Aurora Dugo
@ Version 1.0
@
@ MiniKernel test: Check if all instruction are correctly traced even when
@ repeated.

push {r0, r1, r2}

mov r2, #0
mov r0, #5
mov r0, #5
mov r0, #5
mov r0, #500
inst_tracing_loop:
sub r0, r0, #1
mov r1, #5
cmp r0, r2
bne inst_tracing_loop

pop {r0, r1, r2}
