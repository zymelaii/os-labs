#pragma once

#include <stdint.h>

typedef u32 phyaddr_t;
typedef i32 pid_t;
typedef u32 clock_t;
typedef void* handle_t;

typedef void (*int_handler_t)();
typedef void (*irq_handler_t)();

typedef void (*task_handler_t)();

typedef u32 (*syscall_t)();
