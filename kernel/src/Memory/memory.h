#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>
#include <KernelMemory.h>

class MemoryAllocator {
public:
    static constexpr int PAGE_SIZE = 4096;  // Size of a single page (example: 4 KB)
    static constexpr int MAX_PAGE_COUNT = 1024;  // Max number of pages

    MemoryAllocator();
    ~MemoryAllocator();

    void* kalloc(int size);
    void kfree(void* ptr);
    void* krealloc(void* ptr, int new_size);
    void* kcalloc(int num_elements, int size_of_element);

private:
    unsigned char PageBitmaps[MAX_PAGE_COUNT / 8];  // Bitmap for page allocation
    int PageBitmapIndex;  // Index to start searching for free pages

    int kalloc_ceil(int a, int b);
};

#endif // MEMORY_H
