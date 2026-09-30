#ifndef PIC_H
# define PIC_H

# include "types.h"

/* 8259 PIC (master + slave): routes hardware interrupts. */

/* Remaps IRQs 0-15 to vectors 32-47 and only lets the keyboard (IRQ1)
** through. */
void pic_init(void);
/* Signals that `irq` has been handled; otherwise the PIC sends no more. */
void pic_eoi(uint8_t irq);

#endif
