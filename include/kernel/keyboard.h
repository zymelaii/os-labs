#pragma once

#include <stdint.h>

void init_keyboard();

void kb_put_key(u8 ch);
u8 kb_get_key();
