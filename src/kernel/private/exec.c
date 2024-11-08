#include <assert.h>
#include <elf.h>
#include <exec.h>
#include <fat32.h>
#include <fs.h>
#include <interrupt.h>
#include <malloc.h>
#include <page.h>
#include <process.h>
#include <protect.h>
#include <schedule.h>
#include <string.h>
#include <sys/errno.h>

static int exec_fat(process_t *proc, const char *fat_name) {
  assert((uintptr_t)fat_name >= (uintptr_t)K_PHY2LIN(0));

  int result = 0;
  pcb_t *pcb = &proc->pcb;

  while (xchg(&pcb->lock, 1) == 1) {
    schedule();
  }

  do {
    void *elf_addr = (void *)K_PHY2LIN(48 * NUM_1M);
    result = read_file(fat_name, elf_addr);
    if (result != 0) {
      break;
    }

    if (((elf32_hdr_t *)elf_addr)->e_magic != ELF_MAGIC) {
      result = -ENOEXEC;
      break;
    }

    assert(pcb->status == PSTATUS_READY);

    memset(&pcb->user_regs, 0, sizeof(pcb->user_regs));
    init_segment_regs(pcb);
    pcb->user_regs.eflags = EFLAGS_RESERVED | EFLAGS_IF | EFLAGS_IOPL(1);

    init_pagetbl(pcb);
    map_elf(pcb, elf_addr);
    map_stack(pcb);

  } while (0);

  xchg(&pcb->lock, 0);
  return result;
}

/*!
 * \brief translate path src to short fat name dst
 * \retval 0 success
 * \retval -ENOENT failed to translate
 */
static int translate_pathname(char *dst, const char *src) {
  assert(strlen(dst) == FAT32_SHORT_NAME_LEN);

  char *st = (char *)src;
  char *ed = st + strlen(st);
  char *dot = ed;
  for (char *c = st; *c; ++c) {
    if (*c == '.') {
      dot = c;
    }
  }

  if (dot - st > 8) {
    return -ENOENT;
  }
  memcpy(dst, st, dot - st);

  if (ed - dot - 1 > 3) {
    return -ENOENT;
  }
  memcpy(dst + 8, dot + 1, ed == dot ? 0 : ed - dot - 1);

  for (char *c = dst; *c; c++) {
    if ('a' <= *c && *c <= 'z') {
      *c += 'A' - 'a';
    }
  }

  return 0;
}

int exec(process_t *proc, const char *path) {
  if (proc == NULL) {
    proc = p_proc_ready;
  }

  int result = 0;

  char *fat_name_kbuf = kmalloc(FAT32_SHORT_NAME_LEN + 1);
  assert(fat_name_kbuf);

  memset(fat_name_kbuf, ' ', FAT32_SHORT_NAME_LEN);
  fat_name_kbuf[FAT32_SHORT_NAME_LEN] = '\0';

  do {
    result = translate_pathname(fat_name_kbuf, path);
    if (result != 0) {
      break;
    }
    result = exec_fat(proc, fat_name_kbuf);
  } while (0);

  if (fat_name_kbuf) {
    kfree(fat_name_kbuf);
  }

  return result;
}
