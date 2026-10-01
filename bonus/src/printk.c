/*
** printk: %c %s %d %i %u %x %X %p %%, plus a zero-padded width on the
** unsigned formats (%08x).
** Variadic arguments go through gcc builtins (__builtin_va_*), since
** <stdarg.h> is not available without libc.
*/

#include "printk.h"
#include "tty.h"
#include "types.h"

static void print_str(const char *s)
{
    if (!s)
        s = "(null)";
    tty_write(s);
}

/* Writes `n` in base `base` (10 or 16), uppercase digits if `upper`,
** padded with leading zeros up to `min_digits` digits. */
static void print_uint(uint32_t n, unsigned base, int upper, int min_digits)
{
    char buf[32];
    const char *digits;
    int i;

    digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    i = 0;
    if (n == 0)
        buf[i++] = '0';
    while (n)
    {
        buf[i++] = digits[n % base];
        n /= base;
    }
    while (i < min_digits && i < (int)sizeof(buf))
        buf[i++] = '0';
    while (i--)
        tty_putchar(buf[i]);
}

/* Writes a signed integer; -(n + 1) + 1 avoids overflow on INT32_MIN. */
static void print_int(int32_t n)
{
    uint32_t un;

    if (n < 0)
    {
        tty_putchar('-');
        un = (uint32_t)(-(n + 1)) + 1;
    }
    else
        un = (uint32_t)n;
    print_uint(un, 10, 0, 0);
}

void printk(const char *fmt, ...)
{
    __builtin_va_list ap;
    size_t i;
    int min_digits;

    __builtin_va_start(ap, fmt);
    i = 0;
    while (fmt[i])
    {
        if (fmt[i] != '%')
        {
            tty_putchar(fmt[i++]);
            continue ;
        }
        i++;
        min_digits = 0;
        while (fmt[i] >= '0' && fmt[i] <= '9')
            min_digits = min_digits * 10 + (fmt[i++] - '0');
        if (fmt[i] == '%')
            tty_putchar('%');
        else if (fmt[i] == 'c')
            tty_putchar((char)__builtin_va_arg(ap, int));
        else if (fmt[i] == 's')
            print_str(__builtin_va_arg(ap, const char *));
        else if (fmt[i] == 'd' || fmt[i] == 'i')
            print_int(__builtin_va_arg(ap, int));
        else if (fmt[i] == 'u')
            print_uint(__builtin_va_arg(ap, uint32_t), 10, 0, min_digits);
        else if (fmt[i] == 'x')
            print_uint(__builtin_va_arg(ap, uint32_t), 16, 0, min_digits);
        else if (fmt[i] == 'X')
            print_uint(__builtin_va_arg(ap, uint32_t), 16, 1, min_digits);
        else if (fmt[i] == 'p')
        {
            print_str("0x");
            print_uint(__builtin_va_arg(ap, uint32_t), 16, 0, min_digits);
        }
        else
        {
            /* Unknown format: print it as is. */
            tty_putchar('%');
            if (fmt[i])
                tty_putchar(fmt[i]);
        }
        if (fmt[i])
            i++;
    }
    __builtin_va_end(ap);
}

extern uint32_t stack_top[];            /* boot.s */

void print_k_stack(void)
{
    uint32_t *stack_pointer;
    uint32_t *word;
    size_t words_on_line;

    __asm__ volatile ("mov %%esp, %0" : "=r"(stack_pointer));
    printk("kernel stack: esp=%p top=%p (%u bytes used)\n", stack_pointer,
        stack_top, (uint32_t)stack_top - (uint32_t)stack_pointer);
    word = stack_pointer;
    words_on_line = 0;
    while (word < stack_top)
    {
        if (words_on_line == 0)
            printk("%08p:", word);
        printk(" %08x", *word++);
        if (++words_on_line == 4 || word == stack_top)
        {
            printk("\n");
            words_on_line = 0;
        }
    }
}
