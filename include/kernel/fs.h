#pragma once

#include <stddef.h>
#include <stdint.h>

void init_fs(u32 partition_lba);

int read(int fd, void *buf, size_t count);
int write(int fd, void *buf, size_t count);

void read_file(const char *filename, void *dst);
