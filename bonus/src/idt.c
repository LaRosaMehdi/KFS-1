/* IDT : 32 exceptions + 16 IRQ, stubs ASM dans cpu.s. */

#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "printk.h"
#include "tty.h"

extern void idt_flush(uint32_t ptr);
extern void (*isr_stubs[ISR_COUNT])(void);
extern void (*irq_stubs[IRQ_COUNT])(void);

static struct idt_entry g_idt[IDT_SIZE];
static struct idt_ptr g_ip;

static void idt_set_gate(uint8_t num, void (*handler)(void))
{
    uint32_t base;

    base = (uint32_t)handler;
    g_idt[num].base_low = (uint16_t)(base & 0xFFFF);
    g_idt[num].base_high = (uint16_t)((base >> 16) & 0xFFFF);
    g_idt[num].selector = IDT_KERNEL_CS;
    g_idt[num].zero = 0;
    g_idt[num].flags = IDT_GATE_INT;
}

void isr_handler(struct registers *regs)
{
    tty_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    printk("\nexception %u err=0x%x eip=0x%x\n",
        regs->int_no, regs->err_code, regs->eip);
    while (1)
        __asm__ volatile ("cli; hlt");
}

void irq_handler(struct registers *regs)
{
    uint8_t irq;

    irq = (uint8_t)(regs->int_no - IRQ_BASE);
    if (irq == 1)
        keyboard_handler();
    pic_eoi(irq);
}

void idt_init(void)
{
    size_t i;

    g_ip.limit = (uint16_t)(sizeof(g_idt) - 1);
    g_ip.base = (uint32_t)&g_idt;
    i = 0;
    while (i < ISR_COUNT)
    {
        idt_set_gate((uint8_t)i, isr_stubs[i]);
        i++;
    }
    i = 0;
    while (i < IRQ_COUNT)
    {
        idt_set_gate((uint8_t)(IRQ_BASE + i), irq_stubs[i]);
        i++;
    }
    idt_flush((uint32_t)&g_ip);
}
