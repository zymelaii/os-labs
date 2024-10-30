#include <process.h>
#include <stddef.h>

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

int get_pid(process_t *proc) {
  if (proc == NULL) {
    proc = p_proc_ready;
  }
  return proc->pcb.pid;
}
