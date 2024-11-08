#pragma once

enum {
  NR_get_ticks = 0,
  NR_get_pid,
  NR_read,
  NR_write,
  NR_fork,
  NR_fork_ack,
  NR_exec,
  NR_wait,
  NR_exit,

  //! total syscalls
  NR_SYSCALLS = NR_exit + 1,
};
