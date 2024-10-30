#include <interrupt.h>
#include <process.h>
#include <protect.h>
#include <schedule.h>

void schedule(void) {
  disable_int_begin();

  process_t *cur_proc = p_proc_ready;
  process_t *next_proc = &pcb_table[(cur_proc - pcb_table + 1) % NR_PCBS];

  p_proc_ready = next_proc;

  // 切换进程页表和tss
  lcr3(p_proc_ready->pcb.cr3);
  tss.esp0 = (u32)(&p_proc_ready->pcb.user_regs + 1);

  // 调度函数最精髓的部分，非常的有意思。
  // 对于目前的os来说，eax,ebx,ecx,edx,esi,edi,esp,ebp,eflags这九个寄存器就可以表达当前执行流的上下文了，
  // 所以这个函数干了个什么事呢，它先将当前的esp保存到kern_regs中保存当前的栈信息，
  // 之后将esp移到当前kern_context栈顶然后按顺序依次将eax,ecx,edx,ebx,ebp,esi,edi,eflags压栈，
  // 这样就把当前进程寄存器上下文都保存下来了，再之后esp移到目标kern_context栈底，
  // 再依次恢复eflags,edi,esi,ebp,ebx,edx,ecx,eax，
  // 最后一步恢复esp，这个esp本身被push时保留的是进程栈中的esp，这样就完成了执行流的切换。
  // 从当前进程的角度上这就很有意思，相当于执行流进入了一个函数之后突然就被送往其他次元了，而且当前次元的时间静止了。
  // 然后再之后突然又回来了，还是之前的那些寄存器信息，感觉这个函数啥都没干。
  // 由于涉及到一大堆跟执行流切换相关的操作，所以一定！一定！一定！要关中断。
  switch_kernel_context(&cur_proc->pcb.kernel_regs,
                        &next_proc->pcb.kernel_regs);

  // 在传送回之后需要做的第一件事就是忘掉所有的全局变量的缓存
  // 防止编译器优化导致某些变量被优化掉，比如说某个全局变量a
  // 你拿eax去获取a的值，在发生调度之后a的值发生变化，下面用到了a
  // 然后编译器想都不想就直接用eax接着用了，一个bug就这么来了

  // 这玩意学术名词叫内存屏障，这还只是单核，如果多核就更加恐怖了，
  // 你会看到内存屏障遍地插，这也是为什么os难的一部分原因，跟缓存斗，跟硬件斗，跟编译器斗，跟并发斗，其乐无穷。
  // 要敬佩那些写系统软件的人，斗都要斗麻了的。
  barrier();

  // 这里可能你会比较疑惑为什么还要开中断，
  // 这个时候你需要要以单线程的思路去想，
  // 上面的switch_kern_context在执行后执行流就交给了其他进程了，
  // 但是其他进程总是会通过调度调度回来，对于当前这个进程来说，
  // 它相当于突然被送到其他次元了，包还掉在地上（关中断），而且那个次元的时间就静止了（The
  // World！） 当在其他次元兜兜转转回来了之后，还得把包捡回来（开中断）。
  // 不用去关心其他次元是怎么干的，但是对于这个次元来说还是得捡包的。
  // 如果每个次元都遵守这个规则，就可以保证执行不会出问题。
  disable_int_end();
}
