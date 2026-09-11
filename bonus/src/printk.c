/* printk : %c %s %d %i %u %x %X %p %% */

#include "printk.h"
#include "tty.h"
#include "types.h"

static void print_str(const char *s)
{
    if (!s)
        s = "(null)";
    tty_write(s);
}

static void print_uint(uint32_t n, unsigned base, int upper)
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
    while (i--)
        tty_putchar(buf[i]);
}

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
    print_uint(un, 10, 0);
}

void printk(const char *fmt, ...)
{
    __builtin_va_list ap;
    size_t i;

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
        if (fmt[i] == '%')
            tty_putchar('%');
        else if (fmt[i] == 'c')
            tty_putchar((char)__builtin_va_arg(ap, int));
        else if (fmt[i] == 's')
            print_str(__builtin_va_arg(ap, const char *));
        else if (fmt[i] == 'd' || fmt[i] == 'i')
            print_int(__builtin_va_arg(ap, int));
        else if (fmt[i] == 'u')
            print_uint(__builtin_va_arg(ap, uint32_t), 10, 0);
        else if (fmt[i] == 'x')
            print_uint(__builtin_va_arg(ap, uint32_t), 16, 0);
        else if (fmt[i] == 'X')
            print_uint(__builtin_va_arg(ap, uint32_t), 16, 1);
        else if (fmt[i] == 'p')
        {
            print_str("0x");
            print_uint(__builtin_va_arg(ap, uint32_t), 16, 0);
        }
        else
        {
            tty_putchar('%');
            if (fmt[i])
                tty_putchar(fmt[i]);
        }
        if (fmt[i])
            i++;
    }
    __builtin_va_end(ap);
}
