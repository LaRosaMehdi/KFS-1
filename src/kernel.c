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
    for (;;)
        __asm__ volatile ("hlt");
}
