; multiple_access.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; MiniKernel test: Check if one instruction loading/storing multiple data is
; correctly traced


call test_call
jmp next_inst

test_call:
ret

next_inst:
pusha
popa

pushf
popf
