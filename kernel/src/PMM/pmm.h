#ifndef PMM_H
#define PMM_H 1

#include <stdint.h>
#include <stddef.h>

int KiPmmInit(uintptr_t memory_base, size_t memory_size, uintptr_t bitmap_virt);
void* KiPmmAlloc();
void* KiPmmNAlloc(size_t count);
void KiPmmFree(void* frame_ptr);
void KiPmmNFree(void* frame_ptr, size_t count);
size_t KiPmmGetTotalPages();
size_t KiPmmGetFreePages();

void KiPmmClearPidTracedResources(int pid);

#endif /* PMM_H */
