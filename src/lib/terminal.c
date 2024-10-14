#include <terminal.h>
#include <vga.h>

void term_write_char(u16 pos, u16 char_code) {
  asm("mov %1, %%gs:(%0)" ::"r"(pos * 2), "r"(char_code) : "memory");
}

void term_write_str(u16 pos, const char* str, u8 char_attr) {
  const char* p = str;
  while (*p) {
    term_write_char(pos++, *p++ | ((u16)char_attr << 8));
  }
}

void term_clear() {
  u16 content = DEFAULT_COLOR | ' ';
  for (int i = 0; i < TERMINAL_ROW; i++) {
    for (int j = 0; j < TERMINAL_COLUMN; j++) {
      term_write_char(TERMINAL_POS(i, j), content);
    }
  }
}
