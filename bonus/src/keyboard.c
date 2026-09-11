/* PS/2 : Shift, F1-F4, le reste va sur le TTY. */

#include "keyboard.h"
#include "io.h"
#include "tty.h"

# define KBD_DATA    0x60
# define KBD_STATUS  0x64
# define KBD_LSHIFT  0x2A
# define KBD_RSHIFT  0x36
# define KBD_F1      0x3B
# define KBD_F4      0x3E
# define KBD_RELEASE 0x80

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

static int g_shift;

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
        tty_switch((size_t)(sc - KBD_F1 + 1));
        return ;
    }
    c = g_shift ? g_map_shift[sc] : g_map[sc];
    if (c)
        tty_putchar(c);
}
