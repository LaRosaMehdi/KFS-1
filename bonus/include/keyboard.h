#ifndef KEYBOARD_H
# define KEYBOARD_H

/*
** PS/2 keyboard (scancode set 1, US QWERTY layout), read on interrupt
** (IRQ1).
*/

/* Drains the controller buffer to start from a clean state. */
void keyboard_init(void);
/* Called on IRQ1: reads a scancode, handles Shift, F1-F4 and PageUp/Down,
** sends characters and left/right arrows to the shell. */
void keyboard_handler(void);

#endif
