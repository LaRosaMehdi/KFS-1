#ifndef IDT_H
# define IDT_H

# include "types.h"

# define IDT_SIZE      256
# define ISR_COUNT     32
# define IRQ_COUNT     16
# define IRQ_BASE      32
# define IDT_KERNEL_CS 0x08
# define IDT_GATE_INT  0x8E

struct idt_entry
{
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct registers
{
    uint32_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t int_no;
    uint32_t err_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;
};

void idt_init(void);
void irq_handler(struct registers *regs);

#endif
