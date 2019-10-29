; interrupts.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; MiniKernel test: Check qemu tracing behaviour during interrupts, traps
; and exceptions.

; Relloc
%define reloc_base_0  0x00001000
%define reloc_base_32 0x00002000

; Relocate the code
mov esi, handler_32            ; Start address of the source code
mov edi, reloc_base_32         ; Start address of relocation base
mov ecx, handler_32_end - handler_32 ; Size of the relocation
cld                            ; Clear direction flag
rep movsb                      ; Copy the code

; Relocate the code
mov esi, handler_0             ; Start address of the source code
mov edi, reloc_base_0          ; Start address of relocation base
mov ecx, handler_0_end - handler_0 ; Size of the relocation
cld                            ; Clear direction flag
rep movsb                      ; Copy the code

; Soft Int 32
int 32

; Div by 0 exception
mov eax, 0
mov ebx, 0
div ebx

jmp end

handler_32:
    nop
    pusha
    mov eax, 0x00300000
    mov ebx, 0xDEADBEEF
    mov [eax], ebx
    mov ebx, [eax]
    popa
    iret
handler_32_end:

handler_0:
    nop
    pusha
    mov eax, 0x00300000
    mov ebx, 0xDEADBEEF
    mov [eax], ebx
    mov ebx, [eax]
    popa

    pop eax  ; pop eip
    inc eax  ; eip + 1
    push eax ; push eip

    iret
handler_0_end:

end:
