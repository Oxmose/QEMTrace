.text
.globl	test_entry @ Kernel entry point

test_entry:
push {r0, r1, r2, r3, r11, lr}

ldr r0, =_pagdiscodeaddr

ldr r1, =pagdiscodestart
ldr r2, =pagdiscodeend
bl code_relocate

ldr r0, =_pagencodeaddr_phys

ldr r1, =pagencodestart
ldr r2, =pagencodeend
bl code_relocate

@ Paging disabled tests
ldr r0, =_pagdiscodeaddr
blx r0

bl enable_paging

@ Paging enabled tests
ldr r0, =_pagencodeaddr
blx r0


pop {r0, r1, r2, r3, r11, lr}
bx lr

code_relocate:
    cmp r1, r2
    beq code_relocate_end 
    ldr r3, [r1]
    str r3, [r0]
    add r1, r1, #4
    add r0, r0, #4
    b code_relocate
code_relocate_end:
    bx lr

@ Relocated codes protion

pagdiscodestart:
@ Enable tracing: 0xFFFFFFF0
.long 0xFFFFFFF0
.include "test_pagdis_glob.S.inc"
@ Disable tracing: 0xFFFFFFF1
.long 0xFFFFFFF1
bx lr
pagdiscodeend:

pagencodestart:
@ Enable tracing: 0xFFFFFFF0
.long 0xFFFFFFF0
.include "test_pagen_glob.S.inc"
@ Disable tracing: 0xFFFFFFF1
.long 0xFFFFFFF1
bx lr
pagencodeend:
