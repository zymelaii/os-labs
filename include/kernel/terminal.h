#pragma once

#include <stdint.h>

void term_write_char(u16 pos, u16 char_code);
void term_write_str(u16 pos, const char *str, u8 char_attr);
void term_clear();
