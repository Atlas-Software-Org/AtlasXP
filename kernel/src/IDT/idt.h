/*
    Author: Adam Bassem
    Revision 0
    Patch 0
    Minor 0
    Major 0
    Atlas 0.0.7
*/

#ifndef IDT_H
#define IDT_H 1

#include <stdint.h>
#include <stdbool.h>
#include <KiSimple.h>

#include <Drivers/PS2Keyboard.h>

__attribute__((noreturn))
void KiExceptionHandler(int exception);

#define IDT_MAX_DESCRIPTORS 256
#define GDT_OFFSET_KERNEL_CODE 0x08

typedef struct {
	uint16_t	limit;
	uint64_t	base;
} __attribute__((packed)) idtr_t;

void KiIdtSetDesc(uint8_t vector, void* isr, uint8_t flags);
idtr_t KiIdtInit(void);
void KiInitExceptions(void);

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

void KiPicSendEoi(uint8_t irq);

/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
void KiPicRemap(int offset1, int offset2);

void KiIrqSetMask(uint8_t IRQline);
void KiIrqClearMask(uint8_t IRQline);

#endif /* IDT_H */