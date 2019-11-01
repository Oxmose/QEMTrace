.text
.globl	test_entry @ Kernel entry point

test_entry:
push {r11, lr}

@ Paging disabled tests

@ Enable tracing: 0xFFFFFFF0
.long 0xFFFFFFF0

.include "test_pagdis_glob.S"

@ Disable tracing: 0xFFFFFFF1
.long 0xFFFFFFF1

@ Paging enabled tests

@ Enable tracing: 0xFFFFFFF0
.long 0xFFFFFFF0

.include "test_pagen_glob.S"

@ Disable tracing: 0xFFFFFFF1
.long 0xFFFFFFF1

pop {r11, lr}
bx lr