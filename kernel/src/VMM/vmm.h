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

#define PHYS_TO_VIRT(paddr) ((void*)(0xFFFF800000000000ULL + (paddr)))  // Your kernel's mapping base
#define VIRT_TO_PHYS(vaddr) ((uint64_t)(vaddr) - 0xFFFF800000000000ULL) // If needed

// --- Basic access permissions ---
#define MMAP_PRESENT    (1ULL << 0)   // Page is present
#define MMAP_RW         (1ULL << 1)   // Read/write (1 = writeable)
#define MMAP_USER       (1ULL << 2)   // User-accessible page

// --- Caching ---
#define MMAP_PWT        (1ULL << 3)   // Page Write-Through
#define MMAP_PCD        (1ULL << 4)   // Page Cache Disable

// --- Access tracking ---
#define MMAP_ACCESSED   (1ULL << 5)   // Set by CPU when accessed
#define MMAP_DIRTY      (1ULL << 6)   // Set by CPU when written (PTE only)

// --- Page size ---
#define MMAP_HUGE       (1ULL << 7)   // Set on PDE for 2MiB/1GiB pages

// --- TLB behavior ---
#define MMAP_GLOBAL     (1ULL << 8)   // Global mapping (not flushed on CR3 switch)

// --- Software available bits ---
#define MMAP_SOFT0      (1ULL << 9)
#define MMAP_SOFT1      (1ULL << 10)
#define MMAP_SOFT2      (1ULL << 11)

// --- Execute permissions (if NX supported) ---
#define MMAP_NX         (1ULL << 63)  // No-execute

void KiMMap(void* virt_addr, void* phys_addr, uint64_t attributes);
void KiUMap(void* virt_addr);

#endif /* VMM_H */
