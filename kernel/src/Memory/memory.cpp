#include "memory.h"

MemoryAllocator::MemoryAllocator() : PageBitmapIndex(0) {
    memset(PageBitmaps, 0, sizeof(PageBitmaps));
}

MemoryAllocator::~MemoryAllocator() {
}

int MemoryAllocator::kalloc_ceil(int a, int b) {
    if (a % b == 0) {
        return a / b;  // No remainder
    }
    return (a / b) + 1;  // Round up
}

void* MemoryAllocator::kalloc(int size) {
    int totalPages = kalloc_ceil(size, PAGE_SIZE);
    int startPage = -1;

    // Search for free pages
    for (int i = PageBitmapIndex; i < MAX_PAGE_COUNT; ++i) {
        if (!(PageBitmaps[i / 8] & (1 << (i % 8)))) {
            // If this is the first free page found
            if (startPage == -1) {
                startPage = i;
            }

            // Check if we have enough contiguous free pages
            if (i - startPage + 1 >= totalPages) {
                // Mark the pages as allocated
                for (int j = startPage; j < startPage + totalPages; ++j) {
                    PageBitmaps[j / 8] |= (1 << (j % 8));
                }
                PageBitmapIndex = startPage + totalPages;
                return (void*)(startPage * PAGE_SIZE);
            }
        } else {
            // If a free page streak breaks, reset startPage
            startPage = -1;
        }
    }

    return nullptr;  // Not enough continuous pages available
}

void MemoryAllocator::kfree(void* ptr) {
    int startPage = ((uint8_t*)ptr - (uint8_t*)0) / PAGE_SIZE;

    // If the pointer is outside of the valid range, return
    if (startPage < 0 || startPage >= MAX_PAGE_COUNT) {
        return;
    }

    // Find how many pages to free (using the PageBitmapIndex)
    int totalPages = 0;
    while (startPage + totalPages < MAX_PAGE_COUNT &&
           (PageBitmaps[(startPage + totalPages) / 8] & (1 << ((startPage + totalPages) % 8)))) {
        totalPages++;
    }

    // Free the pages by marking them as free in the bitmap
    for (int i = startPage; i < startPage + totalPages; ++i) {
        PageBitmaps[i / 8] &= ~(1 << (i % 8));
    }
}

void* MemoryAllocator::krealloc(void* ptr, int new_size) {
    if (ptr == nullptr) {
        return kalloc(new_size);
    }

    // Get the original size of the memory block (you would need to store this in a real allocator)
    int original_size = 0;

    // Allocate a new block of memory
    void* new_ptr = kalloc(new_size);
    if (new_ptr == nullptr) {
        return nullptr;  // Allocation failed
    }

    // Copy the old contents to the new block
    if (ptr != nullptr) {
        memcpy(new_ptr, ptr, (original_size < new_size) ? original_size : new_size);
        kfree(ptr);  // Free the old block
    }

    return new_ptr;
}

void* MemoryAllocator::kcalloc(int num_elements, int size_of_element) {
    int total_size = num_elements * size_of_element;
    void* ptr = kalloc(total_size);

    if (ptr != nullptr) {
        // Zero out the allocated memory
        memset(ptr, 0, total_size);
    }

    return ptr;
}
