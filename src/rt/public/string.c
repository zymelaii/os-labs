#include <string.h>

void *memset(void *s, int c, size_t count) {
  char *xs = s;
  while (count--) {
    *xs++ = c;
  }
  return s;
}

void *memcpy(void *dst, const void *src, size_t count) {
  char *tmp = dst;
  const char *s = src;
  while (count--) {
    *tmp++ = *s++;
  }
  return dst;
}

size_t strlen(const char *s) {
  const char *sc = s;
  while (*sc != '\0') {
    ++sc;
  }
  return sc - s;
}

size_t strnlen(const char *s, size_t count) {
  const char *sc = s;
  while (count-- && *sc != '\0') {
    ++sc;
  }
  return sc - s;
}

int strcmp(const char *cs, const char *ct) {
  unsigned char c1, c2;
  while (1) {
    c1 = *cs++;
    c2 = *ct++;
    if (c1 != c2) {
      return c1 < c2 ? -1 : 1;
    }
    if (!c1) {
      break;
    }
  }
  return 0;
}

int strncmp(const char *cs, const char *ct, size_t count) {
  unsigned char c1, c2;
  while (count) {
    c1 = *cs++;
    c2 = *ct++;
    if (c1 != c2) {
      return c1 < c2 ? -1 : 1;
    }
    if (!c1) {
      break;
    }
    count--;
  }
  return 0;
}

char *strcpy(char *dst, const char *src) {
  char *tmp = dst;
  while ((*dst++ = *src++) != '\0') {
  }
  return tmp;
}

char *strncpy(char *dst, const char *src, size_t count) {
  char *tmp = dst;
  while (count) {
    if ((*tmp = *src) != 0) {
      src++;
    }
    tmp++;
    count--;
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  char *tmp = dst;
  while (*dst) {
    dst++;
  }
  while ((*dst++ = *src++) != '\0') {
  }
  return tmp;
}
