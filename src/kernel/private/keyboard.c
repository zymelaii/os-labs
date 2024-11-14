#include <arch/x86.h>
#include <interrupt.h>
#include <keyboard.h>
#include <keymap.h>

#define KB_INBUF_SIZE 1024

typedef struct kb_inbuf {
  u32 lock;
  u8* p_head;
  u8* p_tail;
  int count;
  u8 buf[KB_INBUF_SIZE];
} kb_inbuf_t;

static kb_inbuf_t KB_INPUT = {
    .lock = 0,
    .p_head = KB_INPUT.buf,
    .p_tail = KB_INPUT.buf,
    .count = 0,
};

void init_keyboard() {
  put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
  enable_irq(KEYBOARD_IRQ);
}

void kb_put_key(u8 ch) {
  while (xchg(&KB_INPUT.lock, 1) == 1) {
  }

  do {
    if (KB_INPUT.count == KB_INBUF_SIZE) {
      break;
    }
    *KB_INPUT.p_tail++ = ch;
    if (KB_INPUT.p_tail == KB_INPUT.buf + KB_INBUF_SIZE)
      KB_INPUT.p_tail = KB_INPUT.buf;
    KB_INPUT.count++;
  } while (0);

  xchg(&KB_INPUT.lock, 0);
}

u8 kb_get_key() {
  u8 resp = 0xff;
  while (xchg(&KB_INPUT.lock, 1) == 1) {
  }

  do {
    if (KB_INPUT.count == 0) {
      break;
    }
    resp = *KB_INPUT.p_head++;
    if (KB_INPUT.p_head == KB_INPUT.buf + KB_INBUF_SIZE)
      KB_INPUT.p_head = KB_INPUT.buf;
    KB_INPUT.count--;
  } while (0);

  xchg(&KB_INPUT.lock, 0);
  return resp;
}

void keyboard_handler(int irq) {
  u8 ch = inb(0x60);
  if ((ch & 0x80) != 0) {
    return;
  }
  u32 code = keymap[ch];
  if (code >= 0x20 && code <= 0x7e) {
    kb_put_key(code);
  } else if (code == BACKSPACE) {
    kb_put_key('\b');
  } else if (code == ENTER) {
    kb_put_key('\n');
  }
}
