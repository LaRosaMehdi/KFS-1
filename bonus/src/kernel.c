/*
** Bonus C entry point, called by _start (src/boot.s).
** Sets up GDT/IDT/PIC/TTY/keyboard, then the 4 consoles and their shell
** prompt, then waits for keys: everything else happens in keyboard interrupts.
*/

#include "types.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "tty.h"
#include "printk.h"
#include "shell.h"

/* Writes the protected header of console `n` (plus "42" on the first one),
** then the shell prompt. */
static void setup_tty(size_t n)
{
    tty_switch(n);
    tty_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printk("tty %u    F1-F4 to switch screens\n", (unsigned)(n + 1));
    if (n == 0)
    {
        tty_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        printk("42\n");
    }
    tty_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    tty_lock_lines();
    shell_prompt();
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
    /* Tables are ready: interrupts can be enabled. */
    __asm__ volatile ("sti");
    while (1)
        __asm__ volatile ("hlt");
}
