#include <arch/x86.h>
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <terminal.h>
#include <vga.h>

#define CSI_ESC 0
#define CSI_BRACKET 1
#define CSI_PARAM1 2
#define CSI_PARAM2 3

#define CLEAR_CURSOR2END 0
#define CLEAR_CURSOR2BEGIN 1
#define CLEAR_ENTIRE 2

struct kprintfbuf {
  //! common fields
  u16 color;
  i16 cursor_row;
  i16 cursor_col;
  int cnt;
  //! fields for csi ctrl
  int CSI;
  i16 param1;
  i16 param2;
};

static struct kprintfbuf TTY = {
    .color = DEFAULT_COLOR,
    .cursor_row = 0,
    .cursor_col = 0,
};

void term_write_char(u16 pos, u16 char_code) {
  asm("mov %1, %%gs:(%0)" ::"r"(pos * 2), "r"(char_code) : "memory");
}

void term_write_str(u16 pos, const char *str, u8 char_attr) {
  const char *p = str;
  while (*p) {
    term_write_char(pos++, *p++ | ((u16)char_attr << 8));
  }
}

void term_clear() {
  u16 content = DEFAULT_COLOR | ' ';
  for (int i = 0; i < TERMINAL_ROW; ++i) {
    for (int j = 0; j < TERMINAL_COLUMN; ++j) {
      term_write_char(TERMINAL_POS(i, j), content);
    }
  }
}

inline static u16 cursor_pos(struct kprintfbuf *b) {
  return TERMINAL_POS(b->cursor_row, b->cursor_col);
}

static void cursor_move(i16 move_row, i16 move_col, struct kprintfbuf *b) {
  b->cursor_row = CLAMP(b->cursor_row + move_row, 0, TERMINAL_ROW - 1);
  b->cursor_col = CLAMP(b->cursor_col + move_col, 0, TERMINAL_COLUMN - 1);
}

static void clear_screen(struct kprintfbuf *b) {
  u16 content = DEFAULT_COLOR | ' ';
  if (b->param1 == CLEAR_CURSOR2END) {
    u16 disp_pos = cursor_pos(b);
    while (disp_pos < TERMINAL_SIZE) term_write_char(disp_pos++, content);
  } else if (b->param1 == CLEAR_CURSOR2BEGIN) {
    u16 disp_pos = cursor_pos(b);
    while (disp_pos > 0) {
      term_write_char(disp_pos--, content);
    }
    term_write_char(disp_pos, content);
  } else if (b->param1 == CLEAR_ENTIRE) {
    u16 disp_pos = 0;
    while (disp_pos < TERMINAL_SIZE) term_write_char(disp_pos++, content);
  }
}

static void clear_line(struct kprintfbuf *b) {
  u16 content = DEFAULT_COLOR | ' ';
  if (b->param1 == CLEAR_CURSOR2END) {
    u16 disp_pos = cursor_pos(b);
    while (disp_pos % TERMINAL_COLUMN != 0)
      term_write_char(disp_pos++, content);
  } else if (b->param1 == CLEAR_CURSOR2BEGIN) {
    u16 disp_pos = cursor_pos(b);
    while (disp_pos % TERMINAL_COLUMN != 0)
      term_write_char(disp_pos--, content);
    term_write_char(disp_pos, content);
  } else if (b->param1 == CLEAR_ENTIRE) {
    u16 disp_pos = TERMINAL_POS(b->cursor_row, 0);
    do {
      term_write_char(disp_pos++, content);
    } while (disp_pos % TERMINAL_COLUMN != 0);
  }
}

static void scroll(i16 scroll_up_num) {
  scroll_up_num = MIN(scroll_up_num, +TERMINAL_ROW);
  scroll_up_num = MAX(scroll_up_num, -TERMINAL_ROW);

  if (scroll_up_num > 0) {
    void *dst = (void *)(TERMINAL_POS(0, 0) * 2 + VMEM_LIN_ADDR);
    void *src = (void *)(TERMINAL_POS(scroll_up_num, 0) * 2 + VMEM_LIN_ADDR);
    size_t clear_size = scroll_up_num * TERMINAL_COLUMN * 2;
    size_t copy_size = TERMINAL_SIZE * 2 - clear_size;
    memcpy(dst, src, copy_size);
    void *v = dst + copy_size;
    memset(v, 0, clear_size);
  } else if (scroll_up_num < 0) {
    i16 scroll_down_num = -scroll_up_num;
    void *dst = (void *)(TERMINAL_POS(scroll_down_num, 0) * 2);
    dst += (ptrdiff_t)VMEM_LIN_ADDR;
    void *src = (void *)(TERMINAL_POS(0, 0) * 2);
    src += (ptrdiff_t)VMEM_LIN_ADDR;
    size_t clear_size = scroll_down_num * TERMINAL_COLUMN * 2;
    size_t copy_size = TERMINAL_SIZE * 2 - clear_size;

    memcpy(dst, src, copy_size);

    void *v = src;
    memset(v, 0, clear_size);
  }
}

inline static void param12vga_color(struct kprintfbuf *b) {
  u8 tmp = b->param1 & 1;
  b->param1 &= 0b0110;
  b->param1 |= b->param1 >> 2;
  b->param1 &= 0b0011;
  b->param1 |= tmp << 2;
}

static void set_color(struct kprintfbuf *b) {
  if (b->param1 == 0) {
    b->color = DEFAULT_COLOR;
  } else if (b->param1 == 1) {
    b->color |= 0x8800;
  } else if (b->param1 == 2) {
    b->color &= 0x7700;
  } else if (30 <= b->param1 && b->param1 <= 37) {
    b->param1 -= 30;
    param12vga_color(b);
    b->color = (b->color & 0xf8ff) | FOREGROUND(b->param1);
  } else if (40 <= b->param1 && b->param1 <= 47) {
    b->param1 -= 40;
    param12vga_color(b);
    b->color = (b->color & 0x8fff) | BACKGROUND(b->param1);
  } else if (90 <= b->param1 && b->param1 <= 97) {
    b->param1 -= 90;
    param12vga_color(b);
    b->param1 |= 0x8;
    b->color = (b->color & 0xf0ff) | FOREGROUND(b->param1);
  } else if (100 <= b->param1 && b->param1 <= 107) {
    b->param1 -= 100;
    param12vga_color(b);
    b->param1 |= 0x8;
    b->color = (b->color & 0x0fff) | BACKGROUND(b->param1);
  } else {
    warn("unsupport CSI: %dm", b->param1);
  }
}

static void CSI_handler(u8 terminator, struct kprintfbuf *b) {
  b->CSI = CSI_ESC;

  switch (terminator) {
    case 'A':
      if (b->param1 == 0) b->param1 = 1;
      cursor_move(-b->param1, 0, b);
      break;
    case 'B':
      if (b->param1 == 0) b->param1 = 1;
      cursor_move(+b->param1, 0, b);
      break;
    case 'C':
      if (b->param1 == 0) b->param1 = 1;
      cursor_move(0, +b->param1, b);
      break;
    case 'D':
      if (b->param1 == 0) b->param1 = 1;
      cursor_move(0, -b->param1, b);
      break;
    case 'E':
      if (b->param1 == 0) b->param1 = 1;
      cursor_move(+b->param1, 1 - b->cursor_col, b);
      break;
    case 'F':
      if (b->param1 == 0) b->param1 = 1;
      cursor_move(-b->param1, 1 - b->cursor_col, b);
      break;
    case 'H':
      if (b->param1 == 0) b->param1 = 1;
      if (b->param2 == 0) b->param2 = 1;
      cursor_move(b->param1 - b->cursor_row, b->param2 - b->cursor_col, b);
      break;
    case 'J':
      clear_screen(b);
      break;
    case 'K':
      clear_line(b);
      break;
    case 'S':
      if (b->param1 == 0) b->param1 = 1;
      scroll(+b->param1);
      break;
    case 'T':
      if (b->param1 == 0) b->param1 = 1;
      scroll(-b->param1);
      break;
    case 'm':
      set_color(b);
      break;
    default:
      warn("unsupport CSI: %c", terminator);
      break;
  }
}

static void cursor_move_pre(struct kprintfbuf *b) {
  if (b->cursor_col == 0) {
    if (b->cursor_row > 0) {
      b->cursor_row--;
    } else {
      scroll(-1);
    }
    b->cursor_col = TERMINAL_COLUMN;
  } else {
    b->cursor_col--;
  }
}

static void cursor_move_nxt(struct kprintfbuf *b) {
  if (b->cursor_col + 1 == TERMINAL_COLUMN) {
    if (b->cursor_row + 1 < TERMINAL_ROW) {
      b->cursor_row++;
    } else {
      scroll(1);
    }
    b->cursor_col = 0;
  } else {
    b->cursor_col++;
  }
}

static void kprintfputch(int ch, struct kprintfbuf *b) {
  ch = ch & 0xff;
  b->cnt++;

reswitch:
  if (b->CSI == CSI_ESC) {
    switch (ch) {
      case '\b':
        cursor_move_pre(b);
        break;
      case '\t':
        if (cursor_pos(b) == TERMINAL_SIZE - 1) break;
        while (b->cursor_col % 4 != 1) cursor_move_nxt(b);
        break;
      case '\n':
        cursor_move(0, TERMINAL_COLUMN - b->cursor_col, b);
        cursor_move_nxt(b);
        break;
      case '\x1b':
        b->CSI++;
        break;
      default: {
        u16 disp_pos = TERMINAL_POS(b->cursor_row, b->cursor_col);
        u16 content = b->color | ch;
        term_write_char(disp_pos, content);
        cursor_move_nxt(b);
        break;
      }
    }
  } else if (b->CSI == CSI_BRACKET) {
    switch (ch) {
      case '[':
        b->CSI++;
        b->param1 = b->param2 = 0;
        break;
      default:
        b->CSI = CSI_ESC;
        break;
    }
  } else {
    switch (ch) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        if (b->CSI == CSI_PARAM1)
          b->param1 = b->param1 * 10 + ch - '0';
        else if (b->CSI == CSI_PARAM2)
          b->param2 = b->param2 * 10 + ch - '0';
        else
          ;  // do nothing
        break;
      case ';':
        b->CSI++;
        break;
      default:
        if (!(0x20 <= ch && ch <= 0x7e)) b->CSI = CSI_ESC;
        if (0x40 <= ch && ch <= 0x7e) CSI_handler(ch, b);
        break;
    }
  }
}

int vkprintf(const char *fmt, va_list ap) {
  TTY.cnt = 0;

  vprintfmt((void *)kprintfputch, &TTY, fmt, ap);

  int rc = TTY.cnt;

  return rc;
}

/*!
 * \brief kernel print method, support default fmt mode and csi sequence

 * \note CSI 为以 ESC '\x1b[' '\e[' 开头的序列，用于控制光标和终端属性
 * \note see https://en.wikipedia.org/wiki/ANSI_escape_code
 *
 * * \x1b[nA
 * 将光标向上移动 n 行，如果移动中接触到边界则不继续移动
 * 其中 \x1b[A 等价于 \x1b[1A
 *
 * * \x1b[nB
 * 将光标向下移动 n 行，如果移动中接触到边界则不继续移动
 * 其中 \x1b[B 等价于 \x1b[1B
 *
 * * \x1b[nC
 * 将光标向右移动 n 列，如果移动中接触到边界则不继续移动
 * 其中 \x1b[C 等价于 \x1b[1C
 *
 * * \x1b[nD
 * 将光标向左移动 n 列，如果移动中接触到边界则不继续移动
 * 其中 \x1b[D 等价于 \x1b[1D
 *
 * * \x1b[nE
 * 将光标向下移动 n 行后移动到行首，如果移动中接触到边界则不继续移动
 * 其中 \x1b[E 等价于 \x1b[1E
 *
 * * \x1b[nF
 * 将光标向上移动 n 行后移动到行首，如果移动中接触到边界则不继续移动
 * 其中 \x1b[F 等价于 \x1b[1F
 *
 * * \x1b[n;mH
 * 将光标移动到 n 行 m 列，左上角位 1 行 1 列，n、m 为空时默认为 1，如果移动中接
 * 触到边界则不继续移动
 * - \x1b[;5H 等价于 \x1b[1;5H
 * - \x1b[5H 等价于 \x1b[5;H 等价于 \x1b[5;1H
 * - \x1b[H 等价于 \x1b[1;1H
 *
 * * \x1b[nJ
 * 终端清屏操作，其中 \x1b[J 等价于 \x1b[0J
 * - 当 n 为 0 时，清除当前光标位置到终端末尾的所有字符输出
 * - 当 n 为 1 时，清除当前光标位置到终端开头的所有字符输出
 * - 当 n 为 2 时，清除当前终端的所有字符输出
 *
 * * \x1b[nK
 * 当前行清除字符操作，其中 \x1b[J 等价于 \x1b[0J
 * - 当 n 为 0 时，清除当前光标位置到行末的所有字符输出
 * - 当 n 为 1 时，清除当前光标位置到行首的所有字符输出
 * - 当 n 为 2 时，清除光标位置所在行的所有字符输出
 *
 * * \x1b[nS
 * 将整个终端显示向上移动 n 行，底部字符用空白填充，其中 \x1b[S 等价于 \x1b[1S
 *
 * * \x1b[nT
 * 将整个终端显示向下移动 n 行，顶部字符用空白填充，其中 \x1b[T 等价于 \x1b[1T
 *
 * * \x1b[nm
 * 控制终端之后要输出的颜色，其中 \x1b[m 等价于\x1b[0m
 * - 当 n 为 0 时，将终端之后要输出的字符颜色置为默认色（白底黑字）
 * - 当 n 为 1 时，将终端之后要输出的字符颜色亮度提高（如果之前输出的是暗色）
 * - 当 n 为 2 时，将终端之后要输出的字符颜色亮度降低（如果之前输出的是亮色）
 * - 当 n 为 30/40 时，将终端之后要输出的字符的 前景色/背景色 置为 黑色
 * - 当 n 为 31/41 时，将终端之后要输出的字符的 前景色/背景色 置为 红色
 * - 当 n 为 32/42 时，将终端之后要输出的字符的 前景色/背景色 置为 绿色
 * - 当 n 为 33/43 时，将终端之后要输出的字符的 前景色/背景色 置为 棕色
 * - 当 n 为 34/44 时，将终端之后要输出的字符的 前景色/背景色 置为 蓝色
 * - 当 n 为 35/45 时，将终端之后要输出的字符的 前景色/背景色 置为 洋红色
 * - 当 n 为 36/46 时，将终端之后要输出的字符的 前景色/背景色 置为 青色
 * - 当 n 为 37/47 时，将终端之后要输出的字符的 前景色/背景色 置为 银色
 * - 当 n 为 90/100 时，将终端之后要输出的字符的 前景色/背景色 置为 灰色
 * - 当 n 为 91/101 时，将终端之后要输出的字符的 前景色/背景色 置为 亮红色
 * - 当 n 为 92/102 时，将终端之后要输出的字符的 前景色/背景色 置为 亮绿色
 * - 当 n 为 93/103 时，将终端之后要输出的字符的 前景色/背景色 置为 黄色
 * - 当 n 为 94/104 时，将终端之后要输出的字符的 前景色/背景色 置为 亮蓝色
 * - 当 n 为 95/105 时，将终端之后要输出的字符的 前景色/背景色 置为 亮洋红色
 * - 当 n 为 96/106 时，将终端之后要输出的字符的 前景色/背景色 置为 亮青色
 * - 当 n 为 97/107 时，将终端之后要输出的字符的 前景色/背景色 置为 白色色
 */
int kprintf(const char *fmt, ...) {
  va_list ap;
  int rc;

  va_start(ap, fmt);
  rc = vkprintf(fmt, ap);
  va_end(ap);

  return rc;
}
