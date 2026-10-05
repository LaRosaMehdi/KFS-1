#ifndef KLIB_H
# define KLIB_H

# include "types.h"

/*
** Kernel mini libc: same signatures and behavior as the standard libc,
** since the kernel is built without it (-nostdlib -fno-builtin).
*/

/* Length of `s`, not counting the final '\0'. */
size_t  strlen(const char *s);
/* Compares `a` and `b`: <0, 0 or >0 like strcmp(3). */
int     strcmp(const char *a, const char *b);
/* Like strcmp, on at most `n` characters. */
int     strncmp(const char *a, const char *b, size_t n);
/* Fills `n` bytes of `dst` with `(unsigned char)c`. */
void    *memset(void *dst, int c, size_t n);
/* Copies `n` bytes; the areas must not overlap. */
void    *memcpy(void *dst, const void *src, size_t n);
/* Copies `n` bytes; handles overlap (used for scrolling). */
void    *memmove(void *dst, const void *src, size_t n);

#endif
