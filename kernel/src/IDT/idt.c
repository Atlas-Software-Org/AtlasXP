/*
    Author: Adam Bassem
    Revision 0
    Patch 0
    Minor 0
    Major 0
    Atlas 0.0.7
*/

#include "idt.h"

typedef struct {
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
	uint32_t    isr_high;     // The higher 32 bits of the ISR's address
	uint32_t    reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t;

__attribute__((aligned(0x1000))) 
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance

static idtr_t idtr;

char error_codes[32][128] = {
    "Division by 0", "Reserved1", "NMI Interrupt", "Breakpoint (INT3)", "Overflow (INTO)", "Bounds range exceeded (BOUND)", "Invalid opcode (UD2)",
    "Device not available (WAIT/FWAIT)", "Double fault", "Coprocessor segment overrun", "Invalid TSS", "Segment not present", "Stack-segment fault",
    "General protection fault (GPFault)", "Page fault", "Reserved2", "x87 FPU error", "Alignment check", "SIMD Floating-Point Exception", "Reserved3",
};

void KiExceptionHandler(int exception) {
    if (exception == 0) {
        printk("\x1b[1;91m{ PANIC }\tIDT Exception occured during execution...\n\r\t\t%s\n\r", error_codes[exception]);
        asm volatile (
            ".intel_syntax noprefix\n\t"
            "mov rax, 0\n\t"
            "mov rdx, 0xFFFFFFFFFFFFFFFF\n\t"
            ".att_syntax prefix"
        );
        return;
    }

    if (0 <= exception && exception < 32)
        printk("\x1b[1;91m{ PANIC }\tIDT Exception occured during execution...\n\r\t\t%s\n\r", error_codes[exception]);
    else
        return;
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
    while (1);
}

void KiIdtSetDesc(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low        = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs      = GDT_OFFSET_KERNEL_CODE;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;

    printk("\x1B[92m{ LOG }\tCreated IDT descriptor:\n\r"
        "34: idt_entry_t* descriptor = &idt[0x%x];\n\r"
        "36: descriptor->isr_low = (uint64_t)0x%p & 0xFFFF;\n\r"
        "37: descriptor->kernel_cs = 0x%x; /* 0x08 */\n\r"
        "38: descriptor->ist = 0;\n\r"
        "39: descriptor->attributes = 0x%x;\n\r"
        "40: descriptor->isr_mid = ((uint64_t)0x%p >> 16) & 0xFFFF;\n\r"
        "41: descriptor->isr_high = ((uint64_t)0x%p >> 32) & 0xFFFFFFFF;\n\r"
        "42: descriptor->reserved = 0;\n\r\x1B[0m",
        vector, isr, GDT_OFFSET_KERNEL_CODE, flags, isr, isr
    );
}

static bool vectors[IDT_MAX_DESCRIPTORS];

extern void* isr_stub_table[];

void KiInitExceptions() {
    for (uint8_t vector = 0; vector < 32; vector++) {
        KiIdtSetDesc(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }
}

__attribute__((interrupt)) void KiKeyboardHandler(int* __unused) {
    uint8_t sc = inb(0x60);
    KeyboardDriverMain(sc);
    KiPicSendEoi(1);
    outb(0x20, 0x20);
}

void KeyboardFlushBuffer() {
    while (inb(0x64) & 1) {
        volatile uint8_t discard = inb(0x60);
    }
}

idtr_t KiIdtInit() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)(sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1);

    KiInitExceptions();

    KiPicRemap(0x20, 0x28);

    KiIdtSetDesc(0x21, (void*)&KiKeyboardHandler, 0x8E);

    printk("{ LOG }\tDBG: Keyboard handler address: %lx\n\r", (void*)&KiKeyboardHandler);

    outb(PIC1_DATA, 0b11111101);
    outb(PIC2_DATA, 0b11111111);
    KiIrqClearMask(1);

    asm volatile ("cli");

    __asm__ volatile ("lidt %0" : : "m"(idtr));

    printk("{ LOG }\tLoaded IDT...\n\r");

    asm volatile ("sti");

    KeyboardFlushBuffer();

    return idtr;
}

#define PIC_EOI		0x20		/* End-of-interrupt command code */

void KiPicSendEoi(uint8_t irq)
{
	if(irq >= 8)
		outb(PIC2_COMMAND,PIC_EOI);
	
	outb(PIC1_COMMAND,PIC_EOI);
}

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
void KiPicRemap(int offset1, int offset2)
{
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	IOWait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	IOWait();
	outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	IOWait();
	outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	IOWait();
	outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	IOWait();
	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	IOWait();
	
	outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	IOWait();
	outb(PIC2_DATA, ICW4_8086);
	IOWait();

	// Unmask both PICs.
	outb(PIC1_DATA, 0);
	outb(PIC2_DATA, 0);
}

void KiIrqSetMask(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;

    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);        
}

void KiIrqClearMask(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;

    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);        
}
