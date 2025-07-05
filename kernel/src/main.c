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
uint64_t RamSize = 0;

#define __CONFIG_SERIAL_E9_ENABLE 1

__attribute__((aligned(0x1000))) uint8_t _pmm_mem[4*1024*1024] = {0};

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

	for (size_t i = 0; i < memmap_request.response->entry_count; i++) {
		struct limine_memmap_entry* entry_at_i = memmap_request.response->entries[i];
		if (entry_at_i->type == LIMINE_MEMMAP_USABLE)
			RamSize += entry_at_i->length;
		else
			continue; /* idk but it's trust issues */
	}

	void* stack_base = (void*)&_pmm_mem;
	size_t stack_size = sizeof(_pmm_mem);

	printk("Init PMM with parameters:\n\rBASE: %llX\n\rSIZE: %llX\n\r", VIRT_TO_PHYS(stack_base, RamSize), stack_size);
	
	KiPmmInit(VIRT_TO_PHYS(stack_base, RamSize), stack_size);

	printk("PMM Initiated successfully\n\r");

    KiGdtInit();
    //idtr_t idtr = KiIdtInit();
    //(void)idtr;

	PitInit(1000);

	FB_WIDTH = framebuffer->width;
	FB_HEIGHT = framebuffer->height;
	FB_FLANTERM_CHAR_WIDTH = 1;
	FB_FLANTERM_CHAR_HEIGHT = 1;

	//KiMMap((void*)0x3000, (void*)0x3000, MMAP_PRESENT | MMAP_RW);

	*(uint8_t*)0xFFFFFFFF80400000 = 0xAB;
	printk("%x\n\r", *(uint8_t*)VIRT_TO_PHYS(0xFFFFFFFF80400000, RamSize));

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
