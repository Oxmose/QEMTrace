; unaligned_data_access.asm
;
; Author  Alexy Torres Aurora Dugo
; Version 1.0
;
; MiniKernel test: Check behaviours on unaligned data access.
; An unaligned data access can overlapp two pages or two cache lines.

; Unaligned data access on two pages
mov eax, 0x0000AFFE
mov [eax], ebx
mov [eax], ebx
mov ebx, [eax]
mov ebx, [eax]

; Unaligned data access on two cache lines (32B/64B/128B)
mov eax, 0x0000B07E
mov [eax], ebx
mov [eax], ebx
mov ebx, [eax]
mov ebx, [eax]

; Unaligned data access on two pages
mov eax, 0x00010DFFE
mov [eax], ebx
mov [eax], ebx
mov ebx, [eax]
mov ebx, [eax]

; Unaligned data access on two cache lines (32B/64B/128B)
mov eax, 0x0010D07E
mov [eax], ebx
mov [eax], ebx
mov ebx, [eax]
mov ebx, [eax]
