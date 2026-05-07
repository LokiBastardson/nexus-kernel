#ifndef _KERNEL_BOOT_MULTIBOOT_H
#define _KERNEL_BOOT_MULTIBOOT_H

#include <types.h>

#define MULTIBOOT_MAGIC        0x1BADB002
#define MULTIBOOT_FLAGS        0x00000003
#define MULTIBOOT_CHECKSUM     -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

typedef struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
} PACKED multiboot_info_t;

typedef struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} PACKED multiboot_mmap_entry_t;

#define MULTIBOOT_MEMORY_AVAILABLE 1

#endif
