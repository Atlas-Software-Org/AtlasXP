
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <stdarg.h>
#include <HtKernelUtils/debug.h>
#include <HtKernelUtils/io.h>
#include <gfx.h>

#include <GDT/gdt.h>
#include <IDT/idt.h>

#include <InterruptDescriptors/Drivers/KbdDev.h>

#pragma region LIMINE
__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_efi_memmap_request efi_memmap_request = {
    .id = LIMINE_EFI_MEMMAP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
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
#pragma endregion

void kmain(void) {
    #pragma region Step1K
    e9debugk("Loaded AtlasOS64\n\r");
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }
    if (rsdp_request.response == NULL || rsdp_request.response->address == NULL) {
        hcf();
    }
    if (module_request.response == NULL || module_request.response->module_count < 3) {
        hcf();
    }
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    e9debugkf("Framebuffer: %dx%d, BPP: %d\n", framebuffer->width, framebuffer->height, framebuffer->bpp);
    /*efi_memmap efi_memmap = {
        .memmap = efi_memmap_request.response->memmap,
        .memmap_size = efi_memmap_request.response->memmap_size,
        .desc_size = efi_memmap_request.response->desc_size
    };*/
    void* rsdp_address = rsdp_request.response->address;
    struct limine_file *initrd = module_request.response->modules[0]; // INITRD
    e9debugk("Found initramfs.axf\n\r");
    struct limine_file *fontFile = module_request.response->modules[1]; // ZAP LIGHT 16 FONT
    e9debugk("Found zap-light16.psf\n\r");
    struct limine_file *osCfg = module_request.response->modules[2]; // OS CFG
    e9debugk("Found kern64.config\n\r");
    
    // Cast font file address properly
    PSF1_HEADER* fontHeader = (PSF1_HEADER*)fontFile->address;
    if (fontHeader->magic[0] != PSF1_MAGIC0 || fontHeader->magic[1] != PSF1_MAGIC1) {
        e9debugk("Failed to verify font file\n\r");
        hcf();
    }
    
    // Determine glyph buffer size
    uint64_t glyphBufferSize = fontHeader->charsize * 256;
    if (fontHeader->mode == 1) { // 512 Glyph mode
        glyphBufferSize = fontHeader->charsize * 512;
    }
    
    // Allocate memory for glyph buffer (use heap allocation to avoid stack issues)
    void* glyphBuffer = (void*)((uint8_t*)fontFile->address + sizeof(PSF1_HEADER));
    
    // Set up the PSF1 font structure
    PSF1_FONT finishedFont_ins;
    PSF1_FONT* finishedFont = &finishedFont_ins;
    finishedFont->psf1_Header = fontHeader;
    finishedFont->glyphBuffer = glyphBuffer;

    e9debugk("Initializing graphics\n\r");
    InitGfx(framebuffer);
    main_psf1_font = finishedFont;
    if (finishedFont == NULL) {
        e9debugk("Error: finishedFont is not initialized\n\r");
        hcf();
    } else {
        main_psf1_font = finishedFont;
        e9debugk("Graphics initialized\n\r");
    }

    DrawRect(0, 0, GetFb()->width, GetFb()->height, 0xFF4C565E);

    e9debugkf("\033[5m\033[31mClosing E9 Debug log, switching to graphics debugging\033[0m\n\r");

    DrawRect(25, 25, 500, 300, 0xFF282828);
    DrawRect(25, 25, 500, 22, 0xFF202020);
    DrawRect(503, 25, 22, 22, 0xFFE82828);

    FontPutStr("Startup", 30, 27, 0xFFFFFFFF);
    FontPutChar('x', 510, 27, 0xFFFFFFFF);

    ClearColor = 0xFF282828;

    
#pragma endregion
    // Start the log

    FontPutStr("[ LOG ] Logging enabled", 30, 50, 0xFFFFFFFF);

    FontPutStr("[ SYS ] Initalizing GDT/IDT", 30, 66, 0xFFFFFFFF);

    GDTDescriptor gdtDescriptor;
    gdtDescriptor.Size = sizeof(GDT) - 1;
    gdtDescriptor.Offset = (uint64_t)&DefaultGDT;
    LoadGDT(&gdtDescriptor);

    FontPutStr("[ SYS ] Initalized GDT Successfully", 30, 82, 0xFFFFFFFF);

    asm volatile ("cli");  // Disable interrupts

    PIC_remap(0x20, 0x28);   // Remap the PIC

    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);

    outb(0x20, 0x20);
    outb(0xA0, 0x20);

    for (volatile int i = 0; i < 1000; i++) { asm volatile ("nop"); }

    init_exceptions();       // Initialize CPU exception handlers
    idtr_t idtr_ = idt_init();              // Initialize IDT (register handlers)
    
    IOWait();

    uint16_t limit;
    uint64_t base;
    asm volatile ("sidt %0" : "=m" (idtr_));
    e9debugkf("IDT Base: %p, Limit: %x\n", idtr_.base, idtr_.limit);
    e9debugkf("Keyboard handler base: %p\n", &KeyboardInt_Hndlr);

    asm volatile ("sti");    // Enable interrupts AFTER everything is ready    
    
    IRQ_clear_mask(1);       // Unmask IRQ1 (keyboard)

    hcf();
}
