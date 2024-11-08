#include <arch/x86.h>
#include <interrupt.h>
#include <layout.h>
#include <malloc.h>
#include <page.h>
#include <process.h>
#include <protect.h>
#include <schedule.h>
#include <stddef.h>
#include <string.h>

process_t *p_proc_ready;
process_t pcb_table[NR_PCBS];

/*!
 * 这个函数是写给汇编函数用，在汇编函数调用 save 函数保存完用户寄存器上下文
 * 后，需要切到进程内核栈中，本实验中的进程栈是与 pcb 是绑定的。相当于每个进
 * 程分配了一个大空间，前面一小部分给了pcb，后面的全可以给进程栈使用，目前进
 * 程占 8KB，进程栈占 7KB 多。
 *
 * 这个函数的目的就是切换到进程栈处，由于每个进程的空间只有 C 语言这知道，所
 * 以直接在 C 语言里面写一个内联汇编实现，这样更方便动态开内核栈空间不用顾忌
 * 到汇编。
 */
void to_kernel_stack(u32 ret_esp) {
  asm volatile(
      "add %1, %0\n\t"
      "mov %0, %%esp\n\t"
      "push %2\n\t"
      "jmp *4(%%ebp)\n\t"
      :
      : "a"((u32)p_proc_ready->padding),
        "b"((u32)sizeof(p_proc_ready->padding)), "c"(ret_esp));
}

pid_t get_pid(process_t *proc) {
  if (proc == NULL) {
    proc = p_proc_ready;
  }
  while (xchg(&proc->pcb.lock, 1) == 1) {
    schedule();
  }
  const int pid = proc->pcb.pid;
  xchg(&proc->pcb.lock, 0);
  return pid;
}

void init_segment_regs(pcb_t *pcb) {
  const u32 index_mask = SA_MASK_RPL & SA_MASK_TI;
  const u32 cs_sel = (SELECTOR_FLAT_C & index_mask) | SA_TIL | RPL_USER;
  const u32 ds_sel = (SELECTOR_FLAT_RW & index_mask) | SA_TIL | RPL_USER;
  const u32 gs_sel = (SELECTOR_VIDEO & index_mask) | RPL_USER;

  pcb->user_regs.cs = cs_sel;
  pcb->user_regs.ds = ds_sel;
  pcb->user_regs.es = ds_sel;
  pcb->user_regs.fs = ds_sel;
  pcb->user_regs.ss = ds_sel;
  pcb->user_regs.gs = gs_sel;
}

void init_pagetbl(pcb_t *pcb) {
  phyaddr_t new_cr3 = phy_malloc_4k();
  memset((void *)K_PHY2LIN(new_cr3), 0, PGSIZE);

  page_node_t *new_page_list = kmalloc(sizeof(page_node_t));
  new_page_list->next = NULL;
  new_page_list->phy_addr = new_cr3;
  new_page_list->addr = 0xffffffff;
  map_kernel(new_cr3, &new_page_list);

  /*!
   * 这里需要特别注意的是，替换用户页表这种危险行为无论如何都是要
   * 关中断的，不允许中间有任何调度否则很有可能换到一半，啪，一个
   * 中断进来调度了
   * 调度回来时要加载 cr3，然后惊喜地发现 #PF 了
   */

  page_node_t *old_page_list = pcb->pages;

  disable_int_begin();
  pcb->cr3 = new_cr3;
  pcb->pages = new_page_list;
  lcr3(pcb->cr3);
  disable_int_end();

  recycle_pages(old_page_list);
}
