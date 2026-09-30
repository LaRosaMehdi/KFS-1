/*
** Virtual consoles: TTY_COUNT independent screens (F1-F4), each with its own
** text, cursor and color. Screen row 0 is a tab bar showing the active
** console. All functions act on the active console.
*/

#ifndef TTY_H
# define TTY_H

# include "types.h"
# include "vga.h"

# define TTY_COUNT 4

/* Clears the consoles, enables the cursor, shows console 0. */
void tty_init(void);
/* Makes console `index` (0 to TTY_COUNT - 1) active and redraws it. */
void tty_switch(size_t index);
/* Color of the next characters written. */
void tty_set_color(uint8_t fg, uint8_t bg);
/* Writes `c`; handles '\n', '\r', '\t' (every 4) and '\b'. Scrolls at the bottom. */
void tty_putchar(char c);
/* Writes the string `s`. */
void tty_write(const char *s);
/* Erases the character before the cursor, never going back into lines
** protected by tty_lock_lines. */
void tty_backspace(void);
/* Protects the lines written so far: they can't be erased or scrolled away. */
void tty_lock_lines(void);
/* Moves the cursor by `row_delta` rows and `col_delta` columns, staying
** inside the editable area. The next characters are written there. */
void tty_move_cursor(int row_delta, int col_delta);
/* Scrolls the view `rows` rows back into the history (negative = forward).
** Writing anything jumps back to the live screen. */
void tty_scroll_view(int rows);

#endif
