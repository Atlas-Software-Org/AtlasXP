#include "Heap.h"

void* HeapStart = NULL;
void* HeapEnd = NULL;
size_t HeapSizeBytes = 0;

static HeapBlock_t* HeapFirstBlock = NULL;

void* kalloc(size_t count) {
	HeapBlock_t* current = HeapFirstBlock;

	while (current) {
		if (!current->Used && current->Size >= count) {
			if (current->Size >= count + sizeof(HeapBlock_t) + 1) {
				HeapBlock_t* split = (HeapBlock_t*)((uint8_t*)current + sizeof(HeapBlock_t) + count);
				split->Size = current->Size - count - sizeof(HeapBlock_t);
				split->Used = 0;
				split->next = current->next;
				current->next = split;
				current->Size = count;
			}

			current->Used = 1;
			return (void*)((uint8_t*)current + sizeof(HeapBlock_t));
		}
		current = current->next;
	}

	return NULL;
}

void kfree(void* ptr) {
	if (!ptr) return;

	HeapBlock_t* block = (HeapBlock_t*)((uint8_t*)ptr - sizeof(HeapBlock_t));
	block->Used = 0;

	HeapBlock_t* current = HeapFirstBlock;
	while (current && current->next) {
		if (!current->Used && !current->next->Used) {
			current->Size += sizeof(HeapBlock_t) + current->next->Size;
			current->next = current->next->next;
		} else {
			current = current->next;
		}
	}
}

void InitHeap(void* _HeapStart, size_t _HeapSize) {
	HeapStart = _HeapStart;
	HeapEnd = (void*)((size_t)_HeapStart + _HeapSize);
	HeapSizeBytes = _HeapSize;

	HeapFirstBlock = (HeapBlock_t*)HeapStart;
	HeapFirstBlock->Size = HeapSizeBytes - sizeof(HeapBlock_t);
	HeapFirstBlock->Used = 0;
	HeapFirstBlock->next = NULL;
}
