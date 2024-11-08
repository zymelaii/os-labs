#include <arch/x86.h>
#include <io.h>
#include <stdint.h>
#include <stdio.h>

extern void printfmt(void (*putch)(int, void *), void *putdat, const char *fmt,
                     ...);
extern void vprintfmt(void (*putch)(int, void *), void *putdat, const char *fmt,
                      va_list ap);

#define OBUF_SIZE 4096
#define IBUF_SIZE 1024

typedef struct printfbuf_t {
  uint32_t lock;
  char buf[OBUF_SIZE];
  char *buf_p;
  int cnt;
} printfbuf_t;

typedef struct getchbuf {
  uint32_t lock;
  char buf[IBUF_SIZE];
  char *st;
  char *en;
} getchbuf_t;

static printfbuf_t printfb = {
    .lock = 0,
    .buf_p = printfb.buf,
    .cnt = 0,
};

static getchbuf_t getchb = {
    .lock = 0,
    .st = getchb.buf,
    .en = getchb.buf,
};

static void printfputch(int ch, printfbuf_t *b) {
  b->cnt++;
  *b->buf_p++ = (char)ch;
  if (ch == '\n' || b->buf_p == b->buf + OBUF_SIZE) {
    write(STDOUT_FILENO, b->buf, b->buf_p - b->buf);
    b->buf_p = b->buf;
  }
}

int vprintf(const char *fmt, va_list ap) {
  printfbuf_t *b = &printfb;

  while (xchg(&b->lock, 1) == 1) {
  }

  b->cnt = 0;
  vprintfmt((void *)printfputch, b, fmt, ap);

  int rc = b->cnt;
  xchg(&b->lock, 0);

  return rc;
}

int printf(const char *fmt, ...) {
  va_list ap;
  int rc;

  va_start(ap, fmt);
  rc = vprintf(fmt, ap);
  va_end(ap);

  return rc;
}

void fflush() {
  printfbuf_t *b = &printfb;

  while (xchg(&b->lock, 1) == 1) {
  }

  write(STDOUT_FILENO, b->buf, b->buf_p - b->buf);
  b->buf_p = b->buf;

  xchg(&b->lock, 0);
}

char getch() {
  getchbuf_t *b = &getchb;

  while (xchg(&b->lock, 1) == 1) {
  }

  if (b->st == b->en) {
    b->st = b->en = b->buf;
    b->en += read(STDIN_FILENO, b->buf, sizeof(b->buf));
  }

  char rc = b->st == b->en ? -1 : *b->st++;

  xchg(&b->lock, 0);

  return rc;
}

char getchar() {
  char rc = 0xff;
  while ((u8)(rc = getch()) == 0xff) {
  }
  return rc;
}
