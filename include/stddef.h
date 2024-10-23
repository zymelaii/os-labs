#pragma once

#include <stdint.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef i32 ssize_t;
typedef u32 size_t;

typedef i32 intptr_t;
typedef u32 uintptr_t;

typedef i32 ptrdiff_t;

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
#define ROUNDDOWN(a, n)         \
  ({                            \
    u32 __a = (u32)(a);         \
    (typeof(a))(__a - __a % n); \
  })

//! round value up to nearest multiple of n
#define ROUNDUP(a, n)                                \
  ({                                                 \
    u32 __n = (u32)(n);                              \
    (typeof(a))(ROUNDDOWN((u32)(a) + __n - 1, __n)); \
  })

//! get length of fixed size array
#define lengthof(a) (sizeof(a) / sizeof(a[0]))
