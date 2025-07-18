#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include <flanterm/flanterm.h>
#include <flanterm/flanterm_backends/fb.h>
#include <printk/printk.h>
#include <CPIO/cpio_newc.h>

#include <GDT/gdt.h>
#include <IDT/idt.h>
#include <PMM/pmm.h>
#include <VMM/vmm.h>
#include <PCI/pci.h>
#include <Heap/Heap.h>

#include <Drivers/PS2Keyboard.h>
#include <Drivers/AHCI.h>
#include <Drivers/XHCI.h>

#include <Syscalls/syscalls.h>

#include <ELF/elf.h>
#include <BIN/nutshell.h>

#include <FS/FAT32/fat32.h>

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

static void hcf_delayed(uint64_t cycles) {
	for (uint64_t i = 0; i < cycles * 2; i++) {
		asm volatile ("nop;nop;nop;nop;nop");
	}
}

void DisplaySplash(int w, int h, char* text);

#define __CONFIG_ENABLE_E9_OUTPUT 0
#define __CONFIG_SPLASH_NOP_CYCLE_WASTE (0xFFFFFFFFFFFFFFFF / 0xFFFFFFFF / 8)

uint64_t FB_WIDTH, FB_HEIGHT, FB_FLANTERM_CHAR_WIDTH, FB_FLANTERM_CHAR_HEIGHT;
uint64_t HhdmOffset;
uint64_t RamSize = 0;

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

    SetGlobalFtCtx(ft_ctx, __CONFIG_ENABLE_E9_OUTPUT);

	printk("\x1b[?25l");

	FB_WIDTH = framebuffer->width;
	FB_HEIGHT = framebuffer->height;

	FB_FLANTERM_CHAR_WIDTH = (FB_WIDTH + 1) / (8 + 1);
	FB_FLANTERM_CHAR_HEIGHT = (FB_HEIGHT + 1) / (16 + 1);

	DisplaySplash(FB_FLANTERM_CHAR_WIDTH, FB_FLANTERM_CHAR_HEIGHT, "ASNU-Kernel project\nAtlas Software & Microsystems\n$INF<Build 1.0.0.2025>INF$");
	hcf_delayed(__CONFIG_SPLASH_NOP_CYCLE_WASTE);

	printk("\e[2J\e[H");

	for (size_t i = 0; i < memmap_request.response->entry_count; i++) {
		struct limine_memmap_entry* entry_at_i = memmap_request.response->entries[i];
		if (entry_at_i->type == LIMINE_MEMMAP_USABLE)
			RamSize += entry_at_i->length;
		else
			continue; /* idk but it's trust issues */
	}

    extern char a_endKernelA[];
    uint64_t endKernelAligned = (uint64_t)&a_endKernelA;

    KiGdtInit();

	PciDevice_t xHciController = PciFindDeviceByClass(0x0C, 0x03);
	if (xHciController.vendor_id == 0xFFFF)
		KiPanic("Could not find an xHCI controller attached", 1);
	PciGetDeviceMMIORegion(&xHciController);

	xHciInit(&xHciController);
    
    idtr_t idtr = KiIdtInit();
    (void)idtr;

    uint8_t bitmap_mem[(0x16636F60 / 0x1000 + 7) / 8];  /* preallocated bitmap */

    int pmm_status = KiPmmInit(0x16636F60, 0x1000, bitmap_mem, sizeof(bitmap_mem), endKernelAligned);
    if (pmm_status != 0) {
        printk("{ LOG }\tFailed to initialize PMM, error code %d / 0x%x\n", pmm_status, pmm_status);
        hcf();
    }

	/* FIXME:

	size_t MemMapEntryCount = memmap_request.response->entry_count;

	size_t MemMapEntryIndexLargest = -1;
	size_t MemMapEntryIndexLargestSz = 0;

	size_t RamSize = 0;

	for (size_t i = 0; i < MemMapEntryCount; i++) {
		struct limine_memmap_entry* entry = memmap_request.response->entries[i];

		if (entry->length > MemMapEntryIndexLargestSz && entry->type == LIMINE_MEMMAP_USABLE) {
			MemMapEntryIndexLargestSz = entry->length;
			MemMapEntryIndexLargest = i;
		}

		if (entry->type == LIMINE_MEMMAP_USABLE) {
			RamSize += entry->length;
		}
	}

	if (MemMapEntryIndexLargest == -1) KiPanic("Failed to detect memory", 1);
	if (MemMapEntryIndexLargestSz < (0x1000 * 2)) KiPanic("Failed to detect memory\n\r(Largest memory segment is smaller than two pages)", 1);

	RamSize = (RamSize + ((1024ULL * 1024 * 1024) / 2)) / (1024ULL * 1024 * 1024);

	printk("Detected memory\n\r%d.0 GB\n\r", RamSize);

	for (size_t i = 0; i < MemMapEntryIndexLargestSz / 0x1000; i++) {
		void* Address = (void*)((uint64_t)memmap_request.response->entries[MemMapEntryIndexLargest]->base + (i * 0x1000));
		KiMMap(Address, Address, MMAP_PRESENT | MMAP_RW);
	}

	InitHeap(memmap_request.response->entries[MemMapEntryIndexLargest]->base, MemMapEntryIndexLargestSz);

	*/

	PciDevice_t MassStoragePci = PciFindDeviceByClass(0x01, 0x06);
	if (MassStoragePci.vendor_id == 0xFFFF)
		KiPanic("Could not find an AHCI (with SATA enabled) controller attached", 1);
	PciGetDeviceMMIORegion(&MassStoragePci);
	
	AhciInit(&MassStoragePci);

	hcf();

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

	LoadKernelElf((void*)nutshell, 0, 0, 0);
}
