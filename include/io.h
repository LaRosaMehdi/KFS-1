/*
** x86 I/O port access (in/out instructions). Inline to avoid a function
** call on every hardware write.
*/

#ifndef IO_H
# define IO_H

# include "types.h"

/* Reads one byte from port `port`. */
static inline uint8_t inb(uint16_t port)
{
    uint8_t value;

    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return (value);
}

/* Writes the byte `value` to port `port`. */
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

#endif
