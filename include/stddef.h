#pragma once

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef int i32;
typedef unsigned int u32;
typedef short i16;
typedef unsigned short u16;
typedef char i8;
typedef unsigned char u8;

typedef u32 size_t;
typedef i32 ssize_t;
typedef i32 ptrdiff_t;
typedef u32 phyaddr_t;

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

//! get offset of member in a struct
#define offsetof(type, member) ((size_t)(&((type *)0)->member))
