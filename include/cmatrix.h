#pragma once

#include <stddef.h>

struct colored_char {
  u8 fg_color;  //<! foreground color
  u8 bg_color;  //<! background color
  u8 code;      //<! acsii code of the char
  u16 pos;      //<! display pos in the ternimal
};

void cmatrix();
