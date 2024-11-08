#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct vpinfo_args {
  const char *color_csi;
  const char *slevel;
  const char *file;
  const char *func;
  int line;
} vpinfo_args_t;

static void vpinfo(vpinfo_args_t *args, const char *fmt, va_list ap) {
  printf("\e[0m%s%s:%s:%d: %s: ", args->color_csi, args->file, args->func,
         args->line, args->slevel);
  vprintf(fmt, ap);
  printf("\e[0m\n");
  fflush();
}

void _panic(const char *file, const char *func, int line, const char *fmt,
            ...) {
  vpinfo_args_t args = {
      .color_csi = "\e[91m",
      .slevel = "fatal",
      .file = file,
      .func = func,
      .line = line,
  };

  va_list ap;
  va_start(ap, fmt);
  vpinfo(&args, fmt, ap);
  va_end(ap);

  while (true) {
  }
}

void _warn(const char *file, const char *func, int line, const char *fmt, ...) {
  vpinfo_args_t args = {
      .color_csi = "\e[93m",
      .slevel = "warn",
      .file = file,
      .func = func,
      .line = line,
  };

  va_list ap;
  va_start(ap, fmt);
  vpinfo(&args, fmt, ap);
  va_end(ap);
}
