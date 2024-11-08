#include <assert.h>
#include <layout.h>
#include <wait.h>

int wait(int *wstatus) {
  //! generally, wait is only allowed to be called from a user program
  assert((void *)wstatus + sizeof(wstatus) < K_PHY2LIN(0));

  /*!
   * 相比于 fork 来说，wait 的实现简单很多
   * 语义实现比较清晰，没有 fork 那么多难点要处理，所以这里并不会给大家太多引导
   * 需要大家自己思考 wait 怎么实现
   * 在实现之前你必须得读一遍文档 man 2 wait 了解到 wait 大概要做什么
   */
  todo("read the mannual");

  /*!
   * 当然读文档是一方面，最重要的还是代码实现
   * wait 系统调用与 exit 系统调用关系密切，所以在实现 wait 之前需要先读一遍
   * exit 为好，可能读完 exit 的代码你可能知道 wait 该具体做什么了
   */
  todo("read the impl of exit");

  /*!
   * 接下来就是你自己的实现了，我们在设计的时候这段代码不会有太大问题
   *
   * 在实现完后你任然要对自己来个灵魂拷问
   * - 上锁上了吗？所有临界情况都考虑到了吗？
   *   永远要相信有各种奇奇怪怪的并发问题
   * - 所有错误情况都判断到了吗？错误情况怎么处理？
   *   RTFM -> man 2 wait
   * - 是否所有的资源都正确回收了？
   * - 你写的代码真的符合wait语义吗？
   */
  todo("impl wait");
  todo("SOUL TORTURE");

  return 0;
}
