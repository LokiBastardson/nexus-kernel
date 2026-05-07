#ifndef _KERNEL_MEMORY_PMM_H
#define _KERNEL_MEMORY_PMM_H

#include <types.h>
#include <boot/multiboot.h>

void     pmm_init(multiboot_info_t *mbi);
uint32_t pmm_alloc_frame(void);
void     pmm_free_frame(uint32_t frame_addr);
void     pmm_mark_used(uint32_t frame_addr);
void     pmm_mark_free(uint32_t frame_addr);
uint32_t pmm_get_total_frames(void);
uint32_t pmm_get_used_frames(void);
uint32_t pmm_get_free_frames(void);

#endif
