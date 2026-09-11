#ifndef TTY_H
# define TTY_H

# include "types.h"
# include "vga.h"

# define TTY_COUNT 4

void tty_init(void);
void tty_switch(size_t index);
void tty_set_color(uint8_t fg, uint8_t bg);
void tty_putchar(char c);
void tty_write(const char *s);
void tty_backspace(void);

#endif
