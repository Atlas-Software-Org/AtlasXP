#include "pmm.h"
#include <VMM/vmm.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PAGE_SIZE 0x1000

typedef struct {
    uint8_t* bitmap;
    size_t bitmap_size;
    uintptr_t base;
    size_t total_pages;
    size_t free_pages;
} Pmm;

static Pmm pmm;

int IsPmmEnabled = 0;

static void bitmap_set(size_t bit) {
    pmm.bitmap[bit >> 3] |= 1 << (bit & 7);
}

static void bitmap_clear(size_t bit) {
    pmm.bitmap[bit >> 3] &= ~(1 << (bit & 7));
}

static bool bitmap_test(size_t bit) {
    return pmm.bitmap[bit >> 3] & (1 << (bit & 7));
}

void KiPmmInit(uint64_t mem_base, uint64_t mem_size) {
	pmm.bitmap = mem_base + mem_size - (mem_size / 8);
	pmm.bitmap_size = mem_size / 8;
	pmm.base = mem_base;
	pmm.total_pages = (mem_size - pmm.bitmap_size) / PAGE_SIZE;
	pmm.free_pages = pmm.total_pages;

	for (uint64_t i = mem_base; i < mem_base+mem_size; i+=0x1000) {
		KiMMap((void*)mem_base, (void*)mem_base, MMAP_PRESENT | MMAP_RW | MMAP_PMM_HEAP_MEMORY);
	}

	IsPmmEnabled = 1;
}

void* KiPmmAlloc(void) {
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
