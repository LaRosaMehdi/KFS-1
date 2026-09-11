/* Cellules VGA et curseur materiel (CRT 0x3D4/0x3D5). */

#include "vga.h"
#include "io.h"

uint8_t vga_entry_color(uint8_t fg, uint8_t bg)
{
    return (fg | (bg << 4));
}

uint16_t vga_entry(char c, uint8_t color)
{
    return ((uint16_t)(unsigned char)c | ((uint16_t)color << 8));
}

uint16_t *vga_buffer(void)
{
    return ((uint16_t *)VGA_MEMORY);
}

void vga_cursor_enable(void)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | 14);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);
}

void vga_cursor_move(size_t row, size_t col)
{
    uint16_t pos;

    pos = (uint16_t)(col * VGA_WIDTH + row);
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}
