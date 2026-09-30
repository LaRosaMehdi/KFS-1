/*
** Virtual consoles. Each console keeps a full copy of the screen (`buffer`);
** writes go there, then tty_sync copies the active console into VGA memory,
** draws the tab bar and moves the cursor.
*/

#include "tty.h"
#include "klib.h"

# define TTY_TOP 1       /* first text row: row 0 is the tab bar */
# define TTY_HISTORY 100  /* rows kept after scrolling off the screen */

struct tty
{
    uint16_t buffer[VGA_SIZE];  /* screen contents of this console */
    size_t top;                 /* first editable row (protected rows above) */
    size_t row;                 /* cursor position */
    size_t col;
    uint8_t color;              /* color of the next characters */
    uint16_t history[TTY_HISTORY * VGA_WIDTH];  /* scrolled-off rows, oldest first */
    size_t history_rows;
    size_t scroll_back;         /* rows the view is scrolled up, 0 = live */
};

static struct tty g_ttys[TTY_COUNT];
static size_t g_current;

/* Row 0: " 1  2  3  4 " on blue, active console in black on cyan. */
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

/* Shows the active console on screen. When scrolled back, the rows below
** `top` show history first, then the start of the live buffer. */
static void tty_sync(void)
{
    struct tty *t;
    uint16_t *screen;
    size_t screen_row;
    size_t line;

    t = &g_ttys[g_current];
    screen = vga_buffer();
    memcpy(screen, t->buffer, t->top * VGA_WIDTH * sizeof(uint16_t));
    screen_row = t->top;
    while (screen_row < VGA_HEIGHT)
    {
        line = t->history_rows - t->scroll_back + (screen_row - t->top);
        if (line < t->history_rows)
            memcpy(screen + screen_row * VGA_WIDTH, t->history + line * VGA_WIDTH,
                VGA_WIDTH * sizeof(uint16_t));
        else
            memcpy(screen + screen_row * VGA_WIDTH,
                t->buffer + (t->top + line - t->history_rows) * VGA_WIDTH,
                VGA_WIDTH * sizeof(uint16_t));
        screen_row++;
    }
    tty_draw_tabs();
    /* Pushed below the screen when scrolled back: the VGA hides it. */
    vga_cursor_move(t->row + t->scroll_back, t->col);
}

/* Moves everything below `top` up one row and clears the last row.
** The row pushed out goes to the history. */
static void tty_scroll(struct tty *t)
{
    size_t col;

    /* Shifts the whole history per scroll: a ring buffer would avoid it. */
    if (t->history_rows == TTY_HISTORY)
    {
        memmove(t->history, t->history + VGA_WIDTH,
            (TTY_HISTORY - 1) * VGA_WIDTH * sizeof(uint16_t));
        t->history_rows--;
    }
    memcpy(t->history + t->history_rows * VGA_WIDTH, t->buffer + t->top * VGA_WIDTH,
        VGA_WIDTH * sizeof(uint16_t));
    t->history_rows++;

    memmove(t->buffer + t->top * VGA_WIDTH, t->buffer + (t->top + 1) * VGA_WIDTH,
        (VGA_HEIGHT - t->top - 1) * VGA_WIDTH * sizeof(uint16_t));
    col = 0;
    while (col < VGA_WIDTH)
    {
        t->buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = vga_entry(' ', t->color);
        col++;
    }
    t->row = VGA_HEIGHT - 1;
    t->col = 0;
}

/* Writes a printable character, wrapping at the end of the row. */
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
        g_ttys[i].top = TTY_TOP;
        g_ttys[i].row = TTY_TOP;
        g_ttys[i].col = 0;
        g_ttys[i].color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        g_ttys[i].history_rows = 0;
        g_ttys[i].scroll_back = 0;
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

void tty_lock_lines(void)
{
    struct tty *t;

    t = &g_ttys[g_current];
    if (t->col != 0)
        tty_putchar('\n');
    /* Always keep at least one editable row. */
    if (t->row < VGA_HEIGHT - 1)
        t->top = t->row;
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
        if (t->row == t->top)
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
    t->scroll_back = 0;
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

void tty_move_cursor(int row_delta, int col_delta)
{
    struct tty *t;
    int new_row;
    int new_col;

    t = &g_ttys[g_current];
    new_row = (int)t->row + row_delta;
    new_col = (int)t->col + col_delta;
    if (new_row < (int)t->top || new_row >= VGA_HEIGHT
        || new_col < 0 || new_col >= VGA_WIDTH)
        return ;
    t->row = (size_t)new_row;
    t->col = (size_t)new_col;
    t->scroll_back = 0;
    tty_sync();
}

void tty_scroll_view(int rows)
{
    struct tty *t;
    int new_scroll_back;

    t = &g_ttys[g_current];
    new_scroll_back = (int)t->scroll_back + rows;
    if (new_scroll_back < 0)
        new_scroll_back = 0;
    if (new_scroll_back > (int)t->history_rows)
        new_scroll_back = (int)t->history_rows;
    t->scroll_back = (size_t)new_scroll_back;
    tty_sync();
}
