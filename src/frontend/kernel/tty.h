#ifndef TTY_H
#define TTY_H

#include "core.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

enum vga_color {
    VGA_COLOR_BLACK,
	VGA_COLOR_BLUE,
	VGA_COLOR_GREEN,
	VGA_COLOR_CYAN,
	VGA_COLOR_RED,
	VGA_COLOR_MAGENTA,
	VGA_COLOR_BROWN,
	VGA_COLOR_LIGHT_GREY,
	VGA_COLOR_DARK_GREY,
	VGA_COLOR_LIGHT_BLUE,
	VGA_COLOR_LIGHT_GREEN,
	VGA_COLOR_LIGHT_CYAN,
	VGA_COLOR_LIGHT_RED,
	VGA_COLOR_LIGHT_MAGENTA,
	VGA_COLOR_LIGHT_BROWN,
	VGA_COLOR_WHITE
};

static inline uint8_t
vga_entry_color(enum vga_color fg, enum vga_color bg)
{
    return fg | (bg << 4);
}

static inline uint16_t
vga_entry(char uc, uint8_t color)
{
    return (uint16_t) uc | (uint16_t) (color << 8);
}

void init_tty(void);
void tty_putc_at(char c, uint8_t color, size_t x, size_t y);
void tty_putc(char c);
void tty_write(const char *data, size_t size);
void tty_puts(const char *data);
void tty_clear(void);

#endif
