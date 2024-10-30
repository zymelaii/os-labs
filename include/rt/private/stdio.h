#pragma once

#include <stdarg.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int snprintf(char *str, int size, const char *fmt, ...);
int vsnprintf(char *str, int size, const char *fmt, va_list ap);

int printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list ap);

char getch();
void fflush();
