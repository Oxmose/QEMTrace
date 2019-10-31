.text
.globl	test_entry @ Kernel entry point

test_entry:
push {r11, lr}

@ Enable tracing: 0xFFFFFFF0
.long 0xFFFFFFF0


@ Disable tracing: 0xFFFFFFF1
.long 0xFFFFFFF1

pop {r11, lr}
bx lr