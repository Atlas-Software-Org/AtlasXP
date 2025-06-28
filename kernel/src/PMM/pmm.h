#ifndef PMM_H
#define PMM_H 1

#include <KiSimple.h>
#include <stdint.h>
#include <stddef.h>

void KiPmmInit(uint64_t mem_base, uint64_t mem_size);
void* KiPmmAlloc();
void* KiPmmNAlloc(size_t count);
void KiPmmFree(void* frame_ptr);
void KiPmmNFree(void* frame_ptr, size_t count);
size_t KiPmmGetTotalPages();
size_t KiPmmGetFreePages();

void KiPmmClearPidTracedResources(int pid);

#endif /* PMM_H */
