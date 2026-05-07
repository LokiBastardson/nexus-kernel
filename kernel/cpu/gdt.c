#include <types.h>
#include <cpu/gdt.h>
#include <lib/string.h>

#define GDT_ENTRIES 6

static gdt_entry_t gdt_entries[GDT_ENTRIES];
static gdt_ptr_t   gdt_ptr;
static tss_entry_t tss;

static void gdt_set_gate(int num, uint32_t base, uint32_t limit,
                         uint8_t access, uint8_t gran)
{
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;
    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt_entries[num].access      = access;
}

static void tss_init(uint32_t ss0, uint32_t esp0)
{
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = base + sizeof(tss_entry_t);

    gdt_set_gate(5, base, limit, 0xE9, 0x00);

    memset(&tss, 0, sizeof(tss_entry_t));
    tss.ss0 = ss0;
    tss.esp0 = esp0;
    tss.cs = 0x08 | 0x3;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x10 | 0x3;
    tss.iomap_base = sizeof(tss_entry_t);
}

void gdt_init(void)
{
    gdt_ptr.limit = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    gdt_set_gate(0, 0, 0,          0,    0);    /* null segment */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* kernel code */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* kernel data */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* user code */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* user data */

    tss_init(0x10, 0);

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}

void tss_set_kernel_stack(uint32_t esp0)
{
    tss.esp0 = esp0;
}
