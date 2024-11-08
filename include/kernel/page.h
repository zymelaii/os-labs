#pragma once

#include <process.h>
#include <stddef.h>  // IWYU pragma: keep
#include <sys/types.h>

/*!
 * A linear address 'la' has a three-part structure as follows:
 *
 * +--------10------+-------10-------+---------12----------+
 * | Page Directory |   Page Table   | Offset within Page  |
 * |      Index     |      Index     |                     |
 * +----------------+----------------+---------------------+
 *  \--- PDX(la) --/ \--- PTX(la) --/ \---- PGOFF(la) ----/
 *  \---------- PGNUM(la) ----------/
 *
 * The PDX, PTX, PGOFF, and PGNUM macros decompose linear addresses as shown.
 * To construct a linear address la from PDX(la), PTX(la), and PGOFF(la),
 * use PGADDR(PDX(la), PTX(la), PGOFF(la)).
 */

#define NPDENTRIES 1024  //<! page directory entries per page directory
#define NPTENTRIES 1024  //<! page table entries per page table

//! bytes mapped by a page
#define PGSIZE 4096
#define PGSHIFT 12

//! bytes mapped by a page directory entry
#define PTSIZE (PGSIZE * NPTENTRIES)
#define PTSHIFT 22

#define PTXSHIFT 12  //<! offset of PTX in a linear address
#define PDXSHIFT 22  //<! offset of PDX in a linear address

// pte/pde attribute flags
#define PTE_P 0x001    //<! present
#define PTE_W 0x002    //<! writeable
#define PTE_U 0x004    //<! user
#define PTE_PWT 0x008  //<! write-through
#define PTE_PCD 0x010  //<! cache-disable
#define PTE_A 0x020    //<! accessed
#define PTE_D 0x040    //<! dirty
#define PTE_PS 0x080   //<! page size
#define PTE_G 0x100    //<! global

//! page number field of address
#define PGNUM(la) (((uintptr_t)(la)) >> PTXSHIFT)

//! page directory index
#define PDX(la) ((((uintptr_t)(la)) >> PDXSHIFT) & 0x3ff)

//! page table index
#define PTX(la) ((((uintptr_t)(la)) >> PTXSHIFT) & 0x3ff)

//! offset in page
#define PGOFF(la) (((uintptr_t)(la)) & 0xfff)

//! construct linear address from indexes and offset
#define PGADDR(d, t, o) ((void *)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))

//! address in page table or page directory entry
#define PTE_ADDR(pte) ((phyaddr_t)(pte) & ~0xfff)

//! page fault error codes
#define FEC_PR 0x1  //<! page fault caused by protection violation
#define FEC_WR 0x2  //<! page fault caused by a write
#define FEC_U 0x4   //<! page fault occured while in user mode

void map_laddr(u32 cr3, page_node_t **page_list, uintptr_t addr,
               phyaddr_t phy_addr, u32 pde_attr, u32 pte_attr);

/*!
 * \brief map lin-addr 3G~3G+128M to phy-addr 0~128M for kernel space
 */
void map_kernel(phyaddr_t cr3, page_node_t **page_list);

/*!
 * \brief load and map elf file into user space for the specified process
 */
void map_elf(pcb_t *proc, void *elf_addr);

/*!
 * \brief map stack 0xbffff000~0xc0000000 for a process
 */
void map_stack(pcb_t *proc);

/*!
 * \brief recycle pages of a process
 */
void recycle_pages(page_node_t *page_list);
