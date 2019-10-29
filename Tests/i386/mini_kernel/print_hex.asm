
print_hex_rm:
	pusha
    mov dx, bx
print_hex_loop_rm:
	cmp dx, 0x0000
	je  end_print_rm
	mov cx, dx
	and cx, 0xF000
	shr cx, 12
	cmp cx, 9
	jg  print_char_hex_rm
print_num_hex_rm:
	add  cx, 48
	call print_char_rm
	shl  dx, 4
	jmp  print_hex_loop_rm
print_char_hex_rm:
	add  cx, 55
	call print_char_rm
	shl  dx, 4
	jmp  print_hex_loop_rm
print_char_rm:
	mov al, cl            ; Printf current character
	mov ah, 0x0e          ; SAME
	int 0x10              ; SAME
	ret
end_print_rm:
	popa
	ret
