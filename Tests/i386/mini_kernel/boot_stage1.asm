; boot_stage1.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; NASM boot code, bootstrap the kernel and call the entry point.
; Set flat mode GDT
; Set PM stack
; Switch to protected mode
; Set pagging refer to MiniKernel i386 memory map
; Enables Qemu memory tracing
; Call boot stage 2 entry point

;-----------------------------------------------------------
; SEGMENT DESCRIPTOR
;-----------------------------------------------------------
%define CODE32 0x0008
%define DATA32 0x0010
%define CODE16 0x0018
%define DATA16 0x0020

%define IDT_SIZE 2048
%define IDT_BASE 0x5000

KERNEL_STACK_BASE  equ 0xEA00 ; Kernel stack base
KERNEL_STACK_SIZE  equ 0x4000 ; 16K kernel stack

BOOT_STAGE2        equ 0x8600 ; Next stage entry point

PAGE_DIR_LOC         equ 0x100000 ; Page dire location in memory
FIRST_PAGE_TABLE_LOC equ 0x101000 ; First page table
PAGE_TABLE_COUNT     equ 1

[bits 16]
[org 0x7e00]

push ax
mov ax, 0x0 ; Reinit
mov es, ax  ; memory
mov ds, ax  ; segments
pop ax

mov [BOOT_DEVICE], ax ; Save boot device id

; Message from bootloader
	mov bx, MSG_BOOTSTAGE1_WELCOME
	call boot_sect_out

; Enable A20 gate
	call enable_a20

; Set basic GDT
    lgdt [gdt16_ptr]

; Set basic IDT
	lidt [idt_ptr]

; Set PMode
	mov  eax, cr0
	inc  eax
	mov  cr0, eax

	jmp  dword CODE32:pm_entry  ; Jump to PM mode

; We never get here
halt:
	hlt
	jmp halt

; Include assembly code
%include "a20_enabler.asm"
%include "boot_sect_output.asm"
%include "boot_sect_output_pm.asm"

[bits 32]
pm_entry:
; Load segment registers
	mov  ax, DATA32
	mov  ds, ax
	mov  es, ax
	mov  fs, ax
	mov  gs, ax
	mov  ss, ax

	mov eax, 6
	mov ebx, 0
	mov ecx, MSG_BOOTSTAGE1_PM_WELCOME
	call boot_sect_out_pm

; Load stack
	mov esp, KERNEL_STACK_BASE
	mov ebp, esp

	mov eax, 7
	mov ebx, 0
	mov ecx, MSG_BOOTSTAGE1_PM_STACK_SET
	call boot_sect_out_pm

; Set page directory
	mov ecx, 0
	mov ebx, 2
	mov eax, PAGE_DIR_LOC
loop_dir_fill:
	mov [eax], ebx
	add eax, 4
	inc ecx
	cmp ecx, 1024
	jne loop_dir_fill

; Load first page table in page dir
	mov eax, PAGE_DIR_LOC
	mov ebx, FIRST_PAGE_TABLE_LOC
	or  ebx, 3
	mov [eax], ebx

	mov eax, 8
	mov ebx, 0
	mov ecx, MSG_BOOTSTAGE1_PAGING_PGDIR_SET
	call boot_sect_out_pm

; Set first page table
	; Map the first 268 entries 1:1
	mov eax, FIRST_PAGE_TABLE_LOC
	mov ebx, 3
loop_map_one:
	mov [eax], ebx
	add eax, 4
	add ebx, 0x1000
	cmp eax, 0x101434
	jne loop_map_one

	; Invert map the rest of the entries
	mov ebx, 0x003FF003
loop_map_inv:
	mov [eax], ebx
	add eax, 4
	sub ebx, 0x1000
	cmp eax, 0x102000
	jne loop_map_inv

	mov eax, 9
	mov ebx, 0
	mov ecx, MSG_BOOTSTAGE1_PAGING_PAGE_SET
	call boot_sect_out_pm

; Enable paging
enablePaging:
	mov eax, PAGE_DIR_LOC
	mov cr3, eax
	mov eax, cr0
	or  eax, 0x80000000
	mov cr0, eax

	mov eax, 10
	mov ebx, 0
	mov ecx, MSG_BOOTSTAGE1_PAGING_EN
	call boot_sect_out_pm

	mov eax, 11
	mov ebx, 0
	mov ecx, MSG_BOOTSTAGE1_EN_TRACE_TEST
	call boot_sect_out_pm

; Enable Qemu tracing: 0x0FA6
db 0x0F
db 0xA6
db 0x00
db 0x00

; Call the loader
    jmp BOOT_STAGE2

; We should never get here
halt_pm:
	hlt
	jmp halt_pm

; Messages
MSG_BOOTSTAGE1_WELCOME: db "Bootstage 1" , 0xA, 0xD, 0
MSG_BOOTSTAGE1_PM_WELCOME: db "Protected Mode enabled", 0
MSG_BOOTSTAGE1_PM_STACK_SET: db "PM Stack set", 0
MSG_BOOTSTAGE1_PAGING_PGDIR_SET: db "Page Dir set", 0
MSG_BOOTSTAGE1_PAGING_PAGE_SET: db "First page set", 0
MSG_BOOTSTAGE1_PAGING_EN: db "Paging enabled", 0
MSG_BOOTSTAGE1_EN_TRACE_TEST: db "Enabling Qemu trace and testing", 0

; Runtime variable
BOOT_DEVICE: db 0  ; Boot device ID

gdt16:                                     ; GDT descriptor table
	.null:
		dd 0x00000000
		dd 0x00000000

	.code_32:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x9A
		db 0xCF
		db 0x00

	.data_32:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x92
		db 0xCF
		db 0x00

	.code_16:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x9A
		db 0x0F
		db 0x00

	.data_16:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x92
		db 0x0F
		db 0x00

gdt16_ptr:                                 ; GDT pointer for 16bit access
	dw gdt16_ptr - gdt16 - 1               ; GDT limit
	dd gdt16                               ; GDT base address

idt_ptr:
	dw IDT_SIZE - 1
	dd IDT_BASE

; Pad rest of the memory
times 2044-($-$$) db 0
times 4 db 0xFF
