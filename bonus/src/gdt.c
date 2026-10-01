/*
** Flat GDT stored at physical address 0x00000800: null, then code, data and
** stack for the kernel (0x08, 0x10, 0x18) and for user mode (0x20, 0x28,
** 0x30). Every segment covers all memory (base 0, limit 4 GiB), so
** addressing is plain linear memory; only the privilege level differs.
** GRUB already provides a GDT, but we cannot know where it lives or keep
** it: the kernel installs its own.
*/

#include "gdt.h"

extern void gdt_flush(uint32_t ptr);     /* cpu.s */

static struct gdt_entry *const g_gdt = (struct gdt_entry *)GDT_ADDRESS;
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
    g_gp.limit = (uint16_t)(GDT_ENTRY_COUNT * sizeof(struct gdt_entry) - 1);
    g_gp.base = GDT_ADDRESS;
    gdt_set_gate(0, 0, 0, 0, 0);                    /* mandatory null descriptor */
    /* access 0x9A / 0xFA: present, ring 0 / 3, executable and readable code.
    ** access 0x92 / 0xF2: present, ring 0 / 3, readable and writable data
    ** (stacks are plain data segments).
    ** gran 0xCF: limit counted in 4 KiB pages, 32-bit segment. */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);     /* kernel code */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);     /* kernel data */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0x92, 0xCF);     /* kernel stack */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xFA, 0xCF);     /* user code */
    gdt_set_gate(5, 0, 0xFFFFFFFF, 0xF2, 0xCF);     /* user data */
    gdt_set_gate(6, 0, 0xFFFFFFFF, 0xF2, 0xCF);     /* user stack */
    gdt_flush((uint32_t)&g_gp);
}
