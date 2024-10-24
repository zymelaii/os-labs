#include <assert.h>
#include <interrupt.h>
#include <keymap.h>
#include <stdint.h>
#include <stdio.h>

#define KB_INBUF_SIZE 4

typedef struct kb_inbuf {
  u8* p_head;
  u8* p_tail;
  int count;
  u8 buf[KB_INBUF_SIZE];
} kb_inbuf_t;

static kb_inbuf_t KB_INPUT = {
    .p_head = KB_INPUT.buf,
    .p_tail = KB_INPUT.buf,
    .count = 0,
};

void init_keyboard() {
  unimplemented("keyboard irq handler");
  enable_irq(KEYBOARD_IRQ);
}

void kb_put_key(u8 ch) { unimplemented(); }

char getch() {
  unimplemented();
  return 0;
}
