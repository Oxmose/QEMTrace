; hard_drive.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; NASM boot code, load sector form disk into memory.
; Used to load the kernel into memory.

[bits 16]
; Load from disk
disk_load:

	; ax contains the number of sectors to read
	; bx contains the address to store the data to
	; cx contains the first sector to read
	; dx contains the device to read

	mov ah, 0x02 ; Set BIOS disk read function
	mov ch, 0x00 ; Select cylinder 0
	mov dh, 0x00 ; Select head 0

	push ax ; Save number of sector to read

	int 0x13     ; Issue read

	jc disk_load_error ; Jump on error (carry)

	pop dx              ; Restore nuber of sectors to be read
	cmp al, dl          ; Compare read sector to requested read size
	jne disk_read_error ; Jump on error
	ret

disk_read_error:
	mov bx, MSG_DISK_READ_ERROR ; Error
	call boot_sect_out          ; Output
	jmp halt					; Halt system

disk_load_error:
	mov bx, MSG_DISK_LOAD_ERROR ; Display error
	call boot_sect_out          ; Output
	jmp halt                    ; Halt the system

MSG_DISK_READ_ERROR:
	db "Error, can't load required amount of sector", 0xA, 0xD, 0
MSG_DISK_LOAD_ERROR:
	db "Error, can't issue load from disk", 0xA, 0xD, 0
