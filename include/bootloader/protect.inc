; NOTE: for struct of segment descriptor, see: https://en.wikipedia.org/wiki/Segment_descriptor

; ===
; 关于 `Type`
; ---
; 对于数据段
;     0x0     只读
;     0x1     只读 已访问
;     0x2     读/写
;     0x3     读/写 已访问
;     0x4     只读 向下扩展
;     0x5     只读 向下扩展 已访问
;     0x6     读/写 向下扩展
;     0x7     读/写 向下扩展 已访问
;
; 对于代码段
;     0x8    只执行
;     0x9    只执行 已访问
;     0xa    执行/读
;     0xb    执行/读 已访问
;     0xc    只执行 一致码段
;     0xd    只执行 一致码段 已访问
;     0xe    执行/读 一致码段
;     0xf    执行/读 一致码段 已访问
;
; 对于系统段
;     0x0    未定义
;     0x1    可用 286TSS
;     0x2    LDT
;     0x3    忙的 286TSS
;     0x4    286 调用门
;     0x5    任务门
;     0x6    286 中断门
;     0x7    286 陷阱门
;     0x8    未定义
;     0x9    可用 386TSS
;     0xa    未定义
;     0xb    忙的 386TSS
;     0xc    386 调用门
;     0xd    未定义
;     0xe    386 中断门
;     0xf    386 陷阱门
; ...

; ===
; 关于 `D`
; ---
; 对于可执行段，决定指令使用的地址及操作数的默认大小，可以使用地址大小前缀和操作数大小前缀分别改变默认的地址或操作数的大小
;    set     使用 32 位地址及 32 位或 8 位操作数，称作 32 位代码段
;    clear   使用 16 位地址及 16 位或 8 位操作数，称作 16 位代码段，与 80286 兼容
;
; 对于向下扩展数据段，决定段的上边界
;    set     上界限为 4G
;    clear   上界限为 64K，与 80286 兼容
;
; 对于由 ss 寄存器寻址的段（通常为堆栈段），决定隐式的堆栈访问指令（如 push、pop）使用何种堆栈指针寄存器
;    set     使用 32 位堆栈指针寄存器 esp
;    clear   使用 16 位堆栈指针寄存器 sp，与 80286 兼容

DA_32       equ 0x4000 ; D/B=1
DA_LIMIT_4K equ 0x8000 ; G=1

DA_DPL0 equ 0x00 ; DPL=0
DA_DPL1 equ 0x20 ; DPL=1
DA_DPL2 equ 0x40 ; DPL=2
DA_DPL3 equ 0x60 ; DPL=3

; field `type` of code/data descriptor
DA_DR   equ 0x90 ; 存在的只读数据段类型值
DA_DRW  equ 0x92 ; 存在的可读写数据段属性值
DA_DRWA equ 0x93 ; 存在的已访问可读写数据段类型值
DA_C    equ 0x98 ; 存在的只执行代码段属性值
DA_CR   equ 0x9a ; 存在的可执行可读代码段属性值
DA_CCO  equ 0x9c ; 存在的只执行一致代码段属性值
DA_CCOR equ 0x9e ; 存在的可执行可读一致代码段属性值

; field `type` of segment descriptor
DA_LDT      equ 0x82 ; 局部描述符表段类型值
DA_TaskGate equ 0x85 ; 任务门类型值
DA_386TSS   equ 0x89 ; 可用 386 任务状态段类型值
DA_386CGate equ 0x8c ; 386 调用门类型值
DA_386IGate equ 0x8e ; 386 中断门类型值
DA_386TGate equ 0x8f ; 386 陷阱门类型值

; data layout of selector
;
;    ┏━━┳━━┳━━┳━━┳━━┳━━┳━━┳━━┳━━┳━━┳━━┳━━┳━━┳━━┳━━┳━━┓
;    ┃15┃14┃13┃12┃11┃10┃09┃08┃07┃06┃05┃04┃03┃02┃01┃00┃
;    ┣━━┻━━┻━━┻━━┻━━┻━━┻━━┻━━┻━━┻━━┻━━┻━━┻━━╋━━╋━━┻━━┫
;    ┃                INDEX                 ┃TI┃ RPL ┃
;    ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━┻━━━━━┛

; Selector Attribute - Requested Privilege Level, RPL
SA_RPL0 equ 0 ; RPL=0
SA_RPL1 equ 1 ; RPL=1
SA_RPL2 equ 2 ; RPL=2
SA_RPL3 equ 3 ; RPL=3

; Selector Attribute - Table Indicator, TI
SA_TIG equ 0 ; TI=0, load descriptor from gdt
SA_TIL equ 4 ; TI=1, load descriptor from ldt

; \usage Descriptor base, limit, attr
; \param base dd
; \param limit dd low 20 bits available
; \param attr dw lower 4 bits of higher byte are always 0
%macro Descriptor 3
    dw	%2 & 0xffff                          ; seg-limit 1
    dw  %1 & 0xffff                          ; seg-base 1
    db  (%1 >> 16) & 0xff                    ; seg-base 2
    dw  ((%2 >> 8) & 0xf00) | (%3 & 0xf0ff)  ; attr 1 | seg-limit 2 | attr 2
    db  (%1 >> 24) & 0xff                    ; seg-base 3
%endmacro

; \usage Gate selector, offset, dcount, attr
; \param selector dw
; \param offset dd
; \param dcount db
; \param attr db
%macro Gate 4
    dw  (%2 & 0xffff)                        ; offset 1
    dw  %1                                   ; selector
    dw  (%3 & 0x1f) | ((%4 << 8) & 0xff00)   ; attr
    dw  ((%2 >> 16) & 0xffff)                ; offset 2
%endmacro

; =========================================
; FLAGS - Intel 8086 Family Flags Register
; =========================================
;
;    |11|10|F|E|D|C|B|A|9|8|7|6|5|4|3|2|1|0|
;      |  | | | | | | | | | | | | | | | | '---  CF……Carry Flag
;      |  | | | | | | | | | | | | | | | '---  1
;      |  | | | | | | | | | | | | | | '---  PF……Parity Flag
;      |  | | | | | | | | | | | | | '---  0
;      |  | | | | | | | | | | | | '---  AF……Auxiliary Flag
;      |  | | | | | | | | | | | '---  0
;      |  | | | | | | | | | | '---  ZF……Zero Flag
;      |  | | | | | | | | | '---  SF……Sign Flag
;      |  | | | | | | | | '---  TF……Trap Flag  (Single Step)
;      |  | | | | | | | '---  IF……Interrupt Flag
;      |  | | | | | | '---  DF……Direction Flag
;      |  | | | | | '---  OF……Overflow flag
;      |  | | | '-----  IOPL……I/O Privilege Level  (286+ only)
;      |  | | '-----  NT……Nested Task Flag  (286+ only)
;      |  | '-----  0
;      |  '-----  RF……Resume Flag (386+ only)
;      '------  VM……Virtual Mode Flag (386+ only)
;
; NOTE: see {pushf, popf, sti, cli, std, cld}
