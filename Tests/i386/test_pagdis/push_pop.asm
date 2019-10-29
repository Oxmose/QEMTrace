; push_pop.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; MiniKernel test: Check if all instruction are correctly traced even when
; repeated.

push eax
pushf
popf
pop eax
pusha
popa
push eax
push eax
push eax
push eax
push eax
pop eax
pop eax
pop eax
pop eax
pop eax
