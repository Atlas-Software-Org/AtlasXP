//
// Created by Adam on 2/3/2025.
//

#include <cstdint>
#include <cstddef>
#include <limine.h>

// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

namespace {

__attribute__((used, section(".limine_requests")))
volatile LIMINE_BASE_REVISION(3);

}

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

namespace {

__attribute__((used, section(".limine_requests")))
volatile limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
    .response = nullptr
};
}

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .cpp file, as seen fit.

namespace {

__attribute__((used, section(".limine_requests_start")))
volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
volatile LIMINE_REQUESTS_END_MARKER;

}

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .cpp file.

#include <KernelMemory.h>

void *memcpy(void *dest, const void *src, std::size_t n) {
    std::uint8_t *pdest = static_cast<std::uint8_t *>(dest);
    const std::uint8_t *psrc = static_cast<const std::uint8_t *>(src);

    for (std::size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, std::size_t n) {
    std::uint8_t *p = static_cast<std::uint8_t *>(s);

    for (std::size_t i = 0; i < n; i++) {
        p[i] = static_cast<uint8_t>(c);
    }

    return s;
}

void *memmove(void *dest, const void *src, std::size_t n) {
    std::uint8_t *pdest = static_cast<std::uint8_t *>(dest);
    const std::uint8_t *psrc = static_cast<const std::uint8_t *>(src);

    if (src > dest) {
        for (std::size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (std::size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, std::size_t n) {
    const std::uint8_t *p1 = static_cast<const std::uint8_t *>(s1);
    const std::uint8_t *p2 = static_cast<const std::uint8_t *>(s2);

    for (std::size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
namespace {

void hcf() {
    for (;;) {
#if defined (__x86_64__)
        asm ("hlt");
#elif defined (__aarch64__) || defined (__riscv)
        asm ("wfi");
#elif defined (__loongarch64)
        asm ("idle 0");
#endif
    }
}

}

// The following stubs are required by the Itanium C++ ABI (the one we use,
// regardless of the "Itanium" nomenclature).
// Like the memory functions above, these stubs can be moved to a different .cpp file,
// but should not be removed, unless you know what you are doing.
extern "C" {
    int __cxa_atexit(void (*)(void *), void *, void *) { return 0; }
    void __cxa_pure_virtual() { hcf(); }
    void *__dso_handle;
}

// Extern declarations for global constructors array.
extern void (*__init_array[])();
extern void (*__init_array_end[])();

static uint8_t font8x8[128][8] = {
    // '(' Character (8x8)
    [0] = {0x1E, 0x30, 0x60, 0x60, 0x60, 0x30, 0x1E, 0x00},
    // ')' Character (8x8)
    [1] = {0x1E, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x1E, 0x00},
    // 'E' Character (8x8)
    [2] = {0x7F, 0x40, 0x40, 0x7F, 0x40, 0x40, 0x7F, 0x00},
    // 'R' Character (8x8)
    [3] = {0x7F, 0x41, 0x41, 0x7F, 0x50, 0x48, 0x44, 0x00},
    // '0' Character (8x8)
    [4] = {0x7F, 0x41, 0x41, 0x41, 0x41, 0x41, 0x7F, 0x00},
    // '1' Character (8x8)
    [5] = {0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x7F, 0x00},
    // '2' Character (8x8)
    [6] = {0x7F, 0x01, 0x01, 0x7E, 0x40, 0x40, 0x7F, 0x00},
    // '3' Character (8x8)
    [7] = {0x7F, 0x01, 0x01, 0x7E, 0x01, 0x01, 0x7F, 0x00},
    // '4' Character (8x8)
    [8] = {0x7F, 0x40, 0x40, 0x7F, 0x40, 0x40, 0x40, 0x00},
    // '5' Character (8x8)
    [9] = {0x7F, 0x40, 0x40, 0x7E, 0x01, 0x01, 0x7F, 0x00},
    // '6' Character (8x8)
    [10] = {0x7F, 0x40, 0x40, 0x7F, 0x41, 0x41, 0x7F, 0x00},
    // '7' Character (8x8)
    [11] = {0x7F, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00},
    // '8' Character (8x8)
    [12] = {0x7F, 0x41, 0x41, 0x7F, 0x41, 0x41, 0x7F, 0x00},
    // '9' Character (8x8)
    [13] = {0x7F, 0x41, 0x41, 0x7F, 0x01, 0x01, 0x7F, 0x00}
};

#include <HtKernelTools.h>
extern KernelStatus _HtKernelLoad(BootInfo bootInfo);

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
extern "C" void kmain() {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    // Call global constructors.
    for (std::size_t i = 0; &__init_array[i] != __init_array_end; i++) {
        __init_array[i]();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == nullptr
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    BootInfo bootinfo;
    bootinfo.KernelVerification[0] = 0xAF;
    bootinfo.KernelVerification[1] = 0xAB;
    bootinfo.KernelVerification[2] = 0x99;
    bootinfo.KernelVerification[3] = 0x99;
    memcpy(&bootinfo.Framebuffer, &framebuffer, sizeof(limine_framebuffer));

    _HtKernelLoad(bootinfo);

    // We're done, just hang...
    hcf();
}
