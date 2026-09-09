#include "klib.h"

size_t  strlen(const char *s)
{
    size_t  i;

    i = 0;
    while (s[i])
        i++;
    return (i);
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b)
    {
        a++;
        b++;
    }
    return ((unsigned char)*a - (unsigned char)*b);
}

int strncmp(const char *a, const char *b, size_t n)
{
    if (n == 0)
        return (0);
    while (--n && *a && *a == *b)
    {
        a++;
        b++;
    }
    return ((unsigned char)*a - (unsigned char)*b);
}

void    *memset(void *dst, int c, size_t n)
{
    unsigned char   *p;

    p = (unsigned char *)dst;
    while (n--)
        *p++ = (unsigned char)c;
    return (dst);
}

void    *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char       *d;
    const unsigned char *s;

    d = (unsigned char *)dst;
    s = (const unsigned char *)src;
    while (n--)
        *d++ = *s++;
    return (dst);
}

void    *memmove(void *dst, const void *src, size_t n)
{
    unsigned char       *d;
    const unsigned char *s;

    d = (unsigned char *)dst;
    s = (const unsigned char *)src;
    if (d == s || n == 0)
        return (dst);
    if (d < s)
        return (memcpy(dst, src, n));
    d += n;
    s += n;
    while (n--)
        *--d = *--s;
    return (dst);
}
