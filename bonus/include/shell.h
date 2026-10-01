#ifndef SHELL_H
# define SHELL_H

/*
** Minimal shell: each console has its own input line, run on Enter.
** Commands: help, stack, gdt, clear, halt, reboot, panic.
*/

/* Prints the prompt on the active console. */
void shell_prompt(void);
/* Feeds one typed character to the active console's input line: inserts it
** at the cursor, handles backspace, and runs the line on '\n'. */
void shell_handle_char(char c);
/* Moves the cursor `column_delta` characters (-1 or 1) inside the active
** console's input line. */
void shell_move_cursor(int column_delta);

#endif
