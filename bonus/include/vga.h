/*
** VGA text screen (bonus): low-level primitives only. Display logic
** (scrolling, colors, software cursor) lives in tty.c.
** 16-bit cell: low byte = character, high byte = background << 4 | foreground.
*/

#ifndef VGA_H
# define VGA_H

# include "types.h"

# define VGA_WIDTH  80
# define VGA_HEIGHT 25
# define VGA_SIZE   (VGA_WIDTH * VGA_HEIGHT)
# define VGA_MEMORY 0xB8000

/* The 16 VGA palette colors, in hardware order. */
enum e_vga_color
{
    VGA_COLOR_BLACK = 0,
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
    VGA_COLOR_YELLOW,
    VGA_COLOR_WHITE
};

/* Color byte: `fg` text on `bg` background. */
uint8_t vga_entry_color(uint8_t fg, uint8_t bg);
/* VGA cell: character `c` in color `color`. */
uint16_t vga_entry(char c, uint8_t color);
/* Address of video memory (VGA_SIZE cells). */
uint16_t *vga_buffer(void);
/* Enables the hardware cursor as an underline (scanlines 14 to 15). */
void vga_cursor_enable(void);
/* Moves the hardware cursor to row `row`, column `col`. */
void vga_cursor_move(size_t row, size_t col);

#endif
