#include <arch/x86.h>
#include <assert.h>
#include <exit.h>
#include <interrupt.h>
#include <process.h>
#include <schedule.h>
#include <stdbool.h>

static void awake_father_and_become_zombie(pcb_t *proc) {
  /*!
   * 由于我们已经协商了上锁的顺序，上锁顺序是从父亲到儿子
   * 但是这里我们必须锁了自己才能知道父进程是谁
   * 所以这里我们换了一种做法，不再一个劲地等锁
   * 如果父进程上锁失败，就直接将自己的锁释放拱手让人
   * 这样做有个好处，要么两个锁同时被上掉，要么两个锁同时被释放
   * 这也是一个非常有趣的实现方法，而真实情况是将大锁拆小锁，不
   * 可能一个 pcb 就一个大锁保着，这样又浪费效率又难写
   */

  pcb_t *fa = NULL;

  while (true) {
    bool done = false;
    bool should_revert = false;
    do {
      if (xchg(&proc->lock, 1) == 0) {
        fa = proc->ptree.fa;
      } else {
        should_revert = true;
        break;
      }
      if (xchg(&fa->lock, 1) == 0) {
        done = true;
      }
    } while (0);
    if (done) {
      break;
    }
    if (should_revert) {
      xchg(&proc->lock, 0);
    }
    schedule();
  }

  assert(proc->status == PSTATUS_READY);
  assert(fa->status == PSTATUS_READY || fa->status == PSTATUS_SLEEPING);
  proc->status = PSTATUS_ZOMBIE;
  fa->status = PSTATUS_READY;

  xchg(&fa->lock, 0);
  xchg(&proc->lock, 0);
}

static void transfer_orphans(pcb_t *proc) {
  pcb_t *idle_proc = &pcb_table[0].pcb;

  //! 上锁顺序：0 号进程 -> 当前进程 -> 子进程
  while (xchg(&idle_proc->lock, 1) == 1) {
    schedule();
  }
  while (xchg(&proc->lock, 1) == 1) {
    schedule();
  }

  son_node_t *node = proc->ptree.sons;
  while (node) {
    son_node_t *next = node->next;
    pcb_t *son = node->son;

    /*!
     * 上子进程的锁
     * 需要修改子进程的父进程信息，转移到初始进程下
     */
    while (xchg(&son->lock, 1) == 1) {
      schedule();
    }

    son->ptree.fa = idle_proc;

    //! NOTE: ensures the new proc node is always inserted to the head
    assert(node->prev == NULL);
    if (node->next != NULL) {
      node->next->prev = NULL;
    }
    node->next = idle_proc->ptree.sons;
    if (node->next != NULL) {
      node->next->prev = node;
    }
    idle_proc->ptree.sons = node;

    xchg(&son->lock, 0);

    node = next;
  }

  /*!
   * 在移交完后当前进程的子进程信息会被清空
   */
  proc->ptree.sons = NULL;

  /*!
   * 初始进程可能在休眠，而且子进程可能是僵尸进程，需要将初始进程唤醒
   * 初始进程会一直调用 wait 系统调用回收僵尸子进程
   */
  assert(idle_proc->status == PSTATUS_READY ||
         idle_proc->status == PSTATUS_SLEEPING);
  idle_proc->status = PSTATUS_READY;

  xchg(&proc->lock, 0);
  xchg(&idle_proc->lock, 0);
}

void exit(process_t *proc, int exit_code) {
  if (proc == NULL) {
    proc = p_proc_ready;
  }
  pcb_t *pcb = &proc->pcb;

  /*!
   * 托孤，将所有子进程转移到初始进程下
   */
  transfer_orphans(pcb);

  /*!
   * 上锁修改 exit code
   */
  while (xchg(&pcb->lock, 1) == 1) {
    schedule();
  }
  pcb->exit_code = exit_code;
  xchg(&pcb->lock, 0);

  /*!
   * 下面两个操作会修改进程的状态，这是非常危险的，最好用开关中断保护上
   */
  disable_int_begin();
  /*!
   * 这个函数干了两件事，唤醒父进程，将自己状态置为僵尸进程，关中断就相当于两件事同时干了
   */
  awake_father_and_become_zombie(pcb);
  /*!
   * 在触发了调度之后这个进程在被回收之前永远无法被调度到
   */
  schedule();
  disable_int_end();

  unreachable();
}
