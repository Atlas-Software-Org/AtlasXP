/*
    Author: Adam Bassem
    Revision 0
    Patch 0
    Minor 0
    Major 0
    Atlas 0.0.7
*/

#include "paging.h"

__attribute__((aligned(0x1000))) PageTable __pml4_ins;

static PageTable *Pml4 = &__pml4_ins;
uint64_t HhdmBase = 0;

uint64_t KiRetrievePml4() {
    return (uint64_t)(uintptr_t)Pml4;
}

static int index = 0;

void d() {
    printk("Debug log index %d\n\r", index++);
}

void* phys_to_virt(uint64_t phys) {
    return (void*)(phys - 0 + HhdmBase);
}

bool MapPage(void* phys, void* virt) {
    const uint64_t physical_address = (uint64_t)phys;
    const uint64_t virtual_address = (uint64_t)virt;
    const int Flags = PAGE_PRESENT | PAGE_READWRITE | PAGE_USER;

    uint64_t Pml4Index = (virtual_address >> 39) & 0x1FF;
    uint64_t PdptIndex = (virtual_address >> 30) & 0x1FF;
    uint64_t PdtIndex = (virtual_address >> 21) & 0x1FF;
    uint64_t PtIndex = (virtual_address >> 12) & 0x1FF;

    if (!(Pml4->entries[Pml4Index] & PAGE_PRESENT)) {
        void* PdptAddr = KiPmmAlloc();
        if ((uintptr_t)PdptAddr & 0xFFF) return false; // Must be aligned
        memset(PdptAddr, 0, sizeof(PageTable));
        Pml4->entries[Pml4Index] = (uint64_t)(uintptr_t)PdptAddr | Flags;
    }

    d();
    PageTable* Pdpt = (PageTable*)phys_to_virt(Pml4->entries[Pml4Index] & PHYS_PAGE_ADDR_MASK);
    d();
    if (!(Pdpt->entries[PdptIndex] & PAGE_PRESENT)) {
        void* PdtAddr = KiPmmAlloc();
        if ((uintptr_t)PdtAddr & 0xFFF) return false;
        memset(PdtAddr, 0, sizeof(PageTable));
        Pdpt->entries[PdptIndex] = (uint64_t)(uintptr_t)PdtAddr | Flags;
    }
    d();

    d();
    PageTable* Pdt = (PageTable*)((uintptr_t)(Pdpt->entries[PdptIndex] & PHYS_PAGE_ADDR_MASK));
    d();
    if (!(Pdt->entries[PdtIndex] & PAGE_PRESENT)) {
        void* PtAddr = KiPmmAlloc();
        if ((uintptr_t)PtAddr & 0xFFF) return false;
        memset(PtAddr, 0, sizeof(PageTable));
        Pdt->entries[PdtIndex] = (uint64_t)(uintptr_t)PtAddr | Flags;
    }
    d();

    d();
    PageTable* Pt = (PageTable*)((uintptr_t)(Pdt->entries[PdtIndex] & PHYS_PAGE_ADDR_MASK));
    d();
    Pt->entries[PtIndex] = (physical_address & PHYS_PAGE_ADDR_MASK) | Flags;
    d();

    d();
    asm volatile ("invlpg (%0)" : : "r"(virtual_address));
    d();

    d();
    return true;
}

void UnmapPage(void* phys, void* virt) {
    (void)phys;
    const uint64_t virtual_address = (uint64_t)virt;

    uint64_t Pml4Index = (virtual_address >> 39) & 0x1FF;
    uint64_t PdptIndex = (virtual_address >> 30) & 0x1FF;
    uint64_t PdtIndex = (virtual_address >> 21) & 0x1FF;
    uint64_t PtIndex = (virtual_address >> 12) & 0x1FF;

    if (!(Pml4->entries[Pml4Index] & PAGE_PRESENT)) return;

    PageTable* Pdpt = (PageTable*)((uintptr_t)(Pml4->entries[Pml4Index] & PHYS_PAGE_ADDR_MASK));
    if (!(Pdpt->entries[PdptIndex] & PAGE_PRESENT)) return;

    PageTable* Pdt = (PageTable*)((uintptr_t)(Pdpt->entries[PdptIndex] & PHYS_PAGE_ADDR_MASK));
    if (!(Pdt->entries[PdtIndex] & PAGE_PRESENT)) return;

    PageTable* Pt = (PageTable*)((uintptr_t)(Pdt->entries[PdtIndex] & PHYS_PAGE_ADDR_MASK));
    Pt->entries[PtIndex] = 0;

    asm volatile ("invlpg (%0)" : : "r"(virtual_address));
}
