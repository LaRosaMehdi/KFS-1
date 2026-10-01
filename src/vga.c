/* Direct writes to VGA text memory + hardware cursor. */

#include "vga.h"
#include "klib.h"
#include "io.h"

/* Software cursor position and current color. */
static uint16_t *const  g_vga = (uint16_t *)VGA_MEMORY;
static size_t           g_row;
static size_t           g_col;
static uint8_t          g_color;

static uint8_t  vga_entry_color(uint8_t fg, uint8_t bg)
{
    return (fg | (bg << 4));
}

static uint16_t vga_entry(char c, uint8_t color)
{
    return ((uint16_t)(unsigned char)c | ((uint16_t)color << 8));
}

/* Moves the hardware cursor: CRT registers 0x0F (position low byte) and
** 0x0E (high byte), selected through port 0x3D4, written through 0x3D5. */
static void vga_update_cursor(void)
{
    uint16_t    position;

    position = g_row * VGA_WIDTH + g_col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, position & 0xFF);
    outb(0x3D4, 0x0E);
    outb(0x3D5, position >> 8);
}

void    vga_clear(void)
{
    size_t  i;

    i = 0;
    while (i < VGA_WIDTH * VGA_HEIGHT)
    {
        g_vga[i] = vga_entry(' ', g_color);
        i++;
    }
    g_row = 0;
    g_col = 0;
    vga_update_cursor();
}

void    vga_init(void)
{
    g_row = 0;
    g_col = 0;
    g_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
}

void    vga_putchar(char c)
{
    size_t  index;

    if (c == '\n')
    {
        g_col = 0;
        if (g_row + 1 < VGA_HEIGHT)
            g_row++;
    }
    else if (c == '\r')
        g_col = 0;
    else
    {
        index = g_row * VGA_WIDTH + g_col;
        g_vga[index] = vga_entry(c, g_color);
        g_col++;
        if (g_col >= VGA_WIDTH)
        {
            g_col = 0;
            if (g_row + 1 < VGA_HEIGHT)
                g_row++;
        }
    }
    vga_update_cursor();
}

void    vga_write(const char *s)
{
    size_t  i;
    size_t  n;

    n = strlen(s);
    i = 0;
    while (i < n)
    {
        vga_putchar(s[i]);
        i++;
    }
}
