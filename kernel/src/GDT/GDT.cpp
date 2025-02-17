//
// Created by Adam on 2/6/2025.
//

#include "GDT.h"

GDT DefaultGDT = {
    {0, 0, 0, 0x00, 0x00, 0},       // null descriptor
    {0xFFFF, 0, 0, 0x9A, 0xA0, 0},   // kernel code segment (0x08)
    {0xFFFF, 0, 0, 0x92, 0xA0, 0},   // kernel data segment (0x10)
    {0xFFFF, 0, 0, 0xFA, 0xA0, 0},   // user code segment (0x18)
    {0xFFFF, 0, 0, 0xF2, 0xA0, 0},   // user data segment (0x20)
    {0, 0, 0, 0x00, 0x00, 0}         // null descriptor
};

// Function to load the segment registers (CS, DS, SS, ES, FS, GS) from GDT selectors
void LoadSegmentRegisters() {
    // Load the segment registers with appropriate selectors from the GDT
    asm volatile (
        "movw $0x10, %%ax;"      // Load data segment selector (example: 0x10)
        "movw %%ax, %%ds;"       // Set DS register
        "movw %%ax, %%es;"       // Set ES register
        "movw %%ax, %%ss;"       // Set SS register
        "movw $0x08, %%ax;"      // Load code segment selector (example: 0x08)
        "movw %%ax, %%cs;"       // Set CS register
        "movw $0x18, %%ax;"      // Load FS segment selector (example: 0x18)
        "movw %%ax, %%fs;"       // Set FS register
        "movw $0x20, %%ax;"      // Load GS segment selector (example: 0x20)
        "movw %%ax, %%gs;"       // Set GS register
        :
        : // No inputs
        : "%ax"  // Clobbered register
    );
}

void LoadGDT(GDTDescriptor* gdt_desc) {
    GDTDescriptor gdtr;  // Create a GDTR instance
    gdtr.Size = gdt_desc->Size;  // Set the GDT size
    gdtr.Offset = gdt_desc->Offset;  // Set the GDT offset

    // Load the GDT into the GDTR register using inline assembly
    asm volatile (
        "lgdt %0"  // The LGDT instruction loads the GDTR from memory
        :  // No output operands
        : "m"(gdtr)  // Input operand: GDTR structure passed to the LGDT instruction
        : "memory"  // Inform the compiler that memory is modified
    );

    //LoadSegmentRegisters();
}