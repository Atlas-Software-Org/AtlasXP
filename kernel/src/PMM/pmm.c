#include "pmm.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <VMM/vmm.h>

#define PAGE_SIZE 0x1000

typedef struct {
    uint8_t* bitmap;
    uintptr_t base;
    size_t total_pages;
    size_t free_pages;
} Pmm;

static Pmm pmm;

static inline void bitmap_set(size_t bit) {
    pmm.bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void bitmap_clear(size_t bit) {
    pmm.bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline bool bitmap_test(size_t bit) {
    return pmm.bitmap[bit / 8] & (1 << (bit % 8));
}

int KiPmmInit(uintptr_t memory_base, size_t memory_size, uintptr_t bitmap_virt) {
    size_t page_count = memory_size / PAGE_SIZE;
    size_t bitmap_size = (page_count + 7) / 8;

    pmm.bitmap = (uint8_t*)bitmap_virt;
    pmm.base = memory_base;
    pmm.total_pages = page_count;
    pmm.free_pages = page_count;

    for (size_t i = 0; i < bitmap_size; i++) {
        pmm.bitmap[i] = 0;
    }

    for (size_t i = 0; i < bitmap_size / PAGE_SIZE + 1; i++) {
        uintptr_t addr = bitmap_virt + (i * PAGE_SIZE);
        KiMMap((void*)addr, (void*)addr, MMAP_PRESENT | MMAP_RW | MMAP_PMM_RESERVED_MEMORY);
    }

    for (size_t i = 0; i < page_count; i++) {
        uintptr_t addr = memory_base + (i * PAGE_SIZE);
        KiMMap((void*)addr, (void*)addr, MMAP_PRESENT | MMAP_RW | MMAP_PMM_HEAP_MEMORY);
    }

    return 0;
}

void* KiPmmAlloc() {
    for (size_t i = 0; i < pmm.total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            pmm.free_pages--;
            return (void*)(pmm.base + (i * PAGE_SIZE));
        }
    }
    return NULL;
}

void KiPmmFree(void* ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    if (addr < pmm.base) return;

    size_t index = (addr - pmm.base) / PAGE_SIZE;
    if (index >= pmm.total_pages) return;

    if (bitmap_test(index)) {
        bitmap_clear(index);
        pmm.free_pages++;
    }
}
