#include <assert.h>
#include <fs.h>
#include <process.h>
#include <sys/syscall.h>
#include <time.h>

#define SYSCALL_ENTRY(name) [NR_##name] = sys_##name

#define SYSCALL_ARGS1(t1) (t1) get_syscall_argument(0)
#define SYSCALL_ARGS2(t1, t2) \
  (t1) get_syscall_argument(0), (t2)get_syscall_argument(1)
#define SYSCALL_ARGS3(t1, t2, t3)                            \
  (t1) get_syscall_argument(0), (t2)get_syscall_argument(1), \
      (t3)get_syscall_argument(2)
#define SYSCALL_ARGS4(t1, t2, t3, t4)                        \
  (t1) get_syscall_argument(0), (t2)get_syscall_argument(1), \
      (t3)get_syscall_argument(2), (t4)get_syscall_argument(3)
#define SYSCALL_ARGS5(t1, t2, t3, t4, t5)                       \
  (t1) get_syscall_argument(0), (t2)get_syscall_argument(1),    \
      (t3)get_syscall_argument(2), (t4)get_syscall_argument(3), \
      (t5)get_syscall_argument(4)
#define SYSCALL_ARGS6(t1, t2, t3, t4, t5, t6)                   \
  (t1) get_syscall_argument(0), (t2)get_syscall_argument(1),    \
      (t3)get_syscall_argument(2), (t4)get_syscall_argument(3), \
      (t5)get_syscall_argument(4), (t6)get_syscall_argument(5)

static u32 get_syscall_argument(int index) {
  switch (index) {
    case 0: {
      return p_proc_ready->pcb.user_regs.ebx;
    } break;
    case 1: {
      return p_proc_ready->pcb.user_regs.ecx;
    } break;
    case 2: {
      return p_proc_ready->pcb.user_regs.edx;
    } break;
    case 3: {
      return p_proc_ready->pcb.user_regs.esi;
    } break;
    case 4: {
      return p_proc_ready->pcb.user_regs.edi;
    } break;
    case 5: {
      return p_proc_ready->pcb.user_regs.ebp;
    } break;
  }
  panic("syscall argument out of range");
}

static u32 sys_get_ticks() { return get_ticks(); }

static u32 sys_get_pid() { return get_pid(NULL); }

static u32 sys_read() { return read(SYSCALL_ARGS3(int, void *, int)); }

static u32 sys_write() { return write(SYSCALL_ARGS3(int, void *, int)); }

static syscall_t SYSCALL_TABLE[NR_SYSCALLS] = {
    SYSCALL_ENTRY(get_ticks),
    SYSCALL_ENTRY(get_pid),
    SYSCALL_ENTRY(read),
    SYSCALL_ENTRY(write),
};

void syscall_handler() {
  const int syscall_nr = p_proc_ready->pcb.user_regs.eax;
  const u32 ret = (*SYSCALL_TABLE[syscall_nr])();
  p_proc_ready->pcb.user_regs.eax = ret;
}
