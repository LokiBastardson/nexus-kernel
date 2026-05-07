#ifndef _KERNEL_MEMORY_VMM_H
#define _KERNEL_MEMORY_VMM_H

#include <types.h>

#define PAGE_PRESENT   0x001
#define PAGE_WRITABLE  0x002
#define PAGE_USER      0x004
#define PAGE_ACCESSED  0x020
#define PAGE_DIRTY     0x040

typedef uint32_t page_dir_entry_t;
typedef uint32_t page_table_entry_t;

typedef struct page_directory {
    page_dir_entry_t entries[1024];
    uint32_t         tables_physical[1024];
    uint32_t         physical_addr;
} ALIGNED(PAGE_SIZE) page_directory_t;

void              vmm_init(void);
page_directory_t *vmm_get_kernel_directory(void);
page_directory_t *vmm_create_address_space(void);
void              vmm_destroy_address_space(page_directory_t *dir);
void              vmm_switch_directory(page_directory_t *dir);
void              vmm_map_page(page_directory_t *dir, uint32_t virt, uint32_t phys, uint32_t flags);
void              vmm_unmap_page(page_directory_t *dir, uint32_t virt);
uint32_t          vmm_get_physical(page_directory_t *dir, uint32_t virt);

#endif
