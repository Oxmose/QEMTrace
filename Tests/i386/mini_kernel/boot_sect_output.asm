; boot_sect_output.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; NASM boot code, display message sent throught BX register
; for the boot sector routines

[bits 16]

boot_sect_out: ; Print string pointed by BX
	pusha	   ; Save registers

boot_sect_out_loop:        ; Display loop
	mov cx, [bx]           ; Load character into cx
	cmp cl, 0		       ; Compare for NULL end
	je boot_sect_out_end   ; If NULL exit

	mov al, cl             ; Printf current character
	mov ah, 0x0e           ; SAME
	int 0x10               ; SAME

	add bx, 1			   ; Move to next character
	jmp boot_sect_out_loop ; Loop

boot_sect_out_end:
	popa ; Restore registers
	ret

boot_sect_out_hex:
	pusha ; Save registers
	mov dx, 0
	push bx
	mov bx, HEX_SEQ
	call boot_sect_out
	pop bx

boot_sect_out_hex_loop:
	cmp dx, 4
	je boot_sect_out_hex_end

	mov cx, bx
	and cx, 0xF000
	shr cx, 12

	cmp cx, 9
	jle boot_sect_out_hex_num

	add cx, 7

boot_sect_out_hex_num:

	add cx, 48
	mov al, cl
	mov ah, 0x0e
	int 0x10

	shl bx, 4
	add dx, 1
	jmp boot_sect_out_hex_loop

boot_sect_out_hex_end:
	popa ; Restore registers
	ret

HEX_SEQ:
	db "0x", 0
