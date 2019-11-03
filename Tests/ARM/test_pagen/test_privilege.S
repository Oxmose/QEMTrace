@ test_privilege.asm
@
@ Author  Alexy Torres Aurora Dugo
@ Version 1.0
@
@ MiniKernel test: Test the QEMU ability to trace
@ the correct privilege level

@ Save registers
push {r0, r1}

@ Set sys
cps #31
mov r0, r0
@ Set fiq
cps #17
mov r0, r0
@ Set irq
cps #18
mov r0, r0
@ Set svc
cps #19
mov r0, r0
@ Set abt
cps #23
mov r0, r0
@ Set undef
cps #27
mov r0, r0
@ Set usr
cps #16
mov r0, r0

@ Disable tracing: 0xFFFFFFF1 
.long 0xFFFFFFF1

@ Since we didnt save the correct LR just loop here
@ It is not an issue as this test shoudlbe the only one running
loop:
    b loop

@ Restore registers
pop {r0, r1}

