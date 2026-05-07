#include <types.h>
#include <memory/vmm.h>
#include <memory/pmm.h>
#include <lib/string.h>
#include <lib/stdio.h>

static page_directory_t *kernel_directory;
static page_directory_t *current_directory;

extern uint32_t _kernel_end;

static uint32_t kernel_end_addr;

static page_table_entry_t *get_or_create_table(page_directory_t *dir, uint32_t table_idx, uint32_t flags)
{
    if (dir->entries[table_idx] & PAGE_PRESENT) {
        return (page_table_entry_t *)(dir->entries[table_idx] & 0xFFFFF000);
    }

    uint32_t table_phys = pmm_alloc_frame();
    if (!table_phys)
        return NULL;

    dir->entries[table_idx] = table_phys | flags | PAGE_PRESENT | PAGE_WRITABLE;
    dir->tables_physical[table_idx] = table_phys;

    page_table_entry_t *table = (page_table_entry_t *)table_phys;
    memset(table, 0, PAGE_SIZE);

    return table;
}

void vmm_init(void)
{
    kernel_end_addr = (uint32_t)&_kernel_end;
    kernel_end_addr = (kernel_end_addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    uint32_t dir_phys = pmm_alloc_frame();
    kernel_directory = (page_directory_t *)dir_phys;
    memset(kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physical_addr = dir_phys;

    /* Identity-map the first 4 MB for kernel and hardware */
    for (uint32_t addr = 0; addr < 0x400000; addr += PAGE_SIZE) {
        vmm_map_page(kernel_directory, addr, addr, PAGE_PRESENT | PAGE_WRITABLE);
    }

    /* Map VGA memory */
    vmm_map_page(kernel_directory, 0xB8000, 0xB8000, PAGE_PRESENT | PAGE_WRITABLE);

    current_directory = kernel_directory;

    /* Enable paging */
    uint32_t cr3 = kernel_directory->physical_addr;
    __asm__ volatile("mov %0, %%cr3" : : "r"(cr3));

    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));

    kprintf("[VMM] Paging enabled, kernel mapped at 0x0-0x400000\n");
}

page_directory_t *vmm_get_kernel_directory(void)
{
    return kernel_directory;
}

page_directory_t *vmm_create_address_space(void)
{
    uint32_t dir_phys = pmm_alloc_frame();
    if (!dir_phys)
        return NULL;

    page_directory_t *dir = (page_directory_t *)dir_phys;
    memset(dir, 0, sizeof(page_directory_t));
    dir->physical_addr = dir_phys;

    /* Copy kernel space mappings (upper half) */
    for (int i = 0; i < 1024; i++) {
        if (kernel_directory->entries[i] & PAGE_PRESENT)
            dir->entries[i] = kernel_directory->entries[i];
    }

    return dir;
}

void vmm_destroy_address_space(page_directory_t *dir)
{
    if (!dir || dir == kernel_directory)
        return;

    for (int i = 0; i < 1024; i++) {
        if ((dir->entries[i] & PAGE_PRESENT) &&
            !(kernel_directory->entries[i] & PAGE_PRESENT)) {
            pmm_free_frame(dir->entries[i] & 0xFFFFF000);
        }
    }

    pmm_free_frame(dir->physical_addr);
}

void vmm_switch_directory(page_directory_t *dir)
{
    current_directory = dir;
    __asm__ volatile("mov %0, %%cr3" : : "r"(dir->physical_addr));
}

void vmm_map_page(page_directory_t *dir, uint32_t virt, uint32_t phys, uint32_t flags)
{
    uint32_t table_idx = virt >> 22;
    uint32_t page_idx  = (virt >> 12) & 0x3FF;

    page_table_entry_t *table = get_or_create_table(dir, table_idx, flags);
    if (table)
        table[page_idx] = (phys & 0xFFFFF000) | (flags & 0xFFF) | PAGE_PRESENT;
}

void vmm_unmap_page(page_directory_t *dir, uint32_t virt)
{
    uint32_t table_idx = virt >> 22;
    uint32_t page_idx  = (virt >> 12) & 0x3FF;

    if (!(dir->entries[table_idx] & PAGE_PRESENT))
        return;

    page_table_entry_t *table = (page_table_entry_t *)(dir->entries[table_idx] & 0xFFFFF000);
    table[page_idx] = 0;

    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

uint32_t vmm_get_physical(page_directory_t *dir, uint32_t virt)
{
    uint32_t table_idx = virt >> 22;
    uint32_t page_idx  = (virt >> 12) & 0x3FF;

    if (!(dir->entries[table_idx] & PAGE_PRESENT))
        return 0;

    page_table_entry_t *table = (page_table_entry_t *)(dir->entries[table_idx] & 0xFFFFF000);
    if (!(table[page_idx] & PAGE_PRESENT))
        return 0;

    return (table[page_idx] & 0xFFFFF000) | (virt & 0xFFF);
}
