; boot_sage2.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; NASM boot code
; Execute paging enabled tests
; Disable paging
; Execute paging disabled tests
; Halt

[bits 32]
[org 0x8600]

;----------------------------------
; PAGING ENABLED TESTS
;----------------------------------
pagen_tests:
%include "test_pagen_glob.asm"

; Padding for page enabled tests
jmp 0x8FF4
times 2544-($-$$) db 0
times 4 db 0xFF


; Disable Paging
disable_paging:
mov eax, cr0
and  eax, 0x7FFFFFFF
mov cr0, eax

; Padding for paging disabler
times 2560-($-$$) db 0x90

;----------------------------------
; PAGING DISABLED TESTS
;----------------------------------
pagdis_tests:
%include "test_pagdis_glob.asm"

; Pad rest of the memory
jmp 0x99F4
times 5104-($-$$) db 0
times 4 db 0xFF

halt_loop:
	; Disable tracing: 0x0FA700 that save the last buffer
	db 0x0F
	db 0xA7
	db 0x00
	db 0x00
	cli
	hlt
	jmp halt_loop
; Padding for halt
times 5120-($-$$) db 0
