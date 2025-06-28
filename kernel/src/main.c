#include <limine.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <flanterm/flanterm.h>
#include <flanterm/flanterm_backends/fb.h>
#include <printk/printk.h>

#include <GDT/gdt.h>
#include <IDT/idt.h>
#include <PMM/pmm.h>
#include <VMM/vmm.h>
#include <ACPI/acpi.h>

#include <Drivers/PIT.h>
#include <Drivers/PS2Keyboard.h>
#include <Drivers/AHCI.h>

#include <Syscalls/syscalls.h>

#include <ELF/elf.h>

#include <BIN/nutshell.h>

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
__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
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

uint64_t FB_WIDTH, FB_HEIGHT, FB_FLANTERM_CHAR_WIDTH, FB_FLANTERM_CHAR_HEIGHT;
uint64_t HhdmOffset;

#define __CONFIG_SERIAL_E9_ENABLE 0

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

    SetGlobalFtCtx(ft_ctx, __CONFIG_SERIAL_E9_ENABLE);

    KiGdtInit();
    idtr_t idtr = KiIdtInit();
    (void)idtr;

	uint64_t RAMSize = 0;
	
	uint64_t RAMEntrySizes[memmap_request.response->entry_count];
	uint64_t RAMEntryBase[memmap_request.response->entry_count];
	size_t RAMEntrySizesPtr = 0;
	
	for (size_t i = 0; i < memmap_request.response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_request.response->entries[i];
	
		if (entry->type == LIMINE_MEMMAP_USABLE || entry->type == LIMINE_MEMMAP_FRAMEBUFFER) {
			RAMSize += entry->length;
	
			if (entry->type != LIMINE_MEMMAP_FRAMEBUFFER) {
				RAMEntrySizes[RAMEntrySizesPtr] = entry->length;
				RAMEntryBase[RAMEntrySizesPtr] = entry->base;
				RAMEntrySizesPtr++;
			}
		}
	}
	
	printk("uint64_t RAMSize = 0x%llx;\n\r", RAMSize);
	
	for (size_t i = 0; i < RAMEntrySizesPtr; i++) {
		printk("uint64_t RAMEntryCount[0x%llX] = 0x%llX;\n\r", i, RAMEntrySizes[i]);
	}
	
	uint64_t RAMSizeQuotient = RAMSize / (1024ULL * 1024 * 1024);
	uint64_t RAMSizeRemainder = RAMSize % (1024ULL * 1024 * 1024);
	
	if (RAMSizeRemainder >= (512ULL * 1024 * 1024)) {
		RAMSizeQuotient += 1;
	}
	
	printk("uint64_t RAMSizeRound = %llu;\n\r", RAMSizeQuotient);
	
	size_t LargestIndex = 0;
	
	for (size_t i = 1; i < RAMEntrySizesPtr; i++) {
		if (RAMEntrySizes[i] > RAMEntrySizes[LargestIndex] &&
			RAMEntryBase[i] < RAMSize - ((2ULL * 1024 * 1024 * 1024) - 4096)
		) {
			LargestIndex = i;
		}
	}
	
	printk("Largest index below the topmost 2GiB region is index: %llu\n\rSize: 0x%llX\n\rBase: 0x%llX\n\r",
		LargestIndex, RAMEntrySizes[LargestIndex], RAMEntryBase[LargestIndex]);

	KiPmmInit(RAMEntryBase[LargestIndex], RAMEntrySizes[LargestIndex]);

	hcf();

	PitInit(1000);

	FB_WIDTH = framebuffer->width;
	FB_HEIGHT = framebuffer->height;
	FB_FLANTERM_CHAR_WIDTH = 1;
	FB_FLANTERM_CHAR_HEIGHT = 1;

    printk("\e[2J\e[H");

    LoadKernelElf((void*)nutshell, 0, 0, 0);

    hcf();
}

void _kernel_idle_process() {
	printk("\e[2J\e[H");
	printk("Entered IDLE mode...\n\r\n\r");
	printk("To restart nutshell press any key...\n\r\n\r");
	printk("-\tYou may change the power state of the system as there is no process executing at the moment\n\r");

	KiReadHidC();

	LoadKernelElf(nutshell, 0, 0, 0);
}
