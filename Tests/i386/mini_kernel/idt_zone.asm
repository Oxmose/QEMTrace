; idt_zone.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; This file contains the IDT memory zone ofthe kernel

IDT_SIZE equ 2048

;-----------------------------------------------------------
; SEGMENT DESCRIPTOR
;-----------------------------------------------------------
%define CODE32 0x0008
%define DATA32 0x0010
%define CODE16 0x0018
%define DATA16 0x0020

[bits 32]
[org 0x5000]

; Entry 0
	dw 0x1000    ; Low 16 Bits of the handler address
	dw CODE32    ; Kernel CS
	db 0x00      ; Zero
	db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
	dw 0x0000    ; High 16 Bits of the handler address

; Entries 1 - 31
times 256 -($-$$) db 0

; Entry 32
	dw 0x2000    ; Low 16 Bits of the handler address
	dw CODE32    ; Kernel CS
	db 0x00      ; Zero
	db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
	dw 0x0000    ; High 16 Bits of the handler address

; Entries 33 - 79
times 640 -($-$$) db 0

; Entry 80
	dw 0x3000    ; Low 16 Bits of the handler address
	dw CODE32    ; Kernel CS
	db 0x00      ; Zero
	db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
	dw 0x0000    ; High 16 Bits of the handler address

; Entries 33 - 199
times 1600 -($-$$) db 0

; Entry 200
	dw 0x4000    ; Low 16 Bits of the handler address
	dw CODE32    ; Kernel CS
	db 0x00      ; Zero
	db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
	dw 0x0000    ; High 16 Bits of the handler address

; Pad rest of the entries
times IDT_SIZE-($-$$) db 0
