#include <arch/x86.h>
#include <layout.h>
#include <page.h>
#include <stdbool.h>
#include <stdio.h>

void pmap_test() {
  phyaddr_t old_cr3 = rcr3();

  /*!
   * 为了做测试，申请了一个新的物理页用于页表的根目录
   */
  phyaddr_t new_cr3 = phy_malloc_4k();
  kprintf("cr3 physic addr: 0x%x\n", new_cr3);

  /*!
   * 可以打印一下函数地址，看看它们线性地址实际的位置，你会
   * 发现与 lab4 的虚拟地址分布不太一样
   */
  kprintf("map_kernel: 0x%x, lcr3: 0x%x\n", map_kernel, lcr3);

  /*!
   * 这里映射内核区域，这样在加载完之后依然能够正常运行代码，
   * 否则加载完之后执行指令直接爆 page fault
   * 如果不怕死可以直接注释这一行，你能够欣赏到精彩的 PF
   */
  map_kernel(new_cr3);
  lcr3(new_cr3);

  /*!
   * 在没有页表映射之前，任何对地址的读写操作都会爆 PF，
   * 所以我们需要自己映射来实现地址的访存
   * 我们就假定往 0x11451400 地址处写入相关值，接下来要对
   * 该地址处做相关地址映射
   */
  uintptr_t lin1_addr = 0x11451400;

  /*!
   * 首先填充 pde 页表项，之前申请的 new_cr3 就是 pde 对
   * 应的页表，我们需要填充 pde 对应的页表项
   * K_PHY2LIN 能够帮助我们将物理地址转化为能够访问得到该
   * 物理地址的线性地址
   */
  phyaddr_t pde_phy = new_cr3;
  uintptr_t *pde_ptr = (uintptr_t *)K_PHY2LIN(pde_phy);
  phyaddr_t pte_phy = phy_malloc_4k();
  pde_ptr[PDX(lin1_addr)] = pte_phy | PTE_P | PTE_W | PTE_U;

  /*!
   * 我们可以打印一下 PDX(lin1_addr) 和对应的 pde 页表项，
   * 这个物理页就是 pte 对应的页表，被描述在 pde 对应的页表
   * 项中
   */
  kprintf("PDX(lin1_addr) = 0x%x, pde = 0x%x\n", PDX(lin1_addr),
          pde_ptr[PDX(lin1_addr)]);

  /*!
   * 接下来要填充 pte 页表项，操作与刚才差不多，基本上同样的
   * 操作再来一遍
   */
  uintptr_t *pte_ptr = (uintptr_t *)K_PHY2LIN(pte_phy);
  phyaddr_t pg_phy = phy_malloc_4k();
  pte_ptr[PTX(lin1_addr)] = pg_phy | PTE_P | PTE_W | PTE_U;
  kprintf("PTX(lin1_addr) = 0x%x, pte = 0x%x\n\n", PTX(lin1_addr),
          pte_ptr[PTX(lin1_addr)]);

  /*!
   * 你可能会怀疑，刚才打印的 PDX(lin1_addr) 和 PTX(lin1_addr)
   * 是个什么勾八玩意，不过看完这一行的输出，你或许就会有所感悟
   */
  kprintf("PDX(lin1_addr) << 22 | PTX(lin1_addr) << 12 = 0x%x\n",
          PDX(lin1_addr) << PDXSHIFT | PTX(lin1_addr) << PTXSHIFT);

  /*!
   * 接下来你就能愉悦对 0x11451400进行读写了
   */
  *(int *)lin1_addr = 1919810;

  /*!
   * 不过我们重新打印一下 pde 和 pte……哈？怎么有点不太一样？是怎么回事？
   * 看起来硬件都访问到了 lin1_addr 对应的 pde 和 pte，通过这两项找到真
   * 正的物理地址，然后对该地址做了修改。但是这个改了值的含义是什么？这个
   * 的原因需要你自己 RTFM，这里并不做相关解答。
   */
  kprintf("pde = 0x%x, pte = 0x%x\n\n", pde_ptr[PDX(lin1_addr)],
          pte_ptr[PTX(lin1_addr)]);

  /*!
   * 在读写之后我们，当然要查看是不是写进去了
   * 但是除了原来的 0x11451400，我们可以用另一个线性地址访问到了同样的数据
   */
  uintptr_t lin2_addr = (uintptr_t)K_PHY2LIN(pg_phy + PGOFF(lin1_addr));
  kprintf(
      "lin1_addr:0x%x -> %d\n"
      "lin2_addr:0x%x -> %d\n\n",
      lin1_addr, *(int *)lin1_addr, lin2_addr, *(int *)lin2_addr);

  /*!
   * 还是有点摸不清硬件是怎么访问页表的？下面这行打印可能能够帮到你
   */
  kprintf(
      "*(phyaddr_t)(0x%x + 0x%x) => *(uintptr_t)(0x%x + 0x%x) = 0x%x\n"
      "*(phyaddr_t)(0x%x + 0x%x) => *(uintptr_t)(0x%x + 0x%x) = 0x%x\n\n",
      pde_phy, PDX(lin1_addr), pde_ptr, PDX(lin1_addr), pde_ptr[PDX(lin1_addr)],
      pte_phy, PTX(lin1_addr), pte_ptr, PTX(lin1_addr),
      pte_ptr[PTX(lin1_addr)]);

  /*!
   * 当然，我们可以申请一个新的物理页替换掉原来的 pte 对应的页表
   */
  phyaddr_t new_pg_phy = phy_malloc_4k();
  pte_ptr[PTX(lin1_addr)] = new_pg_phy | PTE_P | PTE_W | PTE_U;
  // 同样往相同的地址写入相关数据
  *(int *)lin1_addr = 114514;
  uintptr_t lin3_addr = (uintptr_t)K_PHY2LIN(new_pg_phy + PGOFF(lin1_addr));

  /*!
   * 我们打印一下，结果肯定是 lin1_addr 和 lin3_addr 对应的值相同……吗？
   * 怎么回事？
   * 怎么 lin2_addr 对应的跟 lin1_addr 对应的值相同，lin3_addr 对应的
   * 值一点都没改?
   */
  kprintf(
      "lin1_addr:0x%x -> %d\n"
      "lin2_addr:0x%x -> %d\n"
      "lin3_addr:0x%x -> %d\n\n",
      lin1_addr, *(int *)lin1_addr, lin2_addr, *(int *)lin2_addr, lin3_addr,
      *(int *)lin3_addr);

  /*!
   * 是不是 qemu 出问题了？是该怀疑 qemu，还是自己之前的操作失误了？
   * 但是查了一遍好像没啥问题，照理往 lin1_addr 写入数据 lin3_addr 能够马上看到
   * 这个时候出问题的实际上是 tlb，在你修改已经在 tlb 中的页表项时，tlb 中的缓存
   * 并不会直接刷新
   * 所以需要刷新一下 tlb 缓存，让硬件重新解析页表
   */
  tlbflush();

  /*!
   * 在刷新之后你会发现修改正常了
   */
  *(int *)lin1_addr = 1919810;
  kprintf(
      "lin1_addr:0x%x -> %d\n"
      "lin2_addr:0x%x -> %d\n"
      "lin3_addr:0x%x -> %d\n\n",
      lin1_addr, *(int *)lin1_addr, lin2_addr, *(int *)lin2_addr, lin3_addr,
      *(int *)lin3_addr);
  lcr3(old_cr3);

  while (true) {
  }
}
