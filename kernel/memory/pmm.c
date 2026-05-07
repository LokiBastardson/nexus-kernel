#include <types.h>
#include <memory/pmm.h>
#include <lib/string.h>
#include <lib/stdio.h>

#define BITMAP_SIZE (131072 / 8)

static uint8_t  frame_bitmap[BITMAP_SIZE];
static uint32_t total_frames;
static uint32_t used_frames;

static inline void bitmap_set(uint32_t bit)
{
    frame_bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void bitmap_clear(uint32_t bit)
{
    frame_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline bool bitmap_test(uint32_t bit)
{
    return (frame_bitmap[bit / 8] >> (bit % 8)) & 1;
}

static uint32_t bitmap_find_free(void)
{
    for (uint32_t i = 0; i < total_frames / 8; i++) {
        if (frame_bitmap[i] != 0xFF) {
            for (uint32_t j = 0; j < 8; j++) {
                if (!(frame_bitmap[i] & (1 << j)))
                    return i * 8 + j;
            }
        }
    }
    return (uint32_t)-1;
}

void pmm_init(multiboot_info_t *mbi)
{
    memset(frame_bitmap, 0xFF, sizeof(frame_bitmap));

    total_frames = 0;
    used_frames = 0;

    if (!(mbi->flags & 0x20)) {
        uint32_t mem_kb = mbi->mem_upper + 1024;
        total_frames = mem_kb / 4;
        if (total_frames > BITMAP_SIZE * 8)
            total_frames = BITMAP_SIZE * 8;

        memset(frame_bitmap, 0xFF, sizeof(frame_bitmap));
        used_frames = total_frames;
        kprintf("[PMM] Memory: %u KB, %u frames (no mmap)\n", mem_kb, total_frames);
        return;
    }

    uint32_t mem_end = 0;
    multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t *)mbi->mmap_addr;
    uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

    while ((uint32_t)mmap < mmap_end) {
        uint32_t region_end = (uint32_t)(mmap->addr + mmap->len);
        if (region_end > mem_end)
            mem_end = region_end;
        mmap = (multiboot_mmap_entry_t *)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }

    total_frames = mem_end / PAGE_SIZE;
    if (total_frames > BITMAP_SIZE * 8)
        total_frames = BITMAP_SIZE * 8;

    memset(frame_bitmap, 0xFF, sizeof(frame_bitmap));
    used_frames = total_frames;

    mmap = (multiboot_mmap_entry_t *)mbi->mmap_addr;
    while ((uint32_t)mmap < mmap_end) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint32_t addr = (uint32_t)mmap->addr;
            uint32_t len  = (uint32_t)mmap->len;

            uint32_t frame_start = (addr + PAGE_SIZE - 1) / PAGE_SIZE;
            uint32_t frame_end = (addr + len) / PAGE_SIZE;

            for (uint32_t f = frame_start; f < frame_end && f < total_frames; f++) {
                bitmap_clear(f);
                used_frames--;
            }
        }
        mmap = (multiboot_mmap_entry_t *)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }

    /* Reserve first 1 MB (frames 0-255) for BIOS/hardware */
    for (uint32_t i = 0; i < 256; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            used_frames++;
        }
    }

    kprintf("[PMM] Total: %u frames, Used: %u, Free: %u\n",
            total_frames, used_frames, total_frames - used_frames);
}

uint32_t pmm_alloc_frame(void)
{
    uint32_t frame = bitmap_find_free();
    if (frame == (uint32_t)-1) {
        kprintf("[PMM] ERROR: Out of physical memory!\n");
        return 0;
    }
    bitmap_set(frame);
    used_frames++;
    return frame * PAGE_SIZE;
}

void pmm_free_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr / PAGE_SIZE;
    if (frame < total_frames && bitmap_test(frame)) {
        bitmap_clear(frame);
        used_frames--;
    }
}

void pmm_mark_used(uint32_t frame_addr)
{
    uint32_t frame = frame_addr / PAGE_SIZE;
    if (frame < total_frames && !bitmap_test(frame)) {
        bitmap_set(frame);
        used_frames++;
    }
}

void pmm_mark_free(uint32_t frame_addr)
{
    pmm_free_frame(frame_addr);
}

uint32_t pmm_get_total_frames(void)
{
    return total_frames;
}

uint32_t pmm_get_used_frames(void)
{
    return used_frames;
}

uint32_t pmm_get_free_frames(void)
{
    return total_frames - used_frames;
}
