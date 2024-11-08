#include <arch/x86.h>
#include <assert.h>
#include <elf.h>
#include <layout.h>
#include <malloc.h>
#include <page.h>
#include <stdio.h>
#include <string.h>

static phyaddr_t alloc_phy_page(page_node_t **page_list) {
  phyaddr_t phy_addr = phy_malloc_4k();
  page_node_t *page_node = kmalloc(sizeof(page_node_t));
  page_node->next = *page_list;
  page_node->phy_addr = phy_addr;
  page_node->addr = 0xffffffff;
  *page_list = page_node;
  return phy_addr;
}

void map_laddr(u32 cr3, page_node_t **page_list, uintptr_t addr,
               phyaddr_t phy_addr, u32 pde_attr, u32 pte_attr) {
  assert(PGOFF(addr) == 0);

  uintptr_t *pde_ptr = (uintptr_t *)K_PHY2LIN(cr3);

  if ((pde_ptr[PDX(addr)] & PTE_P) == 0) {
    phyaddr_t pte_phy = alloc_phy_page(page_list);
    memset((void *)K_PHY2LIN(pte_phy), 0, PGSIZE);
    pde_ptr[PDX(addr)] = pte_phy | pde_attr;
  }

  phyaddr_t pte_phy = PTE_ADDR(pde_ptr[PDX(addr)]);
  uintptr_t *pte_ptr = (uintptr_t *)K_PHY2LIN(pte_phy);

  phyaddr_t page_phy;
  if (phy_addr == 0xffffffff) {
    if ((pte_ptr[PTX(addr)] & PTE_P) != 0) {
      return;
    }
    page_phy = alloc_phy_page(page_list);
    (*page_list)->addr = addr;
  } else {
    if ((pte_ptr[PTX(addr)] & PTE_P) != 0) {
      warn("page is already mapped to 0x%x", addr);
    }
    assert(PGOFF(phy_addr) == 0);
    page_phy = phy_addr;
  }

  pte_ptr[PTX(addr)] = page_phy | pte_attr;
}

void map_kernel(u32 cr3, page_node_t **page_list) {
  const u32 pde_attr = PTE_P | PTE_W | PTE_U;
  const u32 pte_attr = PTE_P | PTE_W | PTE_U;
  for (phyaddr_t addr = 0; addr < 128 * NUM_1M; addr += PGSIZE) {
    const uintptr_t laddr = (uintptr_t)K_PHY2LIN(addr);
    map_laddr(cr3, page_list, laddr, addr, pde_attr, pte_attr);
  }
}

void map_elf(pcb_t *proc, void *elf_addr) {
  assert(proc->lock != 0);
  elf32_hdr_t *eh = (elf32_hdr_t *)elf_addr;
  elf32_phdr_t *ph = (elf32_phdr_t *)(elf_addr + eh->e_phoff);
  for (int i = 0; i < eh->e_phnum; i++, ph++) {
    if (ph->p_type != PT_LOAD) {
      continue;
    }
    uintptr_t st = ROUNDDOWN(ph->p_vaddr, PGSIZE);
    uintptr_t en = ROUNDUP(st + ph->p_memsz, PGSIZE);
    for (uintptr_t laddr = st; laddr < en; laddr += PGSIZE) {
      u32 pte_flag = PTE_P | PTE_U;
      if ((ph->p_flags & ELF_PROG_FLAG_WRITE) != 0) {
        pte_flag |= PTE_W;
      }
      map_laddr(proc->cr3, &proc->pages, laddr, 0xffffffff,
                PTE_P | PTE_W | PTE_U, pte_flag);
    }
    memcpy((void *)ph->p_vaddr, (const void *)eh + ph->p_offset, ph->p_filesz);
    memset((void *)ph->p_vaddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
  }
  proc->user_regs.eip = eh->e_entry;
}

void map_stack(pcb_t *proc) {
  assert(proc->lock != 0);
  map_laddr(proc->cr3, &proc->pages, (uintptr_t)K_PHY2LIN(-PGSIZE), 0xffffffff,
            PTE_P | PTE_W | PTE_U, PTE_P | PTE_W | PTE_U);
  proc->user_regs.esp = (u32)K_PHY2LIN(0);
}

void recycle_pages(page_node_t *pages) {
  page_node_t *page = pages;
  while (page != NULL) {
    page_node_t *next_page = page->next;
    phy_free_4k(page->phy_addr);
    kfree(page);
    page = next_page;
  }
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
