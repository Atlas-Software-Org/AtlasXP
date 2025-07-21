#include "stdlib.h"
#include "../LibAsnuApi/AsnuApi.h"

#define PAGE_SIZE 4096

typedef struct {
    int pages;
} MallocHeader;

void* malloc(int size) {
    int npages = (size + sizeof(MallocHeader) + PAGE_SIZE - 1) / PAGE_SIZE;
    void** blocks = (void**)memalloc();
    if (!blocks) return 0;

    blocks[0] = blocks;  // pointer to self for freeing
    for (int i = 1; i < npages; ++i) {
        blocks[i] = memalloc();
        if (!blocks[i]) {
            for (int j = 1; j < i; ++j)
                memfree(blocks[j]);
            memfree(blocks);
            return 0;
        }
    }

    MallocHeader* hdr = (MallocHeader*)blocks[0];
    hdr->pages = npages;
    return (void*)((char*)hdr + sizeof(MallocHeader));
}

void* calloc(int n, int s) {
    void* p = memalloc();
    if (p) {
        char* b = p;
        for (int i = 0; i < n * s; ++i)
            b[i] = 0;
    }
    return p;
}

void free(void* ptr) {
    if (!ptr) return;

    MallocHeader* hdr = (MallocHeader*)((char*)ptr - sizeof(MallocHeader));
    int pages = hdr->pages;
    void** blocks = (void**)hdr;

    for (int i = 1; i < pages; ++i)
        memfree(blocks[i]);
    memfree(blocks);
}

void exit(int code) {
    asnu_api_exit(code);
}

int getpid() {
    return getpid();
}
