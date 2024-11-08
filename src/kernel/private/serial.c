#include <arch/x86.h>
#include <assert.h>
#include <serial.h>
#include <stdio.h>

int init_serial() {
  outb(PORT_COM1 + 1, 0x00);  //<! Disable all interrupts
  outb(PORT_COM1 + 3, 0x80);  //<! Enable DLAB (set baud rate divisor)
  outb(PORT_COM1 + 0, 0x03);  //<! Set divisor to 3 (lo byte) 38400 baud
  outb(PORT_COM1 + 1, 0x00);  //<!                  (hi byte)
  outb(PORT_COM1 + 3, 0x03);  //<! 8 bits, no parity, one stop bit
  outb(PORT_COM1 + 2, 0xc7);  //<! Enable FIFO with 14B threshold, clear them
  outb(PORT_COM1 + 4, 0x0b);  //<! IRQs enabled, RTS/DSR set
  outb(PORT_COM1 + 4, 0x1e);  //<! Set in loopback mode, test the serial chip
  outb(PORT_COM1 + 0, 0xae);  //<! Test serial chip (send byte 0xAE and check if
                              //<! serial returns same byte)

  //! check if serial is faulty, i.e: not same byte as sent
  if (inb(PORT_COM1 + 0) != 0xae) {
    panic("faulty serial");
    return 1;
  }

  //! if serial is not faulty set it in normal operation mode (not-loopback with
  //! IRQs enabled and OUT#1 and OUT#2 bits enabled)
  outb(PORT_COM1 + 4, 0x0f);
  return 0;
}

static inline int is_transmit_empty() { return inb(PORT_COM1 + 5) & 0x20; }

static inline int serial_received() { return inb(PORT_COM1 + 5) & 0x01; }

char serial_read() {
  while (!serial_received()) {
  }
  return inb(PORT_COM1);
}

void serial_write(u8 c) {
  while (!is_transmit_empty()) {
  }
  outb(PORT_COM1, c);
}

static void cprintfputch(char c, void *rc) {
  serial_write(c);
  ++*(int *)rc;
}

int vcprintf(const char *fmt, va_list ap) {
  int rc = 0;
  vprintfmt((void *)cprintfputch, &rc, fmt, ap);
  return rc;
}

int cprintf(const char *fmt, ...) {
  va_list ap;
  int rc;

  va_start(ap, fmt);
  rc = vcprintf(fmt, ap);
  va_end(ap);

  return rc;
}
