//
// Created by Adam on 2/3/2025.
//

using namespace std;

#include <SYS_V_ABI/SYSV_ABI.h>
#include <HtKernelTools.h>
#include <Memory/memory.h>
#include <Atlasglib/aglib.h>
#include <limine.h>
#include <stdint.h>
#include <stddef.h>
#include <GDT/GDT.h>
#include <IDT/IDT.h>
#include <IDT/InterruptDescriptors/Devices/KbdDev/KbdDev.h>
#include <io.h>
#include <Atlasglib/aglib_global.h>
#include <FontRenderer.h>

static KernelStatus _HtKernel(BootInfo* __BootInfo);

__attribute__((section("._HtKernel")))
KernelStatus _HtKernelLoad(BootInfo bootInfo) {
    BootInfo info_ = bootInfo;
    return _HtKernel(&info_);
}

__attribute__((section("._HtKernel")))
static KernelStatus _HtKernel(BootInfo* __BootInfo) {
    // ========================================================================== //
    //              Graphics Library                                              //
    // ========================================================================== //
    GraphicsLibrary gfx(
        __BootInfo->Framebuffer->width,     // Framebuffer width
        __BootInfo->Framebuffer->height,    // Framebuffer height
        __BootInfo->Framebuffer->bpp,       // Bits per pixel (e.g., 32)
        __BootInfo->Framebuffer->pitch,     // Pitch (bytes per row)
        (volatile uint32_t*)__BootInfo->Framebuffer->address,   // Pointer to the framebuffer
        false                                // Use alpha blending (set as true or false depending on your need)
    );

    // ========================================================================== //
    //              GDT & IDT                                                     //
    // ========================================================================== //
    // GDT
    GDTDescriptor gdtDesc;
    gdtDesc.Size = sizeof(GDT) - 1;
    gdtDesc.Offset = (uint64_t)&DefaultGDT;
    LoadGDT(&gdtDesc); // Load the GDT

    // IDT
    idt_init();

    idt_set_descriptor(0x21, GET_FUNC_ADDRESS(KbdDevInterruptHandler), 0x08);
    
    return KERNEL_STARTUP_SUCCESS;
}