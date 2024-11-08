#include <arch/x86.h>
#include <assert.h>
#include <config.h>
#include <elf.h>
#include <fat32.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <terminal.h>

#define SECTOR_SIZE 512
#define CLUSTER_SIZE 4096

u32 PartitionLBA;

u32 fat_start_sec;
u32 data_start_sec;
u32 elf_clus;
u32 elf_off;
u32 fat_now_sec;
fat32_bpb_t bpb;

static void waitdisk() {
  //! wait for disk ready
  while ((inb(0x1f7) & 0xc0) != 0x40) {
  }
}

static void readsect(void *dst, u32 offset) {
  //! offset to current partition
  offset += PartitionLBA;

  waitdisk();
  outb(0x1f2, 1);  //<! count = 1
  outb(0x1f3, offset);
  outb(0x1f4, offset >> 8);
  outb(0x1f5, offset >> 16);
  outb(0x1f6, (offset >> 24) | 0xe0);
  outb(0x1f7, 0x20);  //<! cmd 0x20 - read sectors
  waitdisk();

  //! read a sector to dst
  insl(0x1f0, dst, SECTOR_SIZE / 4);
}

static u32 get_next_clus(u32 current_clus) {
  const u32 sec = current_clus * 4 / SECTOR_SIZE;
  const u32 off = current_clus * 4 % SECTOR_SIZE;
  if (fat_now_sec != fat_start_sec + sec) {
    readsect((void *)FAT_ADDR, fat_start_sec + sec);
    fat_now_sec = fat_start_sec + sec;
  }
  return *(u32 *)(FAT_ADDR + off);
}

static void *read_data_sec(void *dst, u32 clus) {
  u32 sec = (clus - 2) * bpb.BPB_SecPerClus;
  sec += data_start_sec;
  for (int i = 0; i < bpb.BPB_SecPerClus; i++, dst += SECTOR_SIZE) {
    readsect(dst, sec + i);
  }
  return dst;
}

/*!
 * 根据输入的参数读取对应的一段，由于 kernel.bin 是正经 ld 链接的，所以文件偏
 * 移和虚拟地址的增长速度相同，所以采取激进的读取策略，如果文件偏移不能从当前的
 * elf_clus 推出到需要读取的簇号则直接默认为读取失败
 */
static void readseg(void *va, u32 count, u32 offset) {
  void *end_va = va + count;
  assert(offset >= elf_off);
  while (va < end_va) {
    /*!
     * 如果 [elf_off, elf_off + CLUSTER_SIZE) 里面有段需要的数据就将对应的数据从
     * 文件中加载到指定位置，否则找下一个簇，移动 elf_off 的位置
     *
     * 如果要加载整个段，就直接读入
     *
     * 如果要加载部分段，就将其读入至缓存再从缓存转移到指定位置
     */
    if (offset - elf_off < CLUSTER_SIZE) {
      u32 buf_off = offset - elf_off;
      u32 buf_len = MIN(CLUSTER_SIZE - buf_off, end_va - va);
      if (buf_off == 0 && buf_len == CLUSTER_SIZE) {
        read_data_sec((void *)va, elf_clus);
      } else {
        read_data_sec((void *)BUF_ADDR, elf_clus);
        memcpy(va, (void *)BUF_ADDR + buf_off, buf_len);
      }
      va += buf_len;
      offset += buf_len;
    } else {
      elf_off += CLUSTER_SIZE;
      elf_clus = get_next_clus(elf_clus);
    }
  }
}

static u32 find_kernel_elf_clus() {
  u32 elf_clus = 0;
  u32 root_clus = bpb.BPB_RootClus;
  while (root_clus < 0x0ffffff8) {
    fat32_dirent_t *p = (void *)BUF_ADDR;
    void *buf_end = read_data_sec((void *)BUF_ADDR, root_clus);
    for (; p < (fat32_dirent_t *)buf_end; p++) {
      //! NOTE: skip file with long fat name
      if (p->DIR_Attr == 0xf) continue;
      if (strncmp(p->DIR_Name, KERNEL_FAT_NAME, 11) == 0) {
        elf_clus = (u32)p->DIR_FstClusHI << 16 | p->DIR_FstClusLO;
        break;
      }
    }
    if (elf_clus != 0) break;
    root_clus = get_next_clus(root_clus);
  }
  assert(elf_clus);

  return elf_clus;
}

static void load_kernel_elf(elf32_hdr_t *eh) {
  elf32_phdr_t *ph = (void *)eh + eh->e_phoff;
  for (int i = 0; i < eh->e_phnum; i++, ph++) {
    if (ph->p_type != PT_LOAD) {
      continue;
    }
    readseg((void *)ph->p_vaddr, ph->p_filesz, ph->p_offset);
    memset((void *)ph->p_vaddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
  }
}

void load_kernel() {
  term_clear();

  const char *msg_enter = "--- start loading kernel elf ---";
  const char *msg_leave = "---    kernel elf loaded     ---";

  kprintf("%s\n", msg_enter);

  readsect((void *)&bpb, 0);
  assert(bpb.BPB_BytsPerSec == SECTOR_SIZE && bpb.BPB_SecPerClus == 8);
  fat_start_sec = bpb.BPB_RsvdSecCnt;
  data_start_sec = fat_start_sec + bpb.BPB_FATSz32 * bpb.BPB_NumFATs;

  elf_clus = find_kernel_elf_clus();

  read_data_sec((void *)ELF_ADDR, elf_clus);
  elf32_hdr_t *eh = (void *)ELF_ADDR;
  assert(eh->e_ehsize <= CLUSTER_SIZE);

  load_kernel_elf(eh);

  kprintf("%s\n", msg_leave);

  typedef void (*kernel_entry_t)(u32);
  ((kernel_entry_t)eh->e_entry)(PartitionLBA);

  unreachable();
}
