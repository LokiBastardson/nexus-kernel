#ifndef _KERNEL_MEMORY_KHEAP_H
#define _KERNEL_MEMORY_KHEAP_H

#include <types.h>

#define KHEAP_START 0xD0000000
#define KHEAP_SIZE  (4 * 1024 * 1024)

typedef struct heap_block {
    uint32_t           size;
    bool               free;
    struct heap_block  *next;
    struct heap_block  *prev;
} heap_block_t;

void  kheap_init(void);
void *kmalloc(size_t size);
void *kmalloc_aligned(size_t size, size_t alignment);
void  kfree(void *ptr);
void *krealloc(void *ptr, size_t new_size);

#endif
