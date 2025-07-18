#ifndef HEAP_H
#define HEAP_H 1

#include <KiSimple.h>
#include <PMM/pmm.h>
#include <stdint.h>
#include <stddef.h>

typedef struct HeapBlock {
	size_t Size;
	uint8_t Used;
	struct HeapBlock* next;
} HeapBlock_t;

void* kalloc(size_t count);
void kfree(void* ptr);

void InitHeap(void* _HeapStart, size_t _HeapSize);

#endif /* HEAP_H */
