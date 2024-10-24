#include <arch/x86.h>
#include <interrupt.h>
#include <process.h>
#include <stdio.h>
#include <time.h>

static size_t SYSTEM_TICK;

void init_sysclk() {
  //! use 8253 PIT timer0 as system clock
  outb(TIMER_MODE, RATE_GENERATOR);
  outb(TIMER0, (uint8_t)((TIMER_FREQ / 10) >> 0));
  outb(TIMER0, (uint8_t)((TIMER_FREQ / 10) >> 8));
  SYSTEM_TICK = 0;

  //! enable clock irq for 8259A
  put_irq_handler(CLOCK_IRQ, clock_handler);
  enable_irq(CLOCK_IRQ);
}

static void sys_tick_incr() { SYSTEM_TICK++; }

size_t clock() { return SYSTEM_TICK; }

void clock_handler(int irq) {
  kprintf("#");
  ++p_proc_ready;
  sys_tick_incr();
  if (p_proc_ready >= pcb_table + NR_PCBS) {
    p_proc_ready = pcb_table;
  }
}
