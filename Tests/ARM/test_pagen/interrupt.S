@ interrupts.asm
@
@ Author  Alexy Torres Aurora Dugo
@ Version 1.0
@
@ MiniKernel test: Check qemu tracing behaviour during interrupts, traps
@ and exceptions.

.globl int_handler

@ Save registers
push {r0, r1, lr}

swi 0x2A

@ Restore registers
pop {r0, r1, lr}
b end


@ Generate interrupt
int_handler:
    mov r0, #0x8020
    lsl r0, r0, #16
    mov r1, #0xDEAD

    str r1, [r0, #0]
    str r1, [r0, #4]
    str r1, [r0, #8]

    mov r0, #0x8020
    lsl r0, r0, #16
    mov r1, #0xDEAD

    ldr r1, [r0, #0]
    ldr r1, [r0, #4]
    ldr r1, [r0, #8]

    bx lr

end: