#pragma once

#include <stddef.h>

void *memset(void *s, int c, size_t count);
void *memcpy(void *dst, const void *src, size_t count);

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t count);

int strcmp(const char *cs, const char *ct);
int strncmp(const char *cs, const char *ct, size_t count);

char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t count);

char *strcat(char *dst, const char *src);
