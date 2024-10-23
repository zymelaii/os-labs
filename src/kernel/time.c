#include <process.h>
#include <stdio.h>
#include <time.h>

static size_t SYSTEM_TICK;

void timecounter_inc() { SYSTEM_TICK++; }

size_t clock() { return SYSTEM_TICK; }

void clock_handler(int irq) {
  kprintf("#");
  ++p_proc_ready;
  timecounter_inc();
  if (p_proc_ready >= pcb_table + NR_PCBS) {
    p_proc_ready = pcb_table;
  }
}
