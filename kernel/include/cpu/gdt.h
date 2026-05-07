#ifndef _KERNEL_CPU_GDT_H
#define _KERNEL_CPU_GDT_H

#include <types.h>

typedef struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} PACKED gdt_entry_t;

typedef struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} PACKED gdt_ptr_t;

typedef struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} PACKED tss_entry_t;

void gdt_init(void);
void tss_set_kernel_stack(uint32_t esp0);

extern void gdt_flush(uint32_t gdt_ptr);
extern void tss_flush(void);

#endif
