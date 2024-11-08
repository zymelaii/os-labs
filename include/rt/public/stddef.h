#pragma once

#include <stdint.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef int32_t ssize_t;
typedef uint32_t size_t;

typedef int32_t intptr_t;
typedef uint32_t uintptr_t;

typedef int32_t ptrdiff_t;

#define MIN(_a, _b)         \
  ({                        \
    typeof(_a) __a = (_a);  \
    typeof(_b) __b = (_b);  \
    __a <= __b ? __a : __b; \
  })

#define MAX(_a, _b)         \
  ({                        \
    typeof(_a) __a = (_a);  \
    typeof(_b) __b = (_b);  \
    __a >= __b ? __a : __b; \
  })

#define CLAMP(x, lo, hi) MIN(MAX(lo, x), hi)

//! round value down to nearest multiple of n
#define ROUNDDOWN(a, n)           \
  ({                              \
    uint32_t __a = (uint32_t)(a); \
    (typeof(a))(__a - __a % n);   \
  })

//! round value up to nearest multiple of n
#define ROUNDUP(a, n)                                     \
  ({                                                      \
    uint32_t __n = (uint32_t)(n);                         \
    (typeof(a))(ROUNDDOWN((uint32_t)(a) + __n - 1, __n)); \
  })

//! get length of fixed size array
#define lengthof(a) (sizeof(a) / sizeof(a[0]))
