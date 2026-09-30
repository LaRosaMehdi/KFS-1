/*
** 8259 PIC: IRQs remapped to 32, only the keyboard (IRQ1) is unmasked.
** By default IRQs 0-7 land on vectors 8-15, which are reserved for CPU
** exceptions: they have to be moved.
*/

#include "pic.h"
#include "io.h"

# define PIC1_CMD  0x20
# define PIC1_DATA 0x21
# define PIC2_CMD  0xA0
# define PIC2_DATA 0xA1
# define PIC_EOI   0x20

void pic_init(void)
{
    outb(PIC1_CMD, 0x11);       /* ICW1: start init, ICW4 will follow */
    outb(PIC2_CMD, 0x11);
    outb(PIC1_DATA, 0x20);      /* ICW2: master -> vectors 32-39 */
    outb(PIC2_DATA, 0x28);      /*       slave -> vectors 40-47 */
    outb(PIC1_DATA, 0x04);      /* ICW3: slave wired to master IRQ2 */
    outb(PIC2_DATA, 0x02);      /*       slave identity: 2 */
    outb(PIC1_DATA, 0x01);      /* ICW4: 8086 mode */
    outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, 0xFD);      /* masks: block everything but IRQ1 (keyboard) */
    outb(PIC2_DATA, 0xFF);
}

void pic_eoi(uint8_t irq)
{
    /* A slave IRQ also goes through the master: notify both. */
    if (irq >= 8)
        outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
}
