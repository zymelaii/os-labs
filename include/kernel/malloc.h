#pragma once

#include <stddef.h>
#include <sys/types.h>

phyaddr_t phy_malloc_4k();
void phy_free_4k(phyaddr_t phy_addr);

void kfree(void *ptr);
void *kmalloc(size_t size);
