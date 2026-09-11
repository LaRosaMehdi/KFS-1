/* Init GDT/IDT/PIC/TTY/clavier, puis 4 consoles. */

#include "types.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "tty.h"
#include "printk.h"

static void setup_tty(size_t n)
{
    tty_switch(n);
    tty_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printk("tty %u    F1-F4 pour changer d'ecran\n", (unsigned)(n + 1));
    if (n == 0)
    {
        tty_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        printk("42\n");
        tty_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    }
}

void main(uint32_t magic, void *mb_info)
{
    size_t i;

    (void)magic;
    (void)mb_info;
    gdt_init();
    idt_init();
    pic_init();
    tty_init();
    keyboard_init();
    i = 0;
    while (i < TTY_COUNT)
    {
        setup_tty(i);
        i++;
    }
    tty_switch(0);
    __asm__ volatile ("sti");
    while (1)
        __asm__ volatile ("hlt");
}
