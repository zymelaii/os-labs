#include <arch/x86.h>
#include <assert.h>
#include <layout.h>
#include <malloc.h>
#include <page.h>
#include <schedule.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct memblk_info {
  struct memblk_info *ptr;
  size_t size;  //<! in a unit of sizeof(memblk_info_t)
} memblk_info_t;

static u32 PHY_MALLOC_LOCK = 0;
static u32 KMALLOC_LOCK = 0;

static memblk_info_t *KMEM_FREE_SLOT;

/*!
 * \brief allocate a physical page of 4KB size
 * \note 96~128MB is reserved for page allocation
 */
phyaddr_t phy_malloc_4k() {
  static phyaddr_t FREE_MEM_PTR = 96 * NUM_1M;

  while (xchg(&PHY_MALLOC_LOCK, 1) == 1) {
    schedule();
  }

  assert(FREE_MEM_PTR < 128 * NUM_1M);
  const phyaddr_t addr = FREE_MEM_PTR;
  FREE_MEM_PTR += PGSIZE;

  xchg(&PHY_MALLOC_LOCK, 0);

  return addr;
}

void phy_free_4k(phyaddr_t phy_addr) {
  assert(phy_addr % PGSIZE == 0);
  unimplemented();
}

/*!
 * \brief allocate memory in kernel space
 * \note 64~96MB is reserved for page allocation
 */
void *kmalloc(size_t n) {
  void *ret_ptr = NULL;

  while (xchg(&KMALLOC_LOCK, 1) == 1) {
    schedule();
  }

  memblk_info_t *prev = KMEM_FREE_SLOT;
  if (prev == NULL) {
    KMEM_FREE_SLOT = (void *)K_PHY2LIN(64 * NUM_1M);
    KMEM_FREE_SLOT->ptr = KMEM_FREE_SLOT;
    KMEM_FREE_SLOT->size = ((96 - 64) * NUM_1M) / sizeof(memblk_info_t);
    prev = KMEM_FREE_SLOT;
  }

  memblk_info_t *p = prev->ptr;
  const size_t total_units =
      (n + sizeof(memblk_info_t) - 1) / sizeof(memblk_info_t) + 1;

  while (true) {
    if (p->size >= total_units) {
      if (p->size == total_units) {
        prev->ptr = p->ptr;
      } else {
        p->size -= total_units;
        p += p->size;
        p->size = total_units;
      }
      KMEM_FREE_SLOT = prev;
      ret_ptr = (void *)(p + 1);
      break;
    }
    if (p == KMEM_FREE_SLOT) {
      panic("falied to malloc");
    }
    prev = p;
    p = p->ptr;
  }

  xchg(&KMALLOC_LOCK, 0);
  return ret_ptr;
}

void kfree(void *ptr) {
  memblk_info_t *bp = (memblk_info_t *)ptr - 1;

  while (xchg(&KMALLOC_LOCK, 1) == 1) {
    schedule();
  }

  memblk_info_t *p = KMEM_FREE_SLOT;
  while (!(bp > p && bp < p->ptr)) {
    if (p >= p->ptr && (bp > p || bp < p->ptr)) {
      break;
    }
    p = p->ptr;
  }
  if (bp + bp->size == p->ptr) {
    bp->size += p->ptr->size;
    bp->ptr = p->ptr->ptr;
  } else {
    bp->ptr = p->ptr;
  }
  if (p + p->size == bp) {
    p->size += bp->size;
    p->ptr = bp->ptr;
  } else {
    p->ptr = bp;
  }
  KMEM_FREE_SLOT = p;

  xchg(&KMALLOC_LOCK, 0);
}
