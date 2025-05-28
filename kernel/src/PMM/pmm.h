/*
    Author: Adam Bassem
    Revision 0
    Patch 0
    Minor 0
    Major 0
    Atlas 0.0.7
*/

#ifndef PMM_H
#define PMM_H 1

#include <stdint.h>
#include <stddef.h>

int KiPmmInit(uint64_t total_mem_bytes, uint32_t page_size, uint8_t *bitmap_mem, size_t bitmap_mem_size, size_t endKernelAligned_);
void* KiPmmAlloc();
void* KiPmmNAlloc(size_t count);
void KiPmmFree(size_t frame);
void KiPmmNFree(void* frame_ptr, size_t count);
size_t KiPmmGetTotalPages();
size_t KiPmmGetFreePages();

#endif /* PMM_H */
