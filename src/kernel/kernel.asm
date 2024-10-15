[SECTION .bss]

resb 4096
StackTop:

[SECTION .text]

extern cstart
extern gdt_ptr

    global _start
_start:
    mov     esp, StackTop

    sgdt    [gdt_ptr]           ; save current gdt to gdt_ptr
    call    cstart              ; gdt_ptr will be updated in cstart
    lgdt    [gdt_ptr]           ; load gdt from new gdt_ptr

    ; use a long jmp to activate the new gdt immediately
    SELECTOR_KERNEL_CS equ 0x08
    jmp     SELECTOR_KERNEL_CS:csinit

csinit:
    hlt
