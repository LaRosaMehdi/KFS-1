/*
** C entry point (mandatory part), called by _start in boot.s.
** `magic` and `mb_info` come from GRUB (eax/ebx), unused here.
*/

#include "types.h"
#include "vga.h"
#include "klib.h"

void    main(uint32_t magic, void *mb_info)
{
    (void)magic;
    (void)mb_info;
    vga_init();
    if (strcmp("42", "42") == 0)
        vga_write("42");
    /* Nothing else to do: sleep until the next interrupt. */
    for (;;)
        __asm__ volatile ("hlt");
}
