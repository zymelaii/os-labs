; offset to each field in the stack frame
P_STACKBASE  equ 0
GSREG        equ P_STACKBASE
FSREG        equ GSREG + 4
ESREG        equ FSREG + 4
DSREG        equ ESREG + 4
EDIREG       equ DSREG + 4
ESIREG       equ EDIREG + 4
EBPREG       equ ESIREG + 4
KERNELESPREG equ EBPREG + 4
EBXREG       equ KERNELESPREG + 4
EDXREG       equ EBXREG + 4
ECXREG       equ EDXREG + 4
EAXREG       equ ECXREG + 4
RETADR       equ EAXREG + 4
EIPREG       equ RETADR + 4
CSREG        equ EIPREG + 4
EFLAGSREG    equ CSREG + 4
ESPREG       equ EFLAGSREG + 4
SSREG        equ ESPREG + 4
P_STACKTOP   equ SSREG + 4

; offset to tss.sp0
TSS3_S_SP0 equ 4

EOI           equ 0x20
INT_M_CTL     equ 0x20 ; <Master> I/O port for interrupt controller
INT_M_CTLMASK equ 0x21 ; <Master> setting bits in this port disables ints
INT_S_CTL     equ 0xa0 ; <Slave>  I/O port for second interrupt controller
INT_S_CTLMASK equ 0xa1 ; <Slave>  setting bits in this port disables ints

[SECTION .data]

error_code dd 0

[SECTION .text]

extern StackTop
extern to_kernel_stack
extern exception_handler
extern syscall_handler
extern page_fault_handler
extern irq_table

save:
    pushad                      ; `.
    push    ds                  ;  |
    push    es                  ;  | 保存原寄存器值
    push    fs                  ;  |
    push    gs                  ; /

    mov     dx, ss
    mov     ds, dx
    mov     es, dx

    mov     eax, esp            ; eax = 进程表起始地址

    test    dword [eax + CSREG - P_STACKBASE], 3
    jz      .1                  ; 根据段寄存器的状态信息判断是否发生内核重入
    mov     esp, StackTop       ; 如果没有发生内核重入就意味着要到用户栈了，先将 esp 移入临时内核栈
    push    eax                 ; 传入进程表首地址信息
    call    to_kernel_stack     ; 这个函数之后 esp 变为进程内核栈
    pop     eax                 ; 恢复进程表首地址信息（这个是 restart 第一句话要用的）
.1:
    push    eax                 ; 如果在内核栈重入，pop esp 不会有任何变化
    push    restart             ; 否则 eax 在进程表首地址，pop esp 会使 esp 移动到用户栈栈底
    jmp     [eax + RETADR - P_STACKBASE]

    global restart
restart:
    pop     esp                 ; 获悉当前的 esp 该到哪，不用管现在是否要回用户态，语义在 save 中有解释
    pop     gs
    pop     fs
    pop     es
    pop     ds
    popad
    add     esp, 4
    iretd

    global syscall_interrupt
syscall_interrupt:
    call    save
    sti
    call    syscall_handler
    cli
    ret

%macro impl_hwint_master 2
align 16
    global %1
%1:
    call    save
    in      al, INT_M_CTLMASK    ; `.
    or      al, (1 << %2)        ;  | 屏蔽当前中断
    out     INT_M_CTLMASK, al    ; /
    mov     al, EOI              ; `. 置 EOI 位
    out     INT_M_CTL, al        ; /
    sti                          ; CPU 在响应中断的过程中会自动关中断，这句之后就允许响应新的中断
    push    %2                   ; `.
    call    [irq_table + 4 * %2] ;  | 中断处理程序
    pop     ecx                  ; /
    cli
    in     al, INT_M_CTLMASK     ; `.
    and    al, ~(1 << %2)        ;  | 恢复接受当前中断
    out    INT_M_CTLMASK, al     ; /
    ret
%endmacro

%macro impl_hwint_slave 2
align 16
    global %1
%1:
    hlt                         ; 后面的 8 个外设中断暂时不需要，先 halt 休眠核
%endmacro

%macro impl_exception_no_errcode 3
    global %1
%1:
    call    save

    mov     eax, [esp + 4]      ; 旧的 esp
    mov     ebx, [eax + EFLAGSREG - P_STACKBASE]
    push    ebx
    mov     ebx, [eax + CSREG - P_STACKBASE]
    push    ebx
    mov     ebx, [eax + EIPREG - P_STACKBASE]
    push    ebx

    push    0xffffffff          ; 压入 error code 和向量标识
    push    %2
    call    %3                  ; 处理函数
    add     esp, 8

    ret
%endmacro

%macro impl_exception_errcode 3
    global %1
%1:
    xchg    eax, [esp]          ; 这个时候 eax 对应是 error code
    mov     [error_code], eax   ; 暂存一下
    pop     eax
    call    save

    mov     eax, [esp + 4]      ; 旧的 esp
    mov     ebx, [eax + EFLAGSREG - P_STACKBASE]
    push    ebx
    mov     ebx, [eax + CSREG - P_STACKBASE]
    push    ebx
    mov     ebx, [eax + EIPREG - P_STACKBASE]
    push    ebx

    mov     eax, [error_code]   ; 压入 error code 和向量标识
    push    eax
    push    %2
    call    %3                  ; 处理函数
    add     esp, 8

    ret
%endmacro

%if 0
impl_hwint_master hwint00, 0  ; interrupt routine for irq 0 (the clock)
%endif
align 16
    global hwint00
hwint00:
    call    save
    mov     al, EOI
    out     INT_M_CTL, al
    ; ATTENTION: disable interrupts through the whole clock interrupt routine
    push    0
    call    [irq_table + 0]
    pop     ecx
    ret

impl_hwint_master hwint01, 1  ; interrupt routine for irq 1 (keyboard)
impl_hwint_master hwint02, 2  ; interrupt routine for irq 2 (cascade)
impl_hwint_master hwint03, 3  ; interrupt routine for irq 3 (second serial)
impl_hwint_master hwint04, 4  ; interrupt routine for irq 4 (first serial)
impl_hwint_master hwint05, 5  ; interrupt routine for irq 5 (XT winchester)
impl_hwint_master hwint06, 6  ; interrupt routine for irq 6 (floppy)
impl_hwint_master hwint07, 7  ; interrupt routine for irq 7 (printer)
impl_hwint_slave  hwint08, 8  ; interrupt routine for irq 8 (realtime clock)
impl_hwint_slave  hwint09, 9  ; interrupt routine for irq 9 (irq 2 redirected)
impl_hwint_slave  hwint10, 10 ; interrupt routine for irq 10
impl_hwint_slave  hwint11, 11 ; interrupt routine for irq 11
impl_hwint_slave  hwint12, 12 ; interrupt routine for irq 12
impl_hwint_slave  hwint13, 13 ; interrupt routine for irq 13 (fpu exception)
impl_hwint_slave  hwint14, 14 ; interrupt routine for irq 14 (AT winchester)
impl_hwint_slave  hwint15, 15 ; interrupt routine for irq 15

impl_exception_no_errcode division_error,           0,  exception_handler  ; division error, fault, #DE, no error code
impl_exception_no_errcode debug_exception,          1,  exception_handler  ; debug exception, fault/trap, #DB, no error code
impl_exception_no_errcode nmi,                      2,  exception_handler  ; non-maskable interrupt, int, \, no error code
impl_exception_no_errcode breakpoint_exception,     3,  exception_handler  ; breakpoint exception, trap, #BP, no error code
impl_exception_no_errcode overflow_exception,       4,  exception_handler  ; overflow exception, trap, #OF, no error code
impl_exception_no_errcode bound_range_exceeded,     5,  exception_handler  ; bound range exceeded exception, fault, #BR, no error code
impl_exception_no_errcode invalid_opcode,           6,  exception_handler  ; invalid opcode, fault, #UD, no error code
impl_exception_no_errcode device_not_available,     7,  exception_handler  ; device not available, fault, #NM, no error code
impl_exception_errcode    double_fault,             8,  exception_handler  ; double fault, abort, #DF, error code = 0
impl_exception_no_errcode copr_seg_overrun,         9,  exception_handler  ; coprocessor segment overrun, fault, \, no error code
impl_exception_errcode    invalid_tss,              10, exception_handler  ; invalid tss, fault, #TS, error code
impl_exception_errcode    segment_not_present,      11, exception_handler  ; segment not present, fault, #NP, error code
impl_exception_errcode    stack_seg_exception,      12, exception_handler  ; stack-segment fault, fault, #SS, error code
impl_exception_errcode    general_protection,       13, exception_handler  ; general protection fault, fault, #PF, error code
impl_exception_errcode    page_fault,               14, page_fault_handler ; page fault, fault, #PF, error code
impl_exception_no_errcode floating_point_exception, 16, exception_handler  ; floating-point exception, fault, #MF, no error code
