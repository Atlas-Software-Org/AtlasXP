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

#include <Drivers/PS2Keyboard.h>
#include <Drivers/AHCI.h>

#include <Syscalls/syscalls.h>

#include <ELF/elf.h>
#include <BIN/nutshell.h>

#include <FS/FAT32/ff.h>

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
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
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

static void hcf_delayed(int cycles) {
	for (int i = 0; i < cycles * 2; i++) {
		asm volatile ("nop;nop;nop;nop;nop");
	}
}

void DisplaySplash(int w, int h, char* text);

#define __CONFIG_ENABLE_E9_OUTPUT 0
#define __CONFIG_SPLASH_NOP_CYCLE_WASTE (0xFFFFFFFF / 2)

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
    if (module_request.response == NULL
     || module_request.response->module_count < 1
     || module_request.response->modules == NULL) {
    	hcf();
    }
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    struct limine_file* init_module = module_request.response->modules[0];

	if (init_module == NULL) {
		hcf();
	}

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
    idtr_t idtr = KiIdtInit();
    (void)idtr;

    uint8_t bitmap_mem[(0x16636F60 / 0x1000 + 7) / 8];  /* preallocated bitmap */

    int pmm_status = KiPmmInit(0x16636F60, 0x1000, bitmap_mem, sizeof(bitmap_mem), endKernelAligned);
    if (pmm_status != 0) {
        printk("{ LOG }\tFailed to initialize PMM, error code %d / 0x%x\n", pmm_status, pmm_status);
        hcf();
    }

	KiMMap(0x3000, 0x3000, MMAP_PRESENT);	

	PciDevice_t MassStoragePci = PciFindDeviceByClass(0x01, 0x06);
	if (MassStoragePci.vendor_id == 0xFFFF)
		KiPanic("Could not find an AHCI (with SATA enabled) controller attached", 1);
	PciGetDeviceMMIORegion(&MassStoragePci);
	
	AhciInit(&MassStoragePci);

	FIL file;
	f_open(&file, "/filename.txt", FA_READ);

	FILINFO filinfo;
	f_stat("/filename.txt", &filinfo);

	int size = filinfo.fsize;
	char buf[size + 1];

	buf[size + 1] = 0;

	int br = 0;

	f_read(&file, buf, size, &br);

	if (br != size) printk("Read invalid size\n\r");

	printk("Read:\n\r%s\n\r", buf);

	f_close(&file);

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
