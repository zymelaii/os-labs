%include "consts.inc"

%macro WriteChar 3
    mov		[gs:((80 * (%1) + (%2)) * 2)], word 0x0f00 + %3
%endmacro

	org		OffsetOfLoader
_start:
	mov		ax, 0xb800
	mov		gs, ax

	WriteChar 12 - 1, 28 + 0 + 0 * 2, 'H'
	WriteChar 12 - 1, 28 + 0 + 1 * 2, 'e'
	WriteChar 12 - 1, 28 + 0 + 2 * 2, 'l'
	WriteChar 12 - 1, 28 + 0 + 3 * 2, 'l'
	WriteChar 12 - 1, 28 + 0 + 4 * 2, 'o'
	WriteChar 12 - 1, 28 + 0 + 5 * 2, ','
	WriteChar 12 + 1, 28 + 5 + 0 * 2, 'W'
	WriteChar 12 + 1, 28 + 5 + 1 * 2, 'o'
	WriteChar 12 + 1, 28 + 5 + 2 * 2, 'r'
	WriteChar 12 + 1, 28 + 5 + 3 * 2, 'l'
	WriteChar 12 + 1, 28 + 5 + 4 * 2, 'd'
	WriteChar 12 + 1, 28 + 5 + 5 * 2, '!'

	jmp		$
