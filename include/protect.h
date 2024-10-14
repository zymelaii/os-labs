#pragma once

#include <stddef.h>

#define GDT_SIZE 128

typedef struct descriptor {
  u16 limit_low;      //<! limit 1
  u16 base_low;       //<! base 1
  u8 base_mid;        //<! base 2
  u8 attr1;           //<! attr 1 - P(1) | DPL(2) | DT(1) | TYPE(4)
  u8 limit_high : 4;  //<! limit 2
  u8 attr2 : 4;       //<! attr 2 - G(1) D(1) 0(1) AVL(1)
  u8 base_high;       //<! base 3
} __attribute__((packed)) descriptor_t;

typedef struct selector {
  u16 rpl : 2;
  u16 ti : 1;
  u16 index : 13;
} __attribute__((packed)) selector_t;

typedef struct gdt_ptr {
  u16 limit;
  u32 base;
} __attribute__((packed)) gdt_ptr_t;
