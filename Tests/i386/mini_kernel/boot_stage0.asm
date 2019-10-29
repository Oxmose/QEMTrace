; boot_stage0.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; NASM boot code, bootstrap the real bootloader
; Populate the memory with the actual bootloader

[org 0x7C00] ; BIOS offset

; Set Boot drive from bios
mov [BOOT_DRIVE], dl

; Clear screen
mov al, 0x03
mov ah, 0x00
int 0x10

; Invisible cursor
mov ah, 0x01
mov cx, 0x2607
int 0x10

; Set RM stack base
mov bp, STACK_BASE_RM   ; Set stack base
mov sp, bp              ; Set stack pointer

; Output message
mov bx, MSG_BOOTSTAGE0_BOOT_DEVICE
call boot_sect_out

; Display boot sector
mov bx, [BOOT_DRIVE]
call boot_sect_out_hex

; New line
mov bx, MSG_BOOTSTAGE0_ENDLINE
call boot_sect_out

; Init segments
xor ax, ax
mov es, ax
mov ds, ax

; Canonical CS:EIP
jmp boot_stage0

boot_stage0:
; Welcome message
	mov bx, MSG_BOOTSTAGE0_WELCOME
	call boot_sect_out

; Disable interrupts
	cli
	mov bx, MSG_BOOTSTAGE0_CLI
	call boot_sect_out

; Message load second part of the bootloader
	mov bx, MSG_BOOTSTAGE0_LOADING_BOOT
	call boot_sect_out

; Load the rest of the bootloader in the memory
load_bootloader:
	mov bx, BOOT_STAGE1        ; Load storage address
	mov dx, [BOOT_DRIVE]       ; Load the device to read from
	mov cx, BOOT_STAGE1_SECTOR ; Load the first sector to read
	mov ax, BOOT_STAGE1_SIZE   ; Load the number of sectors to read

	call disk_load ; Load stage 1

	; Load the second boot stage
	mov bx, BOOT_STAGE2        ; Load storage address
	mov dx, [BOOT_DRIVE]       ; Load the device to read from
	mov cx, BOOT_STAGE2_SECTOR ; Load the first sector to read
	mov ax, BOOT_STAGE2_SIZE   ; Load the number of sectors to read

	call disk_load ; Load stage 2

	; Load the IDT
	mov bx, IDT_BASE     ; Load storage address
	mov dx, [BOOT_DRIVE] ; Load the device to read from
	mov cx, IDT_SECTOR   ; Load the first sector to read
	mov ax, IDT_SIZE     ; Load the number of sectors to read

	call disk_load ; Load IDT

; Go to boot stage 1
	mov ax, [BOOT_DRIVE] ; Save boot device ID
	call BOOT_STAGE1     ; Go to stage 1

; Halt function, there must be only one
halt:
	hlt
	jmp halt

%include "boot_sect_output.asm" ; Output on screen
%include "hard_drive.asm"       ; Load from disk

; All messages of the boot sequence are here
MSG_BOOTSTAGE0_WELCOME:
	db "Bootstage 0", 0xA, 0xD, 0
MSG_BOOTSTAGE0_CLI:
	db "Interrupts disabled", 0xA, 0xD, 0
MSG_BOOTSTAGE0_STACK:
	db "Stack initialized", 0xA, 0xD, 0
MSG_BOOTSTAGE0_LOADING_BOOT:
	db "Loading bootstage 1 and 2", 0xA, 0xD, 0
MSG_BOOTSTAGE0_BOOT_DEVICE:
	db "Boot device ID: ", 0
MSG_BOOTSTAGE0_ENDLINE:
	db 0xA, 0xD, 0

; All boot settings are here
BOOT_DRIVE: db 0 ; Boot device ID

BOOT_STAGE1        equ 0x7e00  ; Next stage entry point
BOOT_STAGE1_SIZE   equ 4       ; Next stage sector size
BOOT_STAGE1_SECTOR equ 2       ; Start sector of the stage 1
BOOT_STAGE2        equ 0x8600  ; Next stage entry point
BOOT_STAGE2_SIZE   equ 10      ; Next stage sector size
BOOT_STAGE2_SECTOR equ 6       ; Start sector of the stage 2
STACK_BASE_RM      equ 0x9E00  ; Stack base for real mode
IDT_BASE           equ 0x5000  ; IDT memory address
IDT_SECTOR         equ 16      ; Start sector of the stage 1
IDT_SIZE           equ 4       ; IDT sector size

; Boot sector padding
times 510-($-$$) db 0
dw 0xaa55
