#include <arch/x86.h>
#include <protect.h>
#include <serial.h>

void cstart() {
  init_protect_mode();
  init_serial();
}
