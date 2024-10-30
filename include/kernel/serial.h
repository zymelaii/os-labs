#pragma once

#include <stdint.h>

#define PORT_COM1 0x3f8

int init_serial();

char serial_read();
void serial_write(u8 c);
