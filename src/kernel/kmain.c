#include <arch/x86.h>
#include <assert.h>
#include <gameplay.h>
#include <keyboard.h>
#include <process.h>
#include <protect.h>
#include <stdbool.h>
#include <stdio.h>
#include <terminal.h>
#include <time.h>

//! 内核重入计数，为当前内核需要处理中断的数量
int k_reenter;

//! 执行用户进程的入口
void restart();

#pragma GCC push_options
#pragma GCC optimize("O0")
static void busy_delay() {
  for (int j = 0; j < 5e7; ++j) {
  }
}
#pragma GCC pop_options

void TestA() {
  int i = 0;
  while (true) {
    kprintf("A%d.", i++);
    busy_delay();
  }
}

void TestB() {
  int i = 0;
  while (true) {
    kprintf("B%d.", i++);
    busy_delay();
  }
}

void TestC() {
  int i = 0;
  while (true) {
    kprintf("C%d.", i++);
    busy_delay();
  }
}

const char *color_print_str[] = {"\x1b[031m%c %d %d;", "\x1b[032m%c %d %d;",
                                 "\x1b[034m%c %d %d;"};

/*!
 * fmt: format string for kprintf, you'd better use the strings provided above
 * in the string array id: just like the TestA/B/C functions above, you should
 * designate a character to mark different processes my_pcb: the PCB(process
 * control block) of this process
 */
void TestABC(const char *fmt, char id, process_t *my_pcb) {
  todo("ensures that the arguments are passed correctly from caller");
  int i = 0;
  while (true) {
    kprintf(fmt, id, i++, my_pcb->pid);
    busy_delay();
  }
}

//! assign 4K stack space for each process
#define DEFAULT_STACK_SIZE 0x1000
#define STACK_TOTAL_SIZE (DEFAULT_STACK_SIZE * NR_PCBS)
u8 proc_stack_space[STACK_TOTAL_SIZE];

process_t *p_proc_ready;
process_t pcb_table[NR_PCBS];
void(*entry[NR_PCBS]) = {
    TestA,
    TestB,
    TestC,
};

void kernel_main() {
  //! init user proc and transfer ctrl flow

  term_clear();

  process_t *proc = pcb_table;
  u8 *stack_ptr = proc_stack_space;

  const u32 cs_sel =
      (SELECTOR_FLAT_C & SA_MASK_RPL & SA_MASK_TI) | SA_TIL | RPL_USER;
  const u32 ds_sel =
      (SELECTOR_FLAT_RW & SA_MASK_RPL & SA_MASK_TI) | SA_TIL | RPL_USER;
  const u32 gs_sel = (SELECTOR_VIDEO & SA_MASK_RPL & SA_MASK_TI) | RPL_USER;

  for (int i = 0; i < NR_PCBS; ++i, ++proc) {
    proc->regs.cs = cs_sel;
    proc->regs.ds = ds_sel;
    proc->regs.es = ds_sel;
    proc->regs.fs = ds_sel;
    proc->regs.ss = ds_sel;
    proc->regs.gs = gs_sel;

    proc->regs.eip = (u32)entry[i];
    stack_ptr += DEFAULT_STACK_SIZE;
    proc->regs.esp = (u32)stack_ptr;
    proc->pid = i + 1;
    proc->regs.eflags = EFLAGS_RESERVED | EFLAGS_IF | EFLAGS_IOPL(1);
  }

  p_proc_ready = pcb_table;

  init_sysclk();
  init_keyboard();

  restart();
  unreachable();
}
