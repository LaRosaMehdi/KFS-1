#ifndef PRINTK_H
# define PRINTK_H

/*
** printk: simplified printf to the active console.
** Formats: %c %s %d %i %u %x %X %p %%, no width or precision.
*/
void printk(const char *fmt, ...);

#endif
