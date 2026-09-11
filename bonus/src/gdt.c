/* GDT plate : null, code 0x08, data 0x10. */

#include "gdt.h"

extern void gdt_flush(uint32_t ptr);

static struct gdt_entry g_gdt[3];
static struct gdt_ptr g_gp;

static void gdt_set_gate(int num, uint32_t base, uint32_t limit,
        uint8_t access, uint8_t gran)
{
    g_gdt[num].base_low = (uint16_t)(base & 0xFFFF);
    g_gdt[num].base_mid = (uint8_t)((base >> 16) & 0xFF);
    g_gdt[num].base_high = (uint8_t)((base >> 24) & 0xFF);
    g_gdt[num].limit_low = (uint16_t)(limit & 0xFFFF);
    g_gdt[num].granularity = (uint8_t)(((limit >> 16) & 0x0F) | (gran & 0xF0));
    g_gdt[num].access = access;
}

void gdt_init(void)
{
    g_gp.limit = (uint16_t)(sizeof(g_gdt) - 1);
    g_gp.base = (uint32_t)&g_gdt;
    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_flush((uint32_t)&g_gp);
}
