#include "pmm.h"
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

static inline void bitmap_set(size_t bit) {
    pmm.bitmap[bit >> 3] |= 1 << (bit & 7);
}

static inline void bitmap_clear(size_t bit) {
    pmm.bitmap[bit >> 3] &= ~(1 << (bit & 7));
}

static inline bool bitmap_test(size_t bit) {
    return pmm.bitmap[bit >> 3] & (1 << (bit & 7));
}

void KiPmmInit(uintptr_t mem_base, size_t mem_size) {
	printk("KiPmmInit(%p, %p);\n\r", mem_base, mem_size);
    size_t bitmap_size = mem_size / 8;
    uintptr_t bitmap_addr = mem_base + mem_size - bitmap_size;
    size_t usable_size = bitmap_addr - mem_base;
    size_t total_pages = usable_size / PAGE_SIZE;

    pmm.bitmap = (uint8_t*)bitmap_addr;
    pmm.bitmap_size = bitmap_size;
    pmm.base = mem_base;
    pmm.total_pages = total_pages;
    pmm.free_pages = total_pages;

    for (size_t i = 0; i < (bitmap_size); i++) {
        pmm.bitmap[i] = 0;
    }

    IsPmmEnabled = 1;
}

void* KiPmmAlloc(void) {
    if (!IsPmmEnabled) return NULL;
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
    if (!IsPmmEnabled) return;
    uintptr_t addr = (uintptr_t)ptr;
    if (addr < pmm.base) return;

    size_t index = (addr - pmm.base) / PAGE_SIZE;
    if (index >= pmm.total_pages) return;

    if (bitmap_test(index)) {
        bitmap_clear(index);
        pmm.free_pages++;
    }
}
