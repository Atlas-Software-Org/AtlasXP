/*
        No Author - main.c
*/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>
#include <printk/printk.h>

#include <GDT/gdt.h>
#include <IDT/idt.h>
#include <Syscalls/Syscalls.h>
#include <PMM/pmm.h>
#include <Paging/paging.h>

#include <Drivers/PS2Keyboard.h>

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
__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

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
    if (memmap_request.response == NULL) {
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
     *          -> Prepare memory manager                               [PMM ONLY/DONE]
     *          -> Map userspace memory
     *          -> Prepare drivers
     *          -> Find a valid userspace with valid 'AtlasSM' OEM
     *          -> Load the userspace
     *          -> Enter usermode
     *          -> Execute the userspace
     *          -> Prepare simple shell... (If the steps above (related to the userspace) are skiped)
     */

    printk("{ LOG }\tBooting up Atlas...\n\r");
    printk("{ LOG }\tAtlas version 0.0.7...\n\r");

    extern char a_endKernel[];
    extern char a_endKernelA[];
    uint64_t endKernel = (uint64_t)&a_endKernel;
    uint64_t endKernelAligned = (uint64_t)&a_endKernelA;

    printk("{ LOG }\tEnd kernel (memory): %llu / %lx\n\r", endKernel, endKernel);
    printk("{ LOG }\tEnd kernel (memory) / Aligned to 0x1000: %llu / %lx\n\r", endKernelAligned, endKernelAligned);

    KiGdtInit();
    idtr_t idtr = KiIdtInit();

    uint8_t bitmap_mem[(0x16636F60 / 0x1000 + 7) / 8];  /* preallocated bitmap */

    int pmm_status = KiPmmInit(0x16636F60, 0x1000, bitmap_mem, sizeof(bitmap_mem), endKernelAligned);
    if (pmm_status != 0) {
        printk("{ LOG }\tFailed to initialize PMM, error code %d / 0x%x\n", pmm_status, pmm_status);
        hcf();
    }

    /* PMM TESTS START */
    printk("{ LOG }\tPMM initialized: total pages = %zu, free pages = %zu\n",
           KiPmmGetTotalPages(), KiPmmGetFreePages());

    void* frame = KiPmmAlloc();
    if (frame == -1) {
        printk("{ LOG }\tFailed to allocate a page\n");
        hcf();
    }

    printk("{ LOG }\tAllocated page frame: %p\n", frame);
    printk("{ LOG }\tFree pages after allocation: %zu\n", KiPmmGetFreePages());

    KiPmmFree((size_t)frame);

    printk("{ LOG }\tFree pages after free: %zu\n", KiPmmGetFreePages());
    /* PMM TESTS END */

    printk("Test suceeded...\n\r");

    hcf();
}