#include <arch/x86.h>
#include <assert.h>
#include <exec.h>
#include <fat32.h>
#include <fs.h>
#include <keyboard.h>
#include <layout.h>
#include <malloc.h>
#include <page.h>
#include <process.h>
#include <protect.h>
#include <stdbool.h>
#include <terminal.h>
#include <time.h>

void kernel_main(int partition_lba) {
  init_fs(partition_lba);
  term_clear();

  process_t *idle_proc = &pcb_table[0];
  pcb_t *pcb = &idle_proc->pcb;
  u32 *frame = &idle_proc->kernel_stack_bottom[-2];
  frame[0] = (u32)restart;
  frame[1] = (u32)pcb;
  pcb->kernel_regs.esp = (u32)frame;
  pcb->priority = 1;
  pcb->ticks = pcb->priority;
  pcb->ptree.fa = NULL;
  pcb->ptree.sons = NULL;
  pcb->status = PSTATUS_READY;

  int result = exec(idle_proc, "idle.bin");
  if (result != 0) {
    panic("failed to start idle proc");
  }

  p_proc_ready = idle_proc;

  lcr3(p_proc_ready->pcb.cr3);
  tss.esp0 = (u32)(&p_proc_ready->pcb.user_regs + 1);

  init_sysclk();
  init_keyboard();

  kernel_context_t dummy;  //<! only for temporary use
  switch_kernel_context(&dummy, &p_proc_ready->pcb.kernel_regs);

  unreachable();
}
