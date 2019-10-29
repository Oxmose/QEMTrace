; instruction_tracing.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; MiniKernel test: Check if all instruction are correctly traced even when
; repeated.

mov eax, 5
mov eax, 5
mov eax, 5
mov eax, 500
inst_tracing_loop:
dec eax
mov ebx, 5
cmp eax, 0
jne inst_tracing_loop
