#ifndef VMM_H
#define VMM_H 1

#include <stdint.h>
#include <stdbool.h>
#include <PMM/pmm.h>

typedef struct {
    uint64_t Entries[512];
} PageTable;

#define PAGE_PRESENT     0x001
#define PAGE_RW          0x002
#define PAGE_USER        0x004
#define PAGE_PSE         0x080
#define PAGE_ADDR_MASK   0xFFFFFFFFF000ULL

extern uint64_t HhdmOffset;

#define PHYS_TO_VIRT(paddr) ((void*)(HhdmOffset + (paddr)))
#define VIRT_TO_PHYS(vaddr) ((uint64_t)(vaddr) - HhdmOffset)

#define MMAP_PRESENT    (1ULL << 0)
#define MMAP_RW         (1ULL << 1)
#define MMAP_USER       (1ULL << 2)
#define MMAP_PWT        (1ULL << 3)
#define MMAP_PCD        (1ULL << 4)
#define MMAP_ACCESSED   (1ULL << 5)
#define MMAP_DIRTY      (1ULL << 6)
#define MMAP_HUGE       (1ULL << 7)
#define MMAP_GLOBAL     (1ULL << 8)
#define MMAP_PMM_RESERVED_MEMORY (1ULL << 9)
#define MMAP_PMM_HEAP_MEMORY (1ULL << 10)
#define MMAP_SOFT2      (1ULL << 11)
/* --- Execute permissions (if NX supported) --- */
#define MMAP_NX         (1ULL << 63)  // No-execute

void KiMMap(void* virt_addr, void* phys_addr, uint64_t attributes);
void KiUMap(void* virt_addr);

int KiIsMapped(void* phys_addr);

#endif /* VMM_H */
