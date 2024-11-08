#pragma once

#include <stddef.h>

int read(int fd, void *buf, size_t count);
int write(int fd, const void *buf, size_t count);
