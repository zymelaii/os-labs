#pragma once

#define VMEM_ADDR 0xb8000

//! terminal size in default text mode
#define TERMINAL_COLUMN 80
#define TERMINAL_ROW 25
#define TERMINAL_SIZE ((TERMINAL_ROW) * (TERMINAL_COLUMN))

#define TERMINAL_POS(row, column) ((u16)(row) * TERMINAL_COLUMN + (column))

//! color code for vga text mode char attribute
#define BLACK 0x0
#define BLUE 0x1
#define GREEN 0x2
#define CYAN 0x3
#define RED 0x4
#define FUCHUSIA 0x5
#define BROWN 0x6
#define SILVER 0x7
#define GRAY 0x8
#define LIGHT_BLUE 0x9
#define LIGHT_GREEN 0xa
#define LIGHT_CYAN 0xb
#define LIGHT_RED 0xc
#define LIGHT_FUCHSIA 0xd
#define YELLOW 0xe
#define WHITE 0xf

#define FOREGROUND(color) ((u16)(color & 0xf) << 8)
#define BACKGROUND(color) ((u16)(color & 0xf) << 12)

#define DEFAULT_COLOR FOREGROUND(WHITE) | BACKGROUND(BLACK)
