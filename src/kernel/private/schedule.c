#include <arch/x86.h>
#include <interrupt.h>
#include <process.h>
#include <protect.h>
#include <schedule.h>
#include <stdbool.h>

void schedule() {
  static bool in_sche = false;

  disable_int_begin();

  process_t *this_proc = p_proc_ready;
  process_t *next_proc = p_proc_ready;

  barrier();

  do {
    if (in_sche) {
      break;
    }
    in_sche = true;

    do {
      next_proc = &pcb_table[(next_proc - pcb_table + 1) % NR_PCBS];
      //! no ready proc found, wait for next sys tick
      if (this_proc == next_proc && next_proc->pcb.status != PSTATUS_READY) {
        enable_int();
        halt();
        disable_int();
        barrier();
      }
    } while (next_proc->pcb.status != PSTATUS_READY);

    p_proc_ready = next_proc;
    lcr3(p_proc_ready->pcb.cr3);
    tss.esp0 = (u32)(&p_proc_ready->pcb.user_regs + 1);

    in_sche = false;

    switch_kernel_context(&this_proc->pcb.kernel_regs,
                          &next_proc->pcb.kernel_regs);

    barrier();
  } while (0);

  disable_int_end();
}
