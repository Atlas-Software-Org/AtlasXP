#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>
#include <printk/printk.h>

#include <GDT/gdt.h>
#include <IDT/idt.h>
#include <Paging/paging.h>

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};
__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

static void hcf(void) {
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

void KiStartupInit(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }
    if (hhdm_request.response == NULL) {
        hcf();
    }
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    struct flanterm_context *ft_ctx = flanterm_fb_init(
        NULL,
        NULL,
        framebuffer->address, framebuffer->width, framebuffer->height, framebuffer->pitch,
        framebuffer->red_mask_size, framebuffer->red_mask_shift,
        framebuffer->green_mask_size, framebuffer->green_mask_shift,
        framebuffer->blue_mask_size, framebuffer->blue_mask_shift,
        NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, 0, 0, 1,
        0, 0,
        0
    );

    SetGlobalFtCtx(ft_ctx);

    /*
     *      Steps of loading AtlasOS
     *          -> Load FT CTX (flanterm)                               [DONE]
     *          -> Load GDT                                             [DONE]
     *          -> Load IDT                                             [DONE]
     *          -> Prepare memory manager
     *          -> Map userspace memory
     *          -> Prepare drivers
     *          -> Find a valid userspace with valid 'AtlasSM' OEM
     *          -> Load the userspace
     *          -> Enter usermode
     *          -> Execute the userspace
     */

    printk("\t{ LOG }\tBooting up Atlas...\n\r");
    printk("\t{ LOG }\tAtlas version 0.0.7...\n\r");

    KiGdtInit();
    KiIdtInit();

    printk("\t{ LOG }\tHHDM Offset = %llu / %lx\n\r", hhdm_request.response->offset, hhdm_request.response->offset);

    const uint64_t HHDM_BASE  = hhdm_request.response->offset;

    pte_t* pml4 = KiPml4Init();
    uint64_t pml4_phys = (uint64_t)pml4 - HHDM_BASE;

    asm volatile (
        "mov %0, %%cr3"
        :
        : "r"(pml4_phys)
        : "memory"
    );

    printk("\t{ LOG }\tLoaded PML4...\n\r");

    hcf();
}