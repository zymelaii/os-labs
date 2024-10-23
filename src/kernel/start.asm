SELECTOR_KERNEL_CS equ 0x08
SELECTOR_KERNEL_DS equ 0x10
SELECTOR_KERNEL_GS equ 0x18 | 0b11
SELECTOR_TSS       equ 0x20
SELECTOR_LDT       equ 0x28

[SECTION .bss]

global StackTop

resb 4096
StackTop:

[SECTION .text]

extern gdt_ptr
extern idt_ptr

extern cstart
extern kernel_main

    global _start
_start:
    mov     esp, StackTop

    call    cstart

    lgdt    [gdt_ptr]           ; load new gdt
    mov     ax, SELECTOR_LDT
    lldt    ax                  ; load ldt
    mov     ax, SELECTOR_TSS
    ltr     ax                  ; load tss
    lidt    [idt_ptr]           ; load idt

    ; use a long jmp to activate the new gdt immediately
    jmp     SELECTOR_KERNEL_CS:cinit

    global cinit
cinit:
    mov     ax, SELECTOR_KERNEL_DS
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     ss, ax
    mov     ax, SELECTOR_KERNEL_GS
    mov     gs, ax

    jmp     kernel_main
