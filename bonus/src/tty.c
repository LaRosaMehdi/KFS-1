/* Quatre buffers TTY, scroll, couleurs, curseur. */

#include "tty.h"
#include "klib.h"

# define TTY_TOP 1

struct tty
{
    uint16_t buffer[VGA_SIZE];
    size_t row;
    size_t col;
    uint8_t color;
};

static struct tty g_ttys[TTY_COUNT];
static size_t g_current;

static void tty_draw_tabs(void)
{
    uint16_t *screen;
    uint8_t tab_color;
    size_t tty_index;
    size_t col;

    screen = vga_buffer();
    col = 0;
    while (col < VGA_WIDTH)
    {
        screen[col] = vga_entry(' ', vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLUE));
        col++;
    }
    tty_index = 0;
    while (tty_index < TTY_COUNT)
    {
        if (tty_index == g_current)
            tab_color = vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_CYAN);
        else
            tab_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLUE);
        col = tty_index * 3;
        screen[col] = vga_entry(' ', tab_color);
        screen[col + 1] = vga_entry((char)('1' + tty_index), tab_color);
        screen[col + 2] = vga_entry(' ', tab_color);
        tty_index++;
    }
}

static void tty_sync(void)
{
    memcpy(vga_buffer(), g_ttys[g_current].buffer, VGA_SIZE * sizeof(uint16_t));
    tty_draw_tabs();
    vga_cursor_move(g_ttys[g_current].row, g_ttys[g_current].col);
}

static void tty_scroll(struct tty *t)
{
    size_t col;

    memmove(t->buffer + TTY_TOP * VGA_WIDTH, t->buffer + (TTY_TOP + 1) * VGA_WIDTH,
        (VGA_HEIGHT - TTY_TOP - 1) * VGA_WIDTH * sizeof(uint16_t));
    col = 0;
    while (col < VGA_WIDTH)
    {
        t->buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = vga_entry(' ', t->color);
        col++;
    }
    t->row = VGA_HEIGHT - 1;
    t->col = 0;
}

static void tty_putc(struct tty *t, char c)
{
    if (t->col >= VGA_WIDTH)
    {
        t->col = 0;
        t->row++;
    }
    if (t->row >= VGA_HEIGHT)
        tty_scroll(t);
    t->buffer[t->row * VGA_WIDTH + t->col] = vga_entry(c, t->color);
    t->col++;
}

void tty_init(void)
{
    size_t i;
    size_t j;

    i = 0;
    while (i < TTY_COUNT)
    {
        g_ttys[i].row = TTY_TOP;
        g_ttys[i].col = 0;
        g_ttys[i].color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        j = 0;
        while (j < VGA_SIZE)
        {
            g_ttys[i].buffer[j] = vga_entry(' ', g_ttys[i].color);
            j++;
        }
        i++;
    }
    g_current = 0;
    vga_cursor_enable();
    tty_sync();
}

void tty_switch(size_t index)
{
    if (index >= TTY_COUNT || index == g_current)
        return ;
    g_current = index;
    tty_sync();
}

void tty_set_color(uint8_t fg, uint8_t bg)
{
    g_ttys[g_current].color = vga_entry_color(fg, bg);
}

void tty_backspace(void)
{
    struct tty *t;

    t = &g_ttys[g_current];
    if (t->col == 0)
    {
        if (t->row == TTY_TOP)
            return ;
        t->row--;
        t->col = VGA_WIDTH - 1;
    }
    else
        t->col--;
    t->buffer[t->row * VGA_WIDTH + t->col] = vga_entry(' ', t->color);
}

void tty_putchar(char c)
{
    struct tty *t;
    size_t pad;

    t = &g_ttys[g_current];
    if (c == '\n')
    {
        t->col = 0;
        t->row++;
        if (t->row >= VGA_HEIGHT)
            tty_scroll(t);
    }
    else if (c == '\r')
        t->col = 0;
    else if (c == '\t')
    {
        pad = 4 - (t->col % 4);
        while (pad--)
            tty_putc(t, ' ');
    }
    else if (c == '\b')
        tty_backspace();
    else
        tty_putc(t, c);
    tty_sync();
}

void tty_write(const char *s)
{
    size_t i;
    size_t n;

    n = strlen(s);
    i = 0;
    while (i < n)
    {
        tty_putchar(s[i]);
        i++;
    }
}
