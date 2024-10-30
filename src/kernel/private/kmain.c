#include <arch/x86.h>
#include <assert.h>
#include <fat32.h>
#include <fs.h>
#include <keyboard.h>
#include <layout.h>
#include <page.h>
#include <process.h>
#include <protect.h>
#include <stdbool.h>
#include <terminal.h>
#include <time.h>

static char exectuables[NR_PCBS][FAT32_SHORT_NAME_LEN + 1] = {
    "DELAY   BIN",
    "DELAY   BIN",
};

static int priority_table[NR_PCBS] = {1, 2};

void kernel_main(int partition_lba) {
  term_clear();

  init_fs(partition_lba);

  const u32 cs_sel =
      (SELECTOR_FLAT_C & SA_MASK_RPL & SA_MASK_TI) | SA_TIL | RPL_USER;
  const u32 ds_sel =
      (SELECTOR_FLAT_RW & SA_MASK_RPL & SA_MASK_TI) | SA_TIL | RPL_USER;
  const u32 gs_sel = (SELECTOR_VIDEO & SA_MASK_RPL & SA_MASK_TI) | RPL_USER;

  for (int i = 0; i < NR_PCBS; ++i) {
    process_t *const proc = &pcb_table[i];

    //! 初始化进程段寄存器
    proc->pcb.user_regs.cs = cs_sel;
    proc->pcb.user_regs.ds = ds_sel;
    proc->pcb.user_regs.es = ds_sel;
    proc->pcb.user_regs.fs = ds_sel;
    proc->pcb.user_regs.ss = ds_sel;
    proc->pcb.user_regs.gs = gs_sel;

    //! 为进程分配 cr3 物理内存并映射内核页表
    proc->pcb.cr3 = phy_malloc_4k();
    map_kernel(proc->pcb.cr3);

    /*!
     * 在 map_kernel 之后，就内核程序对应的页表已经被映射了，就可以直接
     * lcr3，与此同时执行流不会触发 page fault
     * 如果不先 map_kernel，执行流会发现执行的代码的线性地址不存在爆出
     * page fault
     * 当然选不选择看个人的想法，评价是都行，各有各的优缺点
     */

    //! 将待执行的可执行文件读入内存
    //! NOTE: 推荐将文件加载到 3GB+48MB 处，应用程序保证不会有 16MB 那么大
    read_file(exectuables[i], (void *)K_PHY2LIN(48 * NUM_1M));

    todo("parse the loaded elf and load it to vaddr");
    todo("allocate stack of process in user space");

    proc->pcb.user_regs.eflags = EFLAGS_RESERVED | EFLAGS_IF | EFLAGS_IOPL(1);

    /*!
     * 接下来初始化内核寄存器，需要初始化内核寄存器原因是加入了系统调用后，
     * 非常有可能出现系统调用执行过程中插入其余中断的情况，如果跟之前一样所
     * 有进程共享一个内核栈会发生不可想象的结果。
     * 为了避免这种情况，就需要给每个进程分配一个进程栈。
     * 当触发时钟中断发生调度的时候，不再是简单的切换 p_proc_ready，而是需
     * 要将内核栈进行切换，而且需要切换执行流到另一个进程的内核栈。所以需要
     * 一个地方存放当前进程的寄存器上下文。
     * 这是一个富有技巧性的活，深入研究会觉得很妙，如果需要深入了解，可以查
     * 看 schedule 函数了解切换细节。
     */

    u32 *frame = &proc->kernel_stack_bottom[-2];
    //! 保证切换内核栈后执行流进入的是 restart 函数
    frame[0] = (u32)restart;
    //! 这里是因为 restart 要用 pop esp 确认 esp 该往哪里跳
    frame[1] = (u32)&proc->pcb;

    proc->pcb.kernel_regs.esp = (u32)frame;

    proc->pcb.pid = i;
    proc->pcb.priority = priority_table[i];
    proc->pcb.ticks = proc->pcb.priority;
  }

  p_proc_ready = &pcb_table[0];

  //! 切换进程页表和 tss
  lcr3(p_proc_ready->pcb.cr3);
  tss.esp0 = (u32)(&p_proc_ready->pcb.user_regs + 1);

  init_sysclk();
  init_keyboard();

  kernel_context_t dummy;  //<! only for temporary use
  switch_kernel_context(&dummy, &p_proc_ready->pcb.kernel_regs);

  unreachable();
}
