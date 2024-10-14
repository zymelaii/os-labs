#pragma once

#include <macro_helper.h>
#include <stddef.h>

void busy_delay(u32 times);

void srand(size_t seed);
size_t rand();

__attribute__((noreturn)) void _panic(const char* file, const char* func,
                                      const char* line, const char* msg);

#define panic(msg) _panic(__FILE__, __FUNCTION__, MH_STRINGIFY(__LINE__), msg)

#define unreachable() panic("unreachable")
#define todo(msg) panic("todo: " msg)

#define UNIMPLEMENTED_IMPL0() panic("not implemented")
#define UNIMPLEMENTED_IMPL1(target) panic("not implemented: " target)
#define UNIMPLEMENTED_IMPL(N, ...) \
  MH_EXPAND(MH_CONCAT(UNIMPLEMENTED_IMPL, N)(__VA_ARGS__))
#define unimplemented(...) \
  MH_EXPAND(UNIMPLEMENTED_IMPL(MH_EXPAND(MH_NARG(__VA_ARGS__)), __VA_ARGS__))
