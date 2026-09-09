#ifndef VGA_H
# define VGA_H

# include "types.h"

# define VGA_WIDTH  80
# define VGA_HEIGHT 25
# define VGA_MEMORY 0xB8000

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

void    vga_init(void);
void    vga_clear(void);
void    vga_putchar(char c);
void    vga_write(const char *s);

#endif
