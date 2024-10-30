#include <arch/x86.h>
#include <assert.h>
#include <layout.h>
#include <page.h>
#include <stdio.h>

/*!
 * \brief allocate a physical page of 4KB size
 * \note 64~128MB is reserved for page allocation
 */
phyaddr_t phy_malloc_4k() {
  static phyaddr_t FREE_MEM_PTR = 64 * NUM_1M;

  assert(FREE_MEM_PTR < 128 * NUM_1M);
  const phyaddr_t addr = FREE_MEM_PTR;
  FREE_MEM_PTR += PGSIZE;

  return addr;
}

/*!
 * \brief map lin-addr 3G~3G+128M to phy-addr 0~128M for kernel space
 */
void map_kernel(phyaddr_t cr3) {
  //! 初始化页目录表 pde
  //! 由于 cr3 是物理地址，需要进行一步转化转化到线性地址才能访存
  uintptr_t *pde_ptr = (uintptr_t *)K_PHY2LIN(cr3);
  pde_ptr += PDX(KERNEL_LIN_BASE);
  //! 一个页目录项映射 4MB 的内存，计算需要初始化的页目录项数目
  int pde_num = (128 * NUM_1M) / (4 * NUM_1M);
  //! 被映射的物理地址
  phyaddr_t phy_addr = 0;
  while (pde_num--) {
    //! 对于每一个页目录项申请一页用于页表
    phyaddr_t pte_phy = phy_malloc_4k();
    //! 保证申请出来的物理地址是 4K 对齐的
    assert(PGOFF(pte_phy) == 0);
    //! 初始化页目录项，权限是 P+W+U
    *pde_ptr++ = pte_phy | PTE_P | PTE_W | PTE_U;
    //! 接下来对新申请的页表初始化页表项
    //! 由于申请的 pte_phy 是物理地址，需要进行一步转化转化到线性地址才能访存
    uintptr_t *pte_ptr = (uintptr_t *)K_PHY2LIN(pte_phy);
    //! 初始化页表的所有页表项
    int pte_num = NPTENTRIES;
    while (pte_num--) {
      //! 初始化页表项，权限是 P+W
      //! 直接线性映射物理页
      *pte_ptr++ = phy_addr | PTE_P | PTE_W;
      //! 换下一个物理页
      phy_addr += PGSIZE;
    }
  }
  assert(phy_addr == 128 * NUM_1M);
}

void page_fault_handler(int vec_no, int err_code, int eip, int cs, int eflags) {
  static char fmtstr[256];
  char *p_str = fmtstr, *p_end = fmtstr + sizeof(fmtstr) - 1;
  p_str += snprintf(p_str, p_end - p_str, "\nOh, you receive a page fault!\n");
  p_str +=
      snprintf(p_str, p_end - p_str,
               "CS: 0x%%x EIP: 0x%%x You tried to access the address: 0x%%x\n");
  if ((err_code & FEC_PR) == 0) {
    p_str += snprintf(p_str, p_end - p_str,
                      "You tried to access a nonexistent page!\n");
  }
  if ((err_code & FEC_WR) != 0) {
    p_str +=
        snprintf(p_str, p_end - p_str, "You tried to write in this page!\n");
  } else {
    p_str +=
        snprintf(p_str, p_end - p_str, "You tried to read in this page!\n");
  }
  if ((err_code & FEC_U) != 0) {
    p_str += snprintf(p_str, p_end - p_str,
                      "You tried to access a page in user mode!\n");
  }
  *p_str = '\0';
  panic(fmtstr, cs, eip, rcr2());
}
