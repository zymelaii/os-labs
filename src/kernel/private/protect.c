#include <arch/x86.h>
#include <interrupt.h>
#include <layout.h>
#include <protect.h>
#include <serial.h>
#include <string.h>

descriptor_t gdt[GDT_SIZE];
descriptor_t ldt[LDT_SIZE];
gate_t idt[IDT_SIZE];
tss_t tss;
descptr_t gdt_ptr;
descptr_t idt_ptr;

static void init_descriptor(descriptor_t *desc, u32 base, u32 limit, u16 attr) {
  desc->limit_low = limit & 0xffff;
  desc->base_low = base & 0xffff;
  desc->base_mid = (base >> 16) & 0xff;
  desc->attr1 = attr & 0xff;
  desc->limit_high = (limit >> 16) & 0xf;
  desc->attr2 = (attr >> 12) & 0xf;
  desc->base_high = (base >> 24) & 0xff;
}

static void init_idt_desc(u8 vector, u8 type, int_handler_t handler, u8 rpl) {
  gate_t *gate = &idt[vector];
  u32 base = (u32)handler;
  gate->offset_low = base & 0xffff;
  gate->selector = SELECTOR_FLAT_C;
  gate->dcount = 0;
  gate->attr = type | (rpl << 5);
  gate->offset_high = (base >> 16) & 0xffff;
}

//! \brief initialize global descriptor table
static void init_gdt() {
  memset(&tss, 0, sizeof(tss));
  tss.ss0 = SELECTOR_FLAT_RW;
  tss.iobase = sizeof(tss);  //<! no io bitmap

  init_descriptor(&gdt[0], 0, 0, 0);
  init_descriptor(&gdt[1], 0, 0xfffff, DA_CR | DA_32 | DA_LIMIT_4K);
  init_descriptor(&gdt[2], 0, 0xfffff, DA_DRW | DA_32 | DA_LIMIT_4K);
  init_descriptor(&gdt[3], (u32)K_PHY2LIN(0xb8000), 0xffff, DA_DRW | DA_DPL3);
  init_descriptor(&gdt[4], (u32)&tss, sizeof(tss) - 1, DA_386TSS);
  init_descriptor(&gdt[5], (u32)ldt, sizeof(ldt) - 1, DA_LDT);

  gdt_ptr.base = (u32)gdt;
  gdt_ptr.limit = sizeof(gdt) - 1;
}

//! \brief initialize local descriptor table
static void init_ldt() {
  //! NOTE: ldt is not a popular solution in modern x86, we prefers to use gdt
  //! for both kernel and user space, but in our labs, gdt is only used for
  //! kernel space while user space would use ldt to descript its own segments

  init_descriptor(&ldt[0], 0, 0, 0);
  init_descriptor(&ldt[1], 0, 0xfffff, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL3);
  init_descriptor(&ldt[2], 0, 0xfffff, DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL3);
}

//! \brief initialize interrupt descriptor table
static void init_idt() {
  init_idt_desc(INT_VECTOR_DIVIDE, DA_386IGate, division_error, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_DEBUG, DA_386IGate, debug_exception, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_NMI, DA_386IGate, nmi, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_BREAKPOINT, DA_386IGate, breakpoint_exception,
                RPL_USER);
  init_idt_desc(INT_VECTOR_OVERFLOW, DA_386IGate, overflow_exception, RPL_USER);
  init_idt_desc(INT_VECTOR_BOUNDS, DA_386IGate, bound_range_exceeded,
                RPL_KERNEL);
  init_idt_desc(INT_VECTOR_INVAL_OP, DA_386IGate, invalid_opcode, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_DEVICE_NOT, DA_386IGate, device_not_available,
                RPL_KERNEL);
  init_idt_desc(INT_VECTOR_DOUBLE_FAULT, DA_386IGate, double_fault, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_COPROC_SEG, DA_386IGate, copr_seg_overrun,
                RPL_KERNEL);
  init_idt_desc(INT_VECTOR_INVAL_TSS, DA_386IGate, invalid_tss, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_SEG_NOT, DA_386IGate, segment_not_present,
                RPL_KERNEL);
  init_idt_desc(INT_VECTOR_STACK_FAULT, DA_386IGate, stack_seg_exception,
                RPL_KERNEL);
  init_idt_desc(INT_VECTOR_PROTECTION, DA_386IGate, general_protection,
                RPL_KERNEL);
  init_idt_desc(INT_VECTOR_PAGE_FAULT, DA_386IGate, page_fault, RPL_KERNEL);
  init_idt_desc(INV_VECTOR_FP_EXCEPTION, DA_386IGate, floating_point_exception,
                RPL_KERNEL);
  init_idt_desc(INT_VECTOR_SYSCALL, DA_386IGate, syscall_interrupt, RPL_USER);

  init_idt_desc(INT_VECTOR_IRQ0 + 0, DA_386IGate, hwint00, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ0 + 1, DA_386IGate, hwint01, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ0 + 2, DA_386IGate, hwint02, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ0 + 3, DA_386IGate, hwint03, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ0 + 4, DA_386IGate, hwint04, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ0 + 5, DA_386IGate, hwint05, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ0 + 6, DA_386IGate, hwint06, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ0 + 7, DA_386IGate, hwint07, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ8 + 0, DA_386IGate, hwint08, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ8 + 1, DA_386IGate, hwint09, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ8 + 2, DA_386IGate, hwint10, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ8 + 3, DA_386IGate, hwint11, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ8 + 4, DA_386IGate, hwint12, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ8 + 5, DA_386IGate, hwint13, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ8 + 6, DA_386IGate, hwint14, RPL_KERNEL);
  init_idt_desc(INT_VECTOR_IRQ8 + 7, DA_386IGate, hwint15, RPL_KERNEL);

  idt_ptr.base = (u32)idt;
  idt_ptr.limit = sizeof(idt) - 1;
}

void init_protect_mode() {
  init_gdt();
  init_ldt();

  init_interrupt_controller();
  init_idt();
}
