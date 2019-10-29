; boot_sect_output_pm.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; NASM boot code, display message sent throught ECX register
; Display message in PM mode using VGA buffer
; Line is contained in eax, column is contained in ebx

[bits 32]

boot_sect_out_pm: ; Print string pointed by BX
	pusha	   ; Save registers

	; Comput start address
	mov edx, 160
	mul edx      ; eax now contains the offset in lines
	add eax, ebx ; Offset is half complete
	add eax, ebx ; Offset is complete

	add eax, 0x000B8000 ; VGA buffer address
	mov ebx, ecx

boot_sect_out_pm_loop:      ; Display loop
	mov cl, [ebx]           ; Load character into cx
	cmp cl, 0		        ; Compare for NULL end
	je boot_sect_out_pm_end ; If NULL exit
	mov ch, 0x0007
	mov [eax], cx          ; Printf current character
	add eax, 2                ; Move cursor

	add ebx, 1			   ; Move to next character
	jmp boot_sect_out_pm_loop ; Loop

boot_sect_out_pm_end:
	popa ; Restore registers
	ret
