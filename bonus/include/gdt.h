#ifndef GDT_H
# define GDT_H

# include "types.h"

/*
** GDT (Global Descriptor Table): describes memory segments to the CPU in
** protected mode. Layouts are fixed by the hardware, hence `packed`.
*/

/* The table lives at a fixed physical address, in free low memory. */
# define GDT_ADDRESS 0x00000800
# define GDT_ENTRY_COUNT 7

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

/* Builds the flat GDT at GDT_ADDRESS and reloads the segment registers. */
void gdt_init(void);
/* Prints every entry as read back from GDT_ADDRESS. */
void gdt_print(void);

#endif
