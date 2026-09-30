#ifndef GDT_H
# define GDT_H

# include "types.h"

/*
** GDT (Global Descriptor Table): describes memory segments to the CPU in
** protected mode. Layouts are fixed by the hardware, hence `packed`.
*/

/* One 8-byte segment descriptor (base and limit are split up). */
struct gdt_entry
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

/* Operand of the lgdt instruction: size - 1 and table address. */
struct gdt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* Loads a flat GDT (code and data over all 4 GiB) and reloads the segments. */
void gdt_init(void);

#endif
