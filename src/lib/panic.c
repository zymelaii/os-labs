#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <terminal.h>
#include <vga.h>

void _panic(const char* file, const char* func, const char* line,
            const char* msg) {
  const char sep[] = ":";
  const char ws[] = " ";
  const char newline[] = "\n";

  const char callee_prefix[] = "[retaddr] next pc at caller: 0x";
  char callee_addr[] = "00000000";

  void* ret_addr = NULL;
  asm volatile("mov 4(%%ebp), %0" : "=r"(ret_addr)::"memory");
  for (int i = 0; i < 8; ++i) {
    callee_addr[7 - i] =
        "0123456789abcdef"[((size_t)ret_addr >> (4 * i)) & 0xf];
  }

  const char* parts[] = {file,        sep,     func, sep,     line,
                         sep,         ws,      msg,  newline, callee_prefix,
                         callee_addr, newline, NULL};

  const u8 char_attr = (FOREGROUND(WHITE) | BACKGROUND(RED)) >> 8;
  int pos = 0;

  const char* const* part = parts;
  while (*part != NULL) {
    if (strcmp(*part, newline) == 0) {
      const int last_pos = pos;
      pos = ROUNDUP(pos, TERMINAL_COLUMN);
      for (int p = last_pos; p < pos; ++p) {
        term_write_char(p, ' ' | ((u16)char_attr << 8));
      }
    } else {
      term_write_str(pos, *part, char_attr);
      pos += strlen(*part);
    }
    ++part;
  }

  asm volatile("cli");
  asm volatile("hlt");
  while (1) {
  }
}
