#include <protect.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <terminal.h>
#include <vga.h>

gdt_ptr_t gdt_ptr;
descriptor_t gdt[GDT_SIZE];

static void kernel_demo() {
  //! 在第 0 行输出黑底白色 N 蓝色 W 白底 P 黑色 U
  kprintf(TERMINAL_POS(0, 0), "N%fW%bP%fU", LIGHT_BLUE, WHITE, BLACK);

  //! 从第 2 行起输出白底每行绿紫相间的莎士比亚十四行诗
  {
    const char *poetry =
        "So is it not with me as with that muse,\n"
        "Stirred by a painted beauty to his verse,\n"
        "Who heaven it self for ornament doth use,\n"
        "And every fair with his fair doth rehearse,\n"
        "Making a couplement of proud compare\n"
        "With sun and moon, with earth and sea's rich gems:\n"
        "With April's first-born flowers and all things rare,\n"
        "That heaven's air in this huge rondure hems.\n"
        "O let me true in love but truly write,\n"
        "And then believe me, my love is as fair,\n"
        "As any mother's child, though not so bright\n"
        "As those gold candles fixed in heaven's air:\n"
        "Let them say more that like of hearsay well,\n"
        "I will not praise that purpose not to sell.\n";
    int row = 2;
    int col = 0;
    const char *p = poetry;
    while (*p && row < TERMINAL_ROW) {
      if (*++p != '\n') {
        ++col;
        continue;
      }
      const int left = MAX(0, TERMINAL_COLUMN - col) / 2;
      const int right = left + col;
      for (int i = 0; i < TERMINAL_COLUMN; ++i) {
        kprintf(TERMINAL_POS(row, i), "%b ", WHITE);
      }
      for (int i = left; i <= right; ++i) {
        kprintf(TERMINAL_POS(row, i), "%b%f%c", WHITE,
                row % 2 ? GREEN : FUCHUSIA, p[i - left - col - 1]);
      }
      ++row;
      col = 0;
      ++p;
    }
  }
}

void cstart() {
  memcpy(gdt, (void *)gdt_ptr.base, gdt_ptr.limit + 1);
  gdt_ptr.base = (u32)gdt;
  gdt_ptr.limit = GDT_SIZE * sizeof(descriptor_t) - 1;

  term_clear();
  kernel_demo();

  unreachable();
}
