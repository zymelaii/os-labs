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

[SECTION .text]

extern StackTop
extern tss
extern k_reenter
extern exception_handler
extern irq_table
extern p_proc_ready

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

    inc     dword [k_reenter]             ; ++k_reenter;
    cmp     dword [k_reenter], 0          ; if (k_reenter == 0)
    jne     .1                            ; {
    mov     esp, StackTop                 ;   mov esp, StackTop     <-- 切换到内核栈
    push    restart                       ;   push restart
    jmp     [eax + RETADR - P_STACKBASE]  ;   return
.1:                                       ; } else {                <-- 已经在内核栈，不需要再切换
    push    restart_reenter               ;   push restart_reenter
    jmp     [eax + RETADR - P_STACKBASE]  ;   return
                                          ; }

    global restart
restart:
    mov     esp, [p_proc_ready]
    lea     eax, [esp + P_STACKTOP]
    mov     dword [tss + TSS3_S_SP0], eax
restart_reenter:
    cli
    dec     dword [k_reenter]
    pop     gs
    pop     fs
    pop     es
    pop     ds
    popad
    add     esp, 4
    iretd

exception:
    call    exception_handler
    add     esp, 4 * 2          ; 让栈顶指向 eip，堆栈中从顶向下依次是 eip cs eflags
    hlt

%macro impl_hwint_master 2
align 16
    global %1
%1:
    call    save
    in      al, INT_M_CTLMASK    ; `.
    or      al, (1 << %2)        ;  | 屏蔽当前中断
    out     INT_M_CTLMASK, al    ; /
    mov     al, EOI              ; `. 置EOI位
    out     INT_M_CTL, al        ; /
    sti                          ; CPU在响应中断的过程中会自动关中断，这句之后就允许响应新的中断
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

%macro impl_exception_no_errcode 2
    global %1
%1:
    push    0xffffffff
    push    %2
    jmp     exception
%endmacro

%macro impl_exception_errcode 2
    global %1
%1:
    push    %2
    jmp     exception
%endmacro

impl_hwint_master hwint00, 0  ; interrupt routine for irq 0 (the clock)
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

impl_exception_no_errcode division_error,           0  ; division error, fault, #DE, no error code
impl_exception_no_errcode debug_exception,          1  ; debug exception, fault/trap, #DB, no error code
impl_exception_no_errcode nmi,                      2  ; non-maskable interrupt, int, \, no error code
impl_exception_no_errcode breakpoint_exception,     3  ; breakpoint exception, trap, #BP, no error code
impl_exception_no_errcode overflow_exception,       4  ; overflow exception, trap, #OF, no error code
impl_exception_no_errcode bound_range_exceeded,     5  ; bound range exceeded exception, fault, #BR, no error code
impl_exception_no_errcode invalid_opcode,           6  ; invalid opcode, fault, #UD, no error code
impl_exception_no_errcode device_not_available,     7  ; device not available, fault, #NM, no error code
impl_exception_errcode    double_fault,             8  ; double fault, abort, #DF, error code = 0
impl_exception_no_errcode copr_seg_overrun,         9  ; coprocessor segment overrun, fault, \, no error code
impl_exception_errcode    invalid_tss,              10 ; invalid tss, fault, #TS, error code
impl_exception_errcode    segment_not_present,      11 ; segment not present, fault, #NP, error code
impl_exception_errcode    stack_seg_exception,      12 ; stack-segment fault, fault, #SS, error code
impl_exception_errcode    general_protection,       13 ; general protection fault, fault, #PF, error code
impl_exception_errcode    page_fault,               14 ; page fault, fault, #PF, error code
impl_exception_no_errcode floating_point_exception, 16 ; floating-point exception, fault, #MF, no error code
