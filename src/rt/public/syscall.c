#include <assert.h>
#include <stddef.h>
#include <sys/syscall.h>
#include <sys/types.h>

static int syscall0(int syscall_nr) {
  int ret = 0;
  asm volatile("int $0x80" : "=a"(ret) : "a"(syscall_nr) : "cc", "memory");
  return ret;
}

static int syscall1(int syscall_nr, int p1) {
  int ret = 0;
  asm volatile("int $0x80"
               : "=a"(ret)
               : "a"(syscall_nr), "b"(p1)
               : "cc", "memory");
  return ret;
}

static int syscall2(int syscall_nr, int p1, int p2) {
  int ret = 0;
  asm volatile("int $0x80"
               : "=a"(ret)
               : "a"(syscall_nr), "b"(p1), "c"(p2)
               : "cc", "memory");
  return ret;
}

static int syscall3(int syscall_nr, int p1, int p2, int p3) {
  int ret = 0;
  asm volatile("int $0x80"
               : "=a"(ret)
               : "a"(syscall_nr), "b"(p1), "c"(p2), "d"(p3)
               : "cc", "memory");
  return ret;
}

static int syscall4(int syscall_nr, int p1, int p2, int p3, int p4) {
  int ret = 0;
  asm volatile("int $0x80"
               : "=a"(ret)
               : "a"(syscall_nr), "b"(p1), "c"(p2), "d"(p3), "S"(p4)
               : "cc", "memory");
  return ret;
}

static int syscall5(int syscall_nr, int p1, int p2, int p3, int p4, int p5) {
  int ret = 0;
  asm volatile("int $0x80"
               : "=a"(ret)
               : "a"(syscall_nr), "b"(p1), "c"(p2), "d"(p3), "S"(p4), "D"(p5)
               : "cc", "memory");
  return ret;
}

size_t get_ticks() { return syscall0(NR_get_ticks); }

pid_t get_pid() { return syscall0(NR_get_pid); }

int read(int fd, void *buf, size_t count) {
  return syscall3(NR_read, fd, (int)buf, count);
}

int write(int fd, const void *buf, size_t count) {
  return syscall3(NR_write, fd, (int)buf, count);
}

pid_t fork() { return syscall0(NR_fork); }

void fork_ack() { syscall0(NR_fork_ack); }

int exec(const char *path) { return syscall1(NR_exec, (int)path); }

pid_t wait(int *wstatus) { return syscall1(NR_wait, (int)wstatus); }

void exit(int exit_code) {
  //! 为什么这个参数这么奇怪？你可能需要读读手册
  syscall1(NR_exit, (exit_code & 0xff) << 8);
  unreachable();
}
