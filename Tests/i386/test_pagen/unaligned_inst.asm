; unaligned_inst.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; MiniKernel test: Check if all instructions are correctly traced even when
; non correctly aligned

jmp page_0

; Relocate the code
reloc_code:
mov esi, code_start            ; Start address of the source code
mov edi, eax                   ; Start address of relocation base
mov ecx, code_end - code_start ; Size of the relocation
cld                            ; Clear direction flag
rep movsb                      ; Copy the code
ret

page_0:
; Unaligned code access on two pages
mov eax, 0x0000AFFE
call reloc_code
mov ecx, line_0
jmp eax

line_0:
; Unaligned code access on two cache lines (32B/64B/128B)
mov eax, 0x0000B07E
call reloc_code
mov ecx, page_1
jmp eax

page_1:
; Unaligned code access on two pages
mov eax, 0x00010DFFE
call reloc_code
mov ecx, line_1
jmp eax

line_1:
; Unaligned code access on two cache lines (32B/64B/128B)
mov eax, 0x0010D07E
call reloc_code
mov ecx, code_end
jmp eax

code_start:

mov eax, 0x00300000
mov ebx, 0xDEADBEEF

mov [eax], ebx
mov ebx, [eax]
mov [eax], bx
mov bx, [eax]
mov [eax], bl
mov bl, [eax]

jmp ecx

code_end:
nop
