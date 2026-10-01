#ifndef PRINTK_H
# define PRINTK_H

/*
** printk: simplified printf to the active console.
** Formats: %c %s %d %i %u %x %X %p %%. A width zero-pads %u %x %X %p
** (%08x); no other width or precision.
*/
void printk(const char *fmt, ...);

/* Dumps the kernel stack to the active console, from the current stack
** pointer up to the top of the stack, four 32-bit words per line. */
void print_k_stack(void);

#endif
