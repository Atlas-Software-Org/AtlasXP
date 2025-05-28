/*
    Author: Adam Bassem
    Revision 0
    Patch 0
    Minor 0
    Major 0
    Atlas 0.0.7
*/

#ifndef PAGING_H
#define PAGING_H 1

/*
    Author: Adam Bassem
    Revision 0
    Patch 0
    Minor 0
    Major 0
    Atlas 0.0.7
*/

#include <stdint.h>
#include <stdbool.h>
#include <PMM/pmm.h>
#include <KiSimple.h>

#define PHYS_PAGE_ADDR_MASK 0x000FFFFFFFFFF000LLU
#define PAGE_SIZE 0x1000

typedef struct {
    uint64_t entries[512];
} PageTable;

uint64_t KiRetrievePml4();

#define PAGE_PRESENT    (1 << 0)
#define PAGE_READWRITE  (1 << 1)
#define PAGE_USER       (1 << 2)

bool MapPage(void* phys, void* virt);
void UnmapPage(void* phys, void* virt);

#endif /* PAGING_H */