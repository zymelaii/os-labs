#pragma once

#include <sys/types.h>

#define GDT_SIZE 128
#define LDT_SIZE GDT_SIZE
#define IDT_SIZE 256

//! selector used in our kernel
#define SELECTOR_DUMMY 0x00
#define SELECTOR_FLAT_C 0x08          //<! code seg
#define SELECTOR_FLAT_RW 0x10         //<! data seg
#define SELECTOR_VIDEO (0x18 | 0b11)  //<! vmem, rpl=3
#define SELECTOR_TSS 0x20  //<! 从外层跳到内存时 SS 和 ESP 的值从里面获得
#define SELECTOR_LDT 0x28

#define SELECTOR_KERNEL_CS SELECTOR_FLAT_C
#define SELECTOR_KERNEL_DS SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_GS SELECTOR_VIDEO

//! descriptor attribute
#define DA_32 0x4000        //<! 32 位段
#define DA_LIMIT_4K 0x8000  //<! 段界限粒度为 4K 字节
#define DA_DPL0 0x00        //<! DPL = 0
#define DA_DPL1 0x20        //<! DPL = 1
#define DA_DPL2 0x40        //<! DPL = 2
#define DA_DPL3 0x60        //<! DPL = 3

//! data seg type
#define DA_DR 0x90    //<! 存在的只读数据段类型值
#define DA_DRW 0x92   //<! 存在的可读写数据段属性值
#define DA_DRWA 0x93  //<! 存在的已访问可读写数据段类型值

//! code seg type
#define DA_C 0x98     //<! 存在的只执行代码段属性值
#define DA_CR 0x9a    //<! 存在的可执行可读代码段属性值
#define DA_CCO 0x9c   //<! 存在的只执行一致代码段属性值
#define DA_CCOR 0x9e  //<! 存在的可执行可读一致代码段属性值

//! system seg type
#define DA_LDT 0x82       //<! 局部描述符表段类型值
#define DA_TaskGate 0x85  //<! 任务门类型值
#define DA_386TSS 0x89    //<! 可用 386 任务状态段类型值
#define DA_386CGate 0x8c  //<! 386 调用门类型值
#define DA_386IGate 0x8e  //<! 386 中断门类型值
#define DA_386TGate 0x8f  //<! 386 陷阱门类型值

//! selector attribute
#define SA_MASK_RPL 0xfffc
#define SA_MASK_TI 0xfffb
#define SA_RPL0 0
#define SA_RPL1 1
#define SA_RPL2 2
#define SA_RPL3 3
#define SA_TIG 0
#define SA_TIL 4

#define RPL_KERNEL SA_RPL0
#define RPL_TASK SA_RPL1
#define RPL_USER SA_RPL3

typedef struct descriptor {
  u16 limit_low;      //<! limit 1
  u16 base_low;       //<! base 1
  u8 base_mid;        //<! base 2
  u8 attr1;           //<! attr 1 - P(1) | DPL(2) | DT(1) | TYPE(4)
  u8 limit_high : 4;  //<! limit 2
  u8 attr2 : 4;       //<! attr 2 - G(1) | D(1) | 0(1) | AVL(1)
  u8 base_high;       //<! base 3
} __attribute__((packed)) descriptor_t;

/*!
 * 关于 gate 部分字段的说明
 *
 * gate.dcount
 *
 *    该字段只在调用门描述符中有效，如果在利用调用门调用子程序时引
 *    起特权级的转换和堆栈的改变，需要将外层堆栈中的参数复制到内层
 *    堆栈。该双字计数字段就是用于说明这种情况发生时，要复制的双字
 *    参数的数量。
 */

typedef struct gate {
  u16 offset_low;   //<! offset 1
  u16 selector;     //<! selector
  u8 dcount;        //<! dword counter
  u8 attr;          //<! attr - P(1) | DPL(2) | DT(1) | TYPE(4)
  u16 offset_high;  //<! offset 2
} gate_t;

typedef struct selector {
  u16 rpl : 2;
  u16 ti : 1;
  u16 index : 13;
} __attribute__((packed)) selector_t;

//! pointer to descriptor table
typedef struct descptr_s {
  u16 limit;
  u32 base;
} __attribute__((packed)) descptr_t;

/*!
 * 关于 tss 部分字段的说明
 *
 * tss.esp0
 *
 *    当发生中断的时候 esp 就会变成 esp0
 *
 * tss.ss0
 *
 *    当发生中断的时候 ss 就会变成 ss0，由于 ss0 存储的是内核
 *    态权限段，于是顺利进入内核态
 *
 * tss.iobase
 *
 *    I/O 位图基址大于或等于 TSS 段界限，就表示没有 I/O 许可
 *    位图
 */

//! task state segment
typedef struct tss {
  u32 backlink;
  u32 esp0;
  u32 ss0;
  u32 esp1;
  u32 ss1;
  u32 esp2;
  u32 ss2;
  u32 cr3;
  u32 eip;
  u32 flags;
  u32 eax;
  u32 ecx;
  u32 edx;
  u32 ebx;
  u32 esp;
  u32 ebp;
  u32 esi;
  u32 edi;
  u32 es;
  u32 cs;
  u32 ss;
  u32 ds;
  u32 fs;
  u32 gs;
  u32 ldt;
  u16 trap;
  u16 iobase;
#if 0
  u8 iomap[2];
#endif
} __attribute__((packed)) tss_t;

extern descriptor_t gdt[GDT_SIZE];
extern descriptor_t ldt[LDT_SIZE];
extern gate_t idt[IDT_SIZE];
extern tss_t tss;
extern descptr_t gdt_ptr;
extern descptr_t ldt_ptr;

void init_protect_mode();
