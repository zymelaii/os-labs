#pragma once

#include <stdarg.h>

void printfmt(void (*putch)(int, void *), void *putdat, const char *fmt, ...);
void vprintfmt(void (*putch)(int, void *), void *putdat, const char *fmt,
               va_list ap);
int snprintf(char *str, int size, const char *fmt, ...);
int vsnprintf(char *str, int size, const char *fmt, va_list ap);

int kprintf(const char *fmt, ...);
int vkprintf(const char *fmt, va_list ap);

int cprintf(const char *fmt, ...);
int vcprintf(const char *fmt, va_list ap);

int kprintf(const char *fmt, ...);

char getch();
