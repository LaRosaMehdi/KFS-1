#ifndef IDT_H
# define IDT_H

# include "types.h"

/*
** IDT (Interrupt Descriptor Table): for each vector 0-255, the function the
** CPU calls. Vectors 0-31 = CPU exceptions, 32-47 = PIC IRQs.
*/

# define IDT_SIZE      256
# define ISR_COUNT     32
# define IRQ_COUNT     16
# define IRQ_BASE      32
# define IDT_KERNEL_CS 0x08    /* GDT code segment */
# define IDT_GATE_INT  0x8E    /* present, ring 0, 32-bit interrupt gate */

/* One 8-byte entry: handler address, segment and type. */
struct idt_entry
{
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

/* Operand of the lidt instruction: size - 1 and table address. */
struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* CPU state at interrupt time, as pushed by cpu.s and then by the CPU.
** The field order must match the stack order. */
struct registers
{
    uint32_t ds;            /* pushed by INT_STUB */
    uint32_t edi;           /* pusha */
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t int_no;        /* pushed by the ISR/IRQ stub */
    uint32_t err_code;
    uint32_t eip;           /* pushed by the CPU */
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;
};

/* Fills in the 32 exceptions and 16 IRQs, then loads the table (lidt). */
void idt_init(void);
/* Called by cpu.s on an exception: dumps the kernel stack, prints the cause
** and halts the CPU. */
void isr_handler(struct registers *regs);
/* Called by cpu.s on an IRQ: handles it, then acknowledges the PIC. */
void irq_handler(struct registers *regs);

#endif
