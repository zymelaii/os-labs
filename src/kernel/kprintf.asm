[SECTION .text]
[BITS 32]

	global kprintf
kprintf:

	; TODO: impl your code here
	; NOTE: see terminal.h for the func proto of the method
	; NOTE: see lab guide step 3 for more details
	; NOTE: remove the following lines as long as you've finished your impl

	jmp    .sample
.debug_msg:
	db     "kprintf is invoked somewhere",0
.sample:
	mov    esi, .debug_msg
	mov    ebx, 0
	mov    ah, 0x0f
.sample.1:
	mov    cl, [esi]
    test   cl, cl
    jz     .sample.4
.sample.2:
    mov    al, cl
    mov    [gs:ebx], ax
    add    ebx, 2
.sample.3:
    inc    esi
    jmp    .sample.1
.sample.4:
	jmp    $
