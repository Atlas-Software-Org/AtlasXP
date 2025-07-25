#include "pmm.h"
#include <ELF/elf.h>

typedef struct {
    uint8_t *bitmap;
    int pidTrace[0x1000];
    size_t bitmap_size_bytes;
    size_t total_pages;
    size_t free_pages;
    size_t endKernelAligned_;
} PmmManager;

static PmmManager pmm;

/* Kernel-callable PMM initialization
   bitmap_mem points to preallocated memory for the bitmap
*/
int KiPmmInit(uint64_t total_mem_bytes, uint32_t page_size, uint8_t *bitmap_mem, size_t bitmap_mem_size, size_t endKernelAligned_, int _void_end_k) {
	if (_void_end_k)
    	(void)endKernelAligned_; /* This is why the PMM allocates at address 0, but i will keep it for now since ASNU is not meant to support Lenovo (as of now of course)
    								Due to the stupid reservation of 0x0000000000000000-0x000000000009FFFF even for UEFI */
    pmm.total_pages = total_mem_bytes / page_size;
    pmm.free_pages = pmm.total_pages;
    pmm.bitmap_size_bytes = (pmm.total_pages + 7) / 8;

    if (bitmap_mem_size < pmm.bitmap_size_bytes) return -1;

    pmm.bitmap = bitmap_mem;

    for (size_t i = 0; i < pmm.bitmap_size_bytes; i++) {
        pmm.bitmap[i] = 0;
    }

    for (int i = 0; i < 0x1000; i++) {
        pmm.pidTrace[i] = 0;
    }

    return 0;
}

static inline void PmmSetBit(uint8_t *bitmap, size_t bit) {
    bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void PmmClearBit(uint8_t *bitmap, size_t bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline int PmmTestBit(uint8_t *bitmap, size_t bit) {
    return (bitmap[bit / 8] >> (bit % 8)) & 1;
}

void* KiPmmAlloc() {
    for (size_t i = 0; i < pmm.total_pages; i++) {
        if (!PmmTestBit(pmm.bitmap, i)) {
            PmmSetBit(pmm.bitmap, i);
            pmm.free_pages--;
            pmm.pidTrace[i] = CurrentPid;
            return (void*)(pmm.endKernelAligned_ + (i * 0x1000));
        }
    }
    return NULL;
}

void* KiPmmNAlloc(size_t count) {
    void* initial = KiPmmAlloc();
    if (initial == NULL) return NULL;

    for (size_t i = 1; i < count; i++) {
        void* test = KiPmmAlloc();
        if (test == NULL) return NULL;
    }

    return initial;
}

void KiPmmFree(void* frame_ptr) {
    uintptr_t addr = (uintptr_t)frame_ptr;
    if (addr < pmm.endKernelAligned_) return;

    size_t frame = (addr - pmm.endKernelAligned_) / 0x1000;
    if (frame >= pmm.total_pages) return;

    if (PmmTestBit(pmm.bitmap, frame)) {
        PmmClearBit(pmm.bitmap, frame);
        pmm.free_pages++;
        pmm.pidTrace[frame] = 0;
    }
}

void KiPmmNFree(void* frame_ptr, size_t count) {
    for (size_t i = 0; i < count; i++) {
        KiPmmFree((void*)((uintptr_t)frame_ptr + (i * 0x1000)));
    }
}

size_t KiPmmGetTotalPages() {
    return pmm.total_pages;
}

size_t KiPmmGetFreePages() {
    return pmm.free_pages;
}

static void KiPmmFreeFrame(int frame) {
    if (frame >= pmm.total_pages) return;

    if (PmmTestBit(pmm.bitmap, frame)) {
        PmmClearBit(pmm.bitmap, frame);
        pmm.free_pages++;
        pmm.pidTrace[frame] = 0;
    }
}

void KiPmmClearPidTracedResources(int pid) {
    for (size_t i = 0; i < pmm.total_pages; i++) {
        if (pmm.pidTrace[i] == pid) {
            KiPmmFreeFrame(i);
        }
    }
}
