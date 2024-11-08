#pragma once

#include <stddef.h>
#include <sys/types.h>

size_t get_ticks();
pid_t get_pid();

int exec(const char* path);
pid_t fork();
__attribute__((noreturn)) void exit(int exit_code);

void fork_ack();
