; offset to each field in the kernel stack frame
P_STACKBASE equ 0
EFLAGSREG   equ P_STACKBASE
EDIREG      equ EFLAGSREG + 4
ESIREG      equ EDIREG + 4
EBPREG      equ ESIREG + 4
EBXREG      equ EBPREG + 4
EDXREG      equ EBXREG + 4
ECXREG      equ EDXREG + 4
EAXREG      equ ECXREG + 4
ESPREG      equ EAXREG + 4

[SECTION .text]
[BITS 32]

    global switch_kernel_context
switch_kernel_context:
    push    ebp
    mov     ebp, esp
    push    eax
    push    ebx
    mov     eax, [ebp + 8]
    mov     ebx, [ebp + 12]
    call    .inner_switch
    pop     ebx
    pop     eax
    pop     ebp
    ret
.inner_switch:
    mov     [eax + ESPREG], esp
    lea     esp, [eax + ESPREG]
    push    eax
    push    ecx
    push    edx
    push    ebx
    push    ebp
    push    esi
    push    edi
    pushf

    mov     esp, ebx

    popf
    pop     edi
    pop     esi
    pop     ebp
    pop     ebx
    pop     edx
    pop     ecx
    pop     eax
    pop     esp
    ret
