#include "paging.h"

#define PAGE_PRESENT 0x1
#define PAGE_WRITE   0x2
#define PAGE_USER    0x4
#define PAGE_PSE     0x80

typedef uint64_t pte_t;

static pte_t pml4[512] __attribute__((aligned(4096)));
static pte_t pdpt[512] __attribute__((aligned(4096)));
static pte_t pd[512] __attribute__((aligned(4096)));

pte_t* KiPml4Init() {
    for (int i = 0; i < 512; i++) {
        pml4[i] = 0;
        pdpt[i] = 0;
        pd[i] = 0;
    }

    // Map 1GB physical memory into HHDM region
    // HHDM base address top 16 bits: 0xFFFF8000_00000000
    // PML4 index for 0xFFFF800000000000
    const uint64_t hhdm_base = 0xFFFF800000000000ULL;
    int pml4_index = (hhdm_base >> 39) & 0x1FF;  // should be 511 or 510 depending on your offset
    int pdpt_index = (hhdm_base >> 30) & 0x1FF;

    // Map 1GB with 2MB pages: 512 entries in PD
    for (int i = 0; i < 512; i++) {
        uint64_t phys_addr = i * 0x200000ULL;  // 2MB increments
        pd[i] = phys_addr | PAGE_PRESENT | PAGE_WRITE | PAGE_PSE;
    }

    pdpt[pdpt_index] = ((uint64_t)pd) | PAGE_PRESENT | PAGE_WRITE;
    pml4[pml4_index] = ((uint64_t)pdpt) | PAGE_PRESENT | PAGE_WRITE;

    return pml4;
}
