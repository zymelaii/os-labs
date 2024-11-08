#include <arch/x86.h>
#include <interrupt.h>
#include <process.h>
#include <schedule.h>
#include <stddef.h>
#include <time.h>

static size_t SYSTEM_TICK;

void init_sysclk() {
  //! use 8253 PIT timer0 as system clock
  outb(TIMER_MODE, RATE_GENERATOR);
  outb(TIMER0, (uint8_t)((TIMER_FREQ / SYSCLK_FREQ_HZ) >> 0));
  outb(TIMER0, (uint8_t)((TIMER_FREQ / SYSCLK_FREQ_HZ) >> 8));
  SYSTEM_TICK = 0;

  //! enable clock irq for 8259A
  put_irq_handler(CLOCK_IRQ, clock_handler);
  enable_irq(CLOCK_IRQ);
}

static void sys_tick_incr() { SYSTEM_TICK++; }

void clock_handler(int irq) {
  sys_tick_incr();
  if (--p_proc_ready->pcb.ticks == 0) {
    p_proc_ready->pcb.ticks = p_proc_ready->pcb.priority;
    schedule();
  }
}

size_t get_ticks() { return SYSTEM_TICK; }
