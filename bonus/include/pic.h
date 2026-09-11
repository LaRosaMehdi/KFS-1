#ifndef PIC_H
# define PIC_H

# include "types.h"

void pic_init(void);
void pic_eoi(uint8_t irq);

#endif
