/*
** VGA text screen (mandatory part): 80x25 cells at 0xB8000.
** Each cell = 16 bits: low byte = ASCII character, high byte = color
** (4-bit background << 4 | 4-bit foreground).
*/

#ifndef VGA_H
# define VGA_H

# include "types.h"

# define VGA_WIDTH  80
# define VGA_HEIGHT 25
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

/* Grey on black, empty screen, cursor at the top left. */
void    vga_init(void);
/* Clears the screen and moves the cursor back to (0, 0). */
void    vga_clear(void);
/* Prints `c` at the cursor; handles '\n' and '\r'. No scrolling: the last
** line is overwritten once the screen is full. */
void    vga_putchar(char c);
/* Prints the string `s` one character at a time. */
void    vga_write(const char *s);

#endif
