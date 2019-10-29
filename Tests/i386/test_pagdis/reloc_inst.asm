; reloc_inst.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; MiniKernel test: Check if all instruction are correctly traced even when
; repeated and relocated.

; Relloc at 2Mb
%define reloc_base 0x0200000

; Relocate the code
mov esi, code_start            ; Start address of the source code
mov edi, reloc_base            ; Start address of relocation base
mov ecx, code_end - code_start ; Size of the relocation
cld                            ; Clear direction flag
rep movsb                      ; Copy the code

jmp reloc_base      ; Jump to copied code

code_start:

mov eax, 0x300000
mov ebx, 0xDEADBEEF

mov [eax], ebx
mov ebx, [eax]
mov [eax], bx
mov bx, [eax]
mov [eax], bl
mov bl, [eax]

mov eax, code_end
jmp eax

code_end:
nop
