#pragma once

#include <stddef.h>  // IWYU pragma: keep
#include <sys/types.h>

#define NUM_4B 0x00000004
#define NUM_1K 0x00000400
#define NUM_4K 0x00001000
#define NUM_1M 0x00100000
#define NUM_4M 0x00400000
#define NUM_1G 0x40000000

#define KERNEL_LIN_BASE ((uintptr_t)(3u * NUM_1G))

#define K_PHY2LIN(x) ((void *)((phyaddr_t)(x) + (KERNEL_LIN_BASE)))
#define K_LIN2PHY(x) ((phyaddr_t)((void *)(x) - (KERNEL_LIN_BASE)))
