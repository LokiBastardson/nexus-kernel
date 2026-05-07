#include <types.h>
#include <memory/kheap.h>
#include <memory/pmm.h>
#include <memory/vmm.h>
#include <lib/string.h>
#include <lib/stdio.h>

#define HEAP_BLOCK_HEADER_SIZE sizeof(heap_block_t)
#define HEAP_MIN_BLOCK_SIZE    16
#define HEAP_MAGIC             0xDEADBEEF

static uint8_t     heap_area[KHEAP_SIZE] ALIGNED(PAGE_SIZE);
static heap_block_t *heap_start;
static bool         heap_initialized = false;

void kheap_init(void)
{
    heap_start = (heap_block_t *)heap_area;
    heap_start->size = KHEAP_SIZE - HEAP_BLOCK_HEADER_SIZE;
    heap_start->free = true;
    heap_start->next = NULL;
    heap_start->prev = NULL;

    heap_initialized = true;
    kprintf("[HEAP] Initialized: %u bytes at %x\n", KHEAP_SIZE, (uint32_t)heap_area);
}

static void split_block(heap_block_t *block, size_t size)
{
    if (block->size >= size + HEAP_BLOCK_HEADER_SIZE + HEAP_MIN_BLOCK_SIZE) {
        heap_block_t *new_block = (heap_block_t *)((uint8_t *)block +
                                   HEAP_BLOCK_HEADER_SIZE + size);
        new_block->size = block->size - size - HEAP_BLOCK_HEADER_SIZE;
        new_block->free = true;
        new_block->next = block->next;
        new_block->prev = block;

        if (block->next)
            block->next->prev = new_block;

        block->next = new_block;
        block->size = size;
    }
}

static void coalesce_block(heap_block_t *block)
{
    if (block->next && block->next->free) {
        block->size += HEAP_BLOCK_HEADER_SIZE + block->next->size;
        block->next = block->next->next;
        if (block->next)
            block->next->prev = block;
    }

    if (block->prev && block->prev->free) {
        block->prev->size += HEAP_BLOCK_HEADER_SIZE + block->size;
        block->prev->next = block->next;
        if (block->next)
            block->next->prev = block->prev;
    }
}

void *kmalloc(size_t size)
{
    if (!heap_initialized || size == 0)
        return NULL;

    size = (size + 7) & ~7;

    heap_block_t *current = heap_start;
    while (current) {
        if (current->free && current->size >= size) {
            split_block(current, size);
            current->free = false;
            return (void *)((uint8_t *)current + HEAP_BLOCK_HEADER_SIZE);
        }
        current = current->next;
    }

    kprintf("[HEAP] ERROR: Out of kernel heap memory (requested %u)\n", size);
    return NULL;
}

void *kmalloc_aligned(size_t size, size_t alignment)
{
    void *ptr = kmalloc(size + alignment);
    if (!ptr)
        return NULL;

    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t aligned = (addr + alignment - 1) & ~(alignment - 1);

    return (void *)aligned;
}

void kfree(void *ptr)
{
    if (!ptr)
        return;

    heap_block_t *block = (heap_block_t *)((uint8_t *)ptr - HEAP_BLOCK_HEADER_SIZE);
    block->free = true;
    coalesce_block(block);
}

void *krealloc(void *ptr, size_t new_size)
{
    if (!ptr)
        return kmalloc(new_size);

    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }

    heap_block_t *block = (heap_block_t *)((uint8_t *)ptr - HEAP_BLOCK_HEADER_SIZE);
    if (block->size >= new_size)
        return ptr;

    void *new_ptr = kmalloc(new_size);
    if (!new_ptr)
        return NULL;

    memcpy(new_ptr, ptr, block->size);
    kfree(ptr);
    return new_ptr;
}
