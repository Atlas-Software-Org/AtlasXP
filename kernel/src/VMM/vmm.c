#include "vmm.h"
#include <stdbool.h>

extern int IsPmmEnabled;

uint8_t temporary_memory[65536] = {0};
uint8_t bitmap[2048];

static void bitmap_set(size_t bit) {
    bitmap[bit >> 3] |= 1 << (bit & 7);
}

static void bitmap_clear(size_t bit) {
    bitmap[bit >> 3] &= ~(1 << (bit & 7));
}

static bool bitmap_test(size_t bit) {
    return bitmap[bit >> 3] & (1 << (bit & 7));
}

uint64_t StaticVmmMemAlloc() {
	for (int i = 0; i < 16; i++) {
		if (bitmap_test(i) == false) {
			bitmap_set(i);
			return (uint64_t)&temporary_memory[i*0x1000];
		}
	}
}

uint64_t temp_new_page = 0;
static inline PageTable* KiGetOrCreateTable(uint64_t entry, bool* created) {
    if (!(entry & PAGE_PRESENT)) {
    	if (IsPmmEnabled == 0) {
    		temp_new_page = StaticVmmMemAlloc();
    	} else {
    		temp_new_page = (uint64_t)KiPmmAlloc();
    	}
        if (created) *created = true;
        return (PageTable*)PHYS_TO_VIRT(temp_new_page);
    }
    return (PageTable*)PHYS_TO_VIRT(entry & PAGE_ADDR_MASK);
}

void KiMMap(void* vaddr, void* paddr, uint64_t attributes) {
    uint64_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    PageTable* pml4 = (PageTable*)PHYS_TO_VIRT(cr3 & PAGE_ADDR_MASK);

    uint64_t vaddrU64 = (uint64_t)vaddr;
    uint64_t paddrU64 = (uint64_t)paddr;

    uint16_t pml4_index = (vaddrU64 >> 39) & 0x1FF;
    uint16_t pdpt_index = (vaddrU64 >> 30) & 0x1FF;
    uint16_t pd_index   = (vaddrU64 >> 21) & 0x1FF;
    uint16_t pt_index   = (vaddrU64 >> 12) & 0x1FF;

    bool created = false;

    PageTable* pdpt = KiGetOrCreateTable(pml4->Entries[pml4_index], &created);
    if (created) pml4->Entries[pml4_index] = VIRT_TO_PHYS(pdpt) | PAGE_PRESENT | PAGE_RW;

    PageTable* pd = KiGetOrCreateTable(pdpt->Entries[pdpt_index], &created);
    if (created) pdpt->Entries[pdpt_index] = VIRT_TO_PHYS(pd) | PAGE_PRESENT | PAGE_RW;

    PageTable* pt = KiGetOrCreateTable(pd->Entries[pd_index], &created);
    if (created) pd->Entries[pd_index] = VIRT_TO_PHYS(pt) | PAGE_PRESENT | PAGE_RW;

    pt->Entries[pt_index] = (paddrU64 & PAGE_ADDR_MASK) | (attributes & 0xFFF0000000000FFFULL) | PAGE_PRESENT | PAGE_RW;

    asm volatile ("invlpg (%0)" :: "r" (vaddr) : "memory");
}

void KiUMap(void* virt_addr) {
    uint64_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    PageTable* pml4 = (PageTable*)PHYS_TO_VIRT(cr3 & PAGE_ADDR_MASK);

    uint64_t vaddr = (uint64_t)virt_addr;

    uint16_t pml4_index = (vaddr >> 39) & 0x1FF;
    uint16_t pdpt_index = (vaddr >> 30) & 0x1FF;
    uint16_t pd_index   = (vaddr >> 21) & 0x1FF;
    uint16_t pt_index   = (vaddr >> 12) & 0x1FF;

    PageTable* pdpt = (PageTable*)PHYS_TO_VIRT(pml4->Entries[pml4_index] & PAGE_ADDR_MASK);
    if (!(pml4->Entries[pml4_index] & PAGE_PRESENT)) return;

    PageTable* pd = (PageTable*)PHYS_TO_VIRT(pdpt->Entries[pdpt_index] & PAGE_ADDR_MASK);
    if (!(pdpt->Entries[pdpt_index] & PAGE_PRESENT)) return;

    PageTable* pt = (PageTable*)PHYS_TO_VIRT(pd->Entries[pd_index] & PAGE_ADDR_MASK);
    if (!(pd->Entries[pd_index] & PAGE_PRESENT)) return;

    pt->Entries[pt_index] &= ~PAGE_PRESENT;

    asm volatile ("invlpg (%0)" :: "r" (virt_addr) : "memory");
}

int KiIsMapped(void* phys_addr) {
    uint64_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    PageTable* pml4 = (PageTable*)PHYS_TO_VIRT(cr3 & PAGE_ADDR_MASK);

    uint64_t paddr = (uint64_t)phys_addr;

    for (uint64_t pml4_i = 0; pml4_i < 512; pml4_i++) {
        if (!(pml4->Entries[pml4_i] & PAGE_PRESENT)) continue;

        PageTable* pdpt = (PageTable*)PHYS_TO_VIRT(pml4->Entries[pml4_i] & PAGE_ADDR_MASK);
        for (uint64_t pdpt_i = 0; pdpt_i < 512; pdpt_i++) {
            if (!(pdpt->Entries[pdpt_i] & PAGE_PRESENT)) continue;

            PageTable* pd = (PageTable*)PHYS_TO_VIRT(pdpt->Entries[pdpt_i] & PAGE_ADDR_MASK);
            for (uint64_t pd_i = 0; pd_i < 512; pd_i++) {
                if (!(pd->Entries[pd_i] & PAGE_PRESENT)) continue;

                PageTable* pt = (PageTable*)PHYS_TO_VIRT(pd->Entries[pd_i] & PAGE_ADDR_MASK);
                for (uint64_t pt_i = 0; pt_i < 512; pt_i++) {
                    if (!(pt->Entries[pt_i] & PAGE_PRESENT)) continue;

                    uint64_t mapped = pt->Entries[pt_i] & PAGE_ADDR_MASK;
                    if (mapped == (paddr & PAGE_ADDR_MASK)) return 1;
                }
            }
        }
    }

    return 0;
}
