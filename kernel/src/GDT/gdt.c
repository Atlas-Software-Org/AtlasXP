#include "gdt.h"

__attribute__((aligned(0x1000)))
GDT DefaultGDT = {
    {0, 0, 0, 0x00, 0x00, 0}, // 0x00: Null
    {0, 0, 0, 0x9A, 0x20, 0}, // 0x08: Kernel Code (exec, ring 0)
    {0, 0, 0, 0x92, 0x20, 0}, // 0x10: Kernel Data (rw, ring 0)
    {0, 0, 0, 0x00, 0x00, 0}, // 0x18: Null
    {0, 0, 0, 0xFA, 0x20, 0}, // 0x20: User Code (exec, ring 3)
    {0, 0, 0, 0xF2, 0x20, 0}, // 0x28: User Data (rw, ring 3)
};

__attribute__((aligned(0x1000))) GDTDescriptor gdtDescriptor;

void KiGdtInit() {
    gdtDescriptor.Size = sizeof(GDT) - 1;
    gdtDescriptor.Offset = (uint64_t)&DefaultGDT;
    KiGdtLoad(&gdtDescriptor);
}
