#include <assert.h>
#include <fork.h>
#include <process.h>

pid_t fork(process_t *father) {
  //! NOTE: set father to current process if father is null
  father = father ? father : p_proc_ready;

  /*!
   * 这应该是你第一次实现一个比较完整的功能
   * 你可能会比较急，但是你先别急，先别想自己要实现一个这么大的东西而毫无思路
   * 这样你在焦虑的同时也在浪费时间，就跟你在实验五中被页表折磨一样
   * 人在触碰到未知的时候总是害怕的，这是天性，所以请你先冷静下来
   * fork 的系统调用会一步步引导你写出来，不会让你本科造火箭的
   *
   * 推荐是边写边想，而不是想一箩筐再 ONE SHOT，这样非常容易计划赶不上变化
   */
  todo("NO PANIC OR SCARED, JUST CALM DOWN AND THINK");

  /*!
   * 1. 找一个空闲的进程作为你要 fork 为的子进程
   */
  todo("find an idle process");

  /*!
   * 2. 清晰地理解 pcb_t 中个成员的语义，明确各个成员在何处用到，如何使用
   *
   * exec 和 exit 的代码能够帮助你对 pcb 的理解，不先理解好 pcb 你 fork
   * 是无从下手的
   */
  panic("make insight of pcb");

  /*!
   * 3. 初始化子进程 pcb
   *
   * 在阅读完 pcb 之后终于可以开始 fork 工作了，本质上相当于将父进程的 pcb
   * 内容复制到子进程 pcb 中，但是你需要想清楚，哪些应该复制到子进程，哪些不
   * 应该复制，哪些应该子进程自己初始化
   *
   * **难点**
   *
   * - 子进程 fork 的返回值怎么处理？
   *   这需要你对系统调用整个过程都掌握比较清楚，如果非常清晰这个问题不会很大
   * - 子进程内存如何复制？
   *   别傻乎乎地复制父进程的 cr3，本质上相当于与父进程共享同一块内存，而共享
   *   内存肯定不符合 fork 的语义，这样一个进程写内存某块地方会影响到另一个进
   *   程，这个东西需要你自己思考如何复制父进程的内存
   * - 在 fork 结束后，最终总会调度到子进程，怎么保证子进程能够正常进入用户态？
   *   你肯定会觉得这个问题问的莫名其妙的，只能说你如果遇到那一块问题了就会体会
   *   到这个问题的重要性，这需要你对调度整个过程都掌握比较清楚
   */
  panic("copy and re-init pcb");

  /*!
   * 4. 维护进程树，添加该父子进程关系
   */
  todo("maintain process tree");

  /*!
   * 5. 将子进程的状态置为 READY，说明 fork 已经完成，子进程准备就绪
   */
  todo("mark as ready");

  /*!
   * 6. 别急着跑，先自我灵魂拷问一下
   *
   * - 上锁上了吗？所有临界情况都考虑到了吗？
   *   永远要相信有各种奇奇怪怪的并发问题
   * - 所有错误情况都判断到了吗？错误情况怎么处理？
   *   RTFM -> man 2 fork
   * - 你写的代码真的符合 fork 语义吗？
   */
  panic("SOUL TORTURE");

  return 0;
}
