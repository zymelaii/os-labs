#pragma once

enum {
  NR_get_ticks = 0,
  NR_get_pid,
  NR_read,
  NR_write,

  //! total syscalls
  NR_SYSCALLS = NR_write + 1,
};
