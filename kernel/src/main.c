#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

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

const char *MemMapEntryTypes[] = {
    "LIMINE_MEMMAP_USABLE                ",
    "LIMINE_MEMMAP_RESERVED              ",
    "LIMINE_MEMMAP_ACPI_RECLAIMABLE      ",
    "LIMINE_MEMMAP_ACPI_NVS              ",
    "LIMINE_MEMMAP_BAD_MEMORY            ",
    "LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE",
    "LIMINE_MEMMAP_KERNEL_AND_MODULES    ",
    "LIMINE_MEMMAP_FRAMEBUFFER           "
};

#define __CONFIG_SERIAL_E9_ENABLE 1
#define __CONFIG_LOG_DETECTED_MEMORY_MAP 1

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

	uint64_t max_size = 0;
	uint64_t max_base = 0;

	for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
	    struct limine_memmap_entry *entry = memmap_request.response->entries[i];

	    if (entry->type == LIMINE_MEMMAP_USABLE) {
	        if (entry->length > max_size) {
	            max_size = entry->length;
	            max_base = entry->base;
	            printk("Found largest segment:\n\rSize: %0llX\n\rBase: %0llX\n\r", max_size, max_base);
	        }

	        printk("%2llu: %s - %p - %p", i, MemMapEntryTypes[entry->type],
	               (void*)entry->base, (void*)entry->length);

	        printk("  ***\n\r");
	    }
	}

	if (max_size == 0)
	    KiPanic("No usable memory found in Limine map", 1);

	uint64_t total_pages = max_size / 0x1000;
	uint64_t bitmap_size = (total_pages + 7) / 8;
	bitmap_size = (bitmap_size + 0xFFF) & ~0xFFF; // align to 0x1000

	uint8_t* bitmap_base = (uint8_t*)(uintptr_t)max_base;
	uint64_t usable_base_after_bitmap = max_base + bitmap_size;
	uint64_t usable_size_after_bitmap = max_size - bitmap_size;

	int result = KiPmmInit(
		max_base, max_size, bitmap_base
	);

	if (result != 0)
	    KiPanic("KiPmmInit failed", 1);

	PitInit(1000);

    KiGdtInit();
    idtr_t idtr = KiIdtInit();
    (void)idtr;

	/*

	printk("Searching for AHCI controller (SATA)\n\r");
    
    uint8_t bus, dev, func;
    if (!PciFindDeviceByClass(0x01, 0x06, 0x01, &bus, &dev, &func))
        return;

    uint32_t AhciBAR5 = PciGetBar(bus, dev, func, 5);
    AhciBAR5 &= ~0xF;
    KiMMap((void*)AhciBAR5, (void*)AhciBAR5, MMAP_PRESENT | MMAP_RW);

    volatile uint32_t* abar = (volatile uint32_t*)AhciBAR5;
    uint32_t cap = abar[0]; // AHCI CAP register

    if (cap == 0) {
        KiPanic(1, "AHCI dead...");
    }

    AhciSendZeroRead(abar);

    printk("Initialized AHCI...\n\r[ PCI ] BAR5 is 0x%08X\n\r", AhciBAR5);

	*/

	FB_WIDTH = framebuffer->width;
	FB_HEIGHT = framebuffer->height;
	FB_FLANTERM_CHAR_WIDTH = 1;
	FB_FLANTERM_CHAR_HEIGHT = 1;

    printk("\e[2J\e[H");

    LoadKernelElf(nutshell, 0, 0, 0);

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
