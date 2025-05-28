/*
    Author: Adam Bassem
    Revision 0
    Patch 0
    Minor 0
    Major 0
    Atlas 0.0.7
*/

#include "gdt.h"

__attribute__((aligned(0x1000)))
GDT DefaultGDT = {
    {0, 0, 0, 0x00, 0x00, 0}, // Null (index 0, selector 0x00)
    {0, 0, 0, 0x9A, 0xA0, 0}, // Kernel Code (index 1, selector 0x08)
    {0, 0, 0, 0x92, 0xA0, 0}, // Kernel Data (index 2, selector 0x10)
    {0, 0, 0, 0xFA, 0xA0, 0}, // User Code   (index 3, selector 0x18)
    {0, 0, 0, 0xF2, 0xA0, 0}, // User Data   (index 4, selector 0x20)
};

__attribute__((aligned(0x1000))) GDTDescriptor gdtDescriptor;

void KiGdtInit() {
    gdtDescriptor.Size = sizeof(GDT) - 1;
    gdtDescriptor.Offset = (uint64_t)&DefaultGDT;
    KiGdtLoad(&gdtDescriptor);
    printk("{ LOG }\tLoaded GDT...\n\r");
}