#include <arch/x86.h>
#include <assert.h>
#include <fat32.h>
#include <fs.h>
#include <keyboard.h>
#include <layout.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#define SECTOR_SIZE 512

u32 PartitionLBA;

static void waitdisk() {
  // wait for disk reaady
  while ((inb(0x1f7) & 0xc0) != 0x40) {
  }
}

static void readsect(void *dst, u32 offset) {
  //! offset to current partition
  offset += PartitionLBA;

  // wait for disk to be ready
  waitdisk();

  outb(0x1f2, 1);  // count = 1
  outb(0x1f3, offset);
  outb(0x1f4, offset >> 8);
  outb(0x1f5, offset >> 16);
  outb(0x1f6, (offset >> 24) | 0xe0);
  outb(0x1f7, 0x20);  // cmd 0x20 - read sectors

  // wait for disk to be ready
  waitdisk();

  // read a sector
  insl(0x1f0, dst, SECTOR_SIZE / 4);
}

u32 fat_start_sec;
u32 data_start_sec;
u32 fat_now_sec;
fat32_bpb_t bpb;

static u32 get_next_clus(u32 current_clus) {
  u32 sec = current_clus * 4 / SECTOR_SIZE;
  u32 off = current_clus * 4 % SECTOR_SIZE;
  static u32 buf[SECTOR_SIZE / 4];
  if (fat_now_sec != fat_start_sec + sec) {
    readsect(buf, fat_start_sec + sec);
    fat_now_sec = fat_start_sec + sec;
  }
  return buf[off / 4];
}

static void *read_data_sec(void *dst, u32 current_clus) {
  current_clus -= 2;
  current_clus *= bpb.BPB_SecPerClus;
  current_clus += data_start_sec;
  for (int i = 0; i < bpb.BPB_SecPerClus; i++, dst += SECTOR_SIZE) {
    readsect(dst, current_clus + i);
  }
  return dst;
}

/*!
 * 内核读入函数，由于没有文件描述符概念，所以强制 assert
 * 让 fd 为 0，平时的读入流也默认是 0 的现在只有从键盘缓
 * 冲区获取字符的能力
 */
int read(int fd, void *buf, size_t count) {
  assert(fd == STDIN_FILENO);
  assert(buf + count < (void *)KERNEL_LIN_BASE);
  char *s = buf;
  for (size_t i = 0; i < count; ++i) {
    u8 c = kb_get_key();
    if (c == 0xff) {
      return i;
    }
    *s++ = c;
  }
  return count;
}

/*!
 * 内核写入函数，由于没有文件描述符概念，所以强制 assert
 * 让 fd 为 1，平时的输出流也默认是 1 的，现在只有输出字
 * 符到终端的能力
 */
int write(int fd, void *buf, size_t count) {
  assert(fd == STDOUT_FILENO);
  assert(buf + count < (void *)KERNEL_LIN_BASE);
  const char *s = buf;
  for (size_t i = 0; i < count; ++i) {
    kprintf("%c", *s++);
  }
  return count;
}

void init_fs(u32 partition_lba) { PartitionLBA = partition_lba; }

/*!
 * \brief read file from fs to dst
 * \param [in] filename short fat name of target file
 * \param [out] dst addr to store file content, recommend 3GB + 48MB
 */
int read_file(const char *filename, void *dst) {
  assert(strlen(filename) == 11);

  readsect(&bpb, 0);

  fat_start_sec = bpb.BPB_RsvdSecCnt;
  data_start_sec = fat_start_sec + bpb.BPB_FATSz32 * bpb.BPB_NumFATs;

  u32 root_clus = bpb.BPB_RootClus;
  u32 file_clus = 0;

  assert(bpb.BPB_BytsPerSec == SECTOR_SIZE && bpb.BPB_SecPerClus == 8);

  static char buf[SECTOR_SIZE * 8];
  while (root_clus < 0x0ffffff8) {
    void *read_end = read_data_sec((void *)buf, root_clus);
    for (fat32_dirent_t *p = (void *)buf; (void *)p < read_end; p++) {
      if (strncmp(p->DIR_Name, filename, 11) == 0) {
        assert(p->DIR_FileSize <= 16 * NUM_1M);
        file_clus = (u32)p->DIR_FstClusHI << 16 | p->DIR_FstClusLO;
        break;
      }
    }
    if (file_clus != 0) {
      break;
    }
    root_clus = get_next_clus(root_clus);
  }

  if (file_clus == 0) {
    return -ENOENT;
  }

  while (file_clus < 0x0ffffff8) {
    dst = read_data_sec(dst, file_clus);
    file_clus = get_next_clus(file_clus);
  }

  return 0;
}
