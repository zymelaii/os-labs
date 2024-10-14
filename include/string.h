#pragma once

#include <stddef.h>

void *memset(void *s, int c, size_t count);
void *memcpy(void *dst, const void *src, size_t count);
size_t strlen(const char *s);
int strcmp(const char *cs, const char *ct);
