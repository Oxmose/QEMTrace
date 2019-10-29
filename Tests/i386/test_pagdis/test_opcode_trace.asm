; test_opcode_trace.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; MiniKernel test: Start tracing Qemu custom instruction
; The Qemu emulator should manage this instruction and strart
; tracing the system memory access.

mov eax, 0x300000

; Disable tracing: 0x0FA700
db 0x0F
db 0xA7
db 0x00
db 0x00

mov ebx, [eax]
mov ebx, [eax]
mov ebx, [eax]

; Enable tracing: 0x0FA600
db 0x0F
db 0xA6
db 0x00
db 0x00

mov eax, 0x300000
mov [eax], ebx
mov [eax], ebx
mov [eax], ebx
