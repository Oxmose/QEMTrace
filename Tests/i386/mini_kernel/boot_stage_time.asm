; boot_stage_time_eval.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; NASM boot code
; Run time consuming loops
; Output elapsed time
; Disable tracing
; Run time consuming loops
; Output elapsed time
; Halt

[bits 32]
[org 0x8600]

;-------------------------------------------------------------------------------
; TRACE ENABLED TIMING
;-------------------------------------------------------------------------------

; Start Qemu counter
db 0x0F
db 0x7A
db 0x00
db 0x00

call long_loop

; Stop Qemu counter and display
db 0x0F
db 0x7B
db 0x00
db 0x00

;-------------------------------------------------------------------------------
; TRACE DISABLED TIMING
;-------------------------------------------------------------------------------

; Disable tracing: 0x0FA700
db 0x0F
db 0xA7
db 0x00
db 0x00

; Start Qemu counter
db 0x0F
db 0x7A
db 0x00
db 0x00

call long_loop

; Stop Qemu counter and display
db 0x0F
db 0x7B
db 0x00
db 0x00

halt_loop:

	; Disable tracing: 0x0FA700 that save the last buffer
	db 0x0F
	db 0xA7
	db 0x00
	db 0x00
	cli
	hlt
	jmp halt_loop


long_loop:
	push eax
	push ebx
	push ecx

	mov eax, 100000
	mov ebx, 0x100000

actual_loop:
	mov ecx, [ebx]
	dec eax
	cmp eax, 0
	jne actual_loop

	pop eax
	pop ebx
	pop ecx
	ret


; Padding
times 5120-($-$$) db 0
