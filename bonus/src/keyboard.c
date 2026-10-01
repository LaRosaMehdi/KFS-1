/*
** PS/2 keyboard: Shift, F1-F4 (switch console), PageUp/Down scroll the TTY,
** characters and left/right arrows go to the shell.
** Scancode < 0x80 = key pressed, scancode | 0x80 = key released.
*/

#include "keyboard.h"
#include "io.h"
#include "shell.h"
#include "tty.h"

# define KBD_DATA    0x60    /* port to read scancodes from */
# define KBD_STATUS  0x64    /* bit 0 = a byte is waiting on KBD_DATA */
# define KBD_LSHIFT  0x2A
# define KBD_RSHIFT  0x36
# define KBD_F1      0x3B
# define KBD_F4      0x3E
# define KBD_RELEASE 0x80
# define KBD_LEFT    0x4B    /* arrows and PageUp/Down come after a 0xE0 */
# define KBD_RIGHT   0x4D    /* prefix, skipped since it has bit 7 set */
# define KBD_PGUP    0x49
# define KBD_PGDOWN  0x51
# define KBD_PAGE    (VGA_HEIGHT / 2)  /* rows scrolled per PageUp/Down */

/* Scancode -> character (0 = ignored key), without then with Shift. */
static const char g_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0
};

static const char g_map_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0
};

static int g_shift;     /* 1 while a Shift key is held */

void keyboard_init(void)
{
    g_shift = 0;
    while (inb(KBD_STATUS) & 1)
        inb(KBD_DATA);
}

void keyboard_handler(void)
{
    uint8_t sc;
    char c;

    sc = inb(KBD_DATA);
    if (sc == KBD_LSHIFT || sc == KBD_RSHIFT)
    {
        g_shift = 1;
        return ;
    }
    if (sc == (KBD_LSHIFT | KBD_RELEASE) || sc == (KBD_RSHIFT | KBD_RELEASE))
    {
        g_shift = 0;
        return ;
    }
    if (sc & KBD_RELEASE)
        return ;
    if (sc >= KBD_F1 && sc <= KBD_F4)
    {
        tty_switch((size_t)(sc - KBD_F1));
        return ;
    }
    if (sc == KBD_LEFT)
        shell_move_cursor(-1);
    else if (sc == KBD_RIGHT)
        shell_move_cursor(1);
    else if (sc == KBD_PGUP)
        tty_scroll_view(KBD_PAGE);
    else if (sc == KBD_PGDOWN)
        tty_scroll_view(-KBD_PAGE);
    else
    {
        c = g_shift ? g_map_shift[sc] : g_map[sc];
        if (c)
            shell_handle_char(c);
    }
}
