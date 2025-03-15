#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <HtKernelUtils/io.h>

__attribute__((noreturn))
void exception_handler(void);

#define IDT_MAX_DESCRIPTORS 256
#define GDT_OFFSET_KERNEL_CODE 0x08

typedef struct {
	uint16_t	limit;
	uint64_t	base;
} __attribute__((packed)) idtr_t;

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);
idtr_t idt_init(void);
void init_exceptions(void);

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

void PIC_sendEOI(uint8_t irq);

/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(int offset1, int offset2);

void pic_disable(void);

void IRQ_set_mask(uint8_t IRQline);
void IRQ_clear_mask(uint8_t IRQline);

/* Helper func */
static uint16_t __pic_get_irq_reg(int ocw3);
/* Returns the combined value of the cascaded PICs irq request register */
uint16_t pic_get_irr(void);
/* Returns the combined value of the cascaded PICs in-service register */
uint16_t pic_get_isr(void);

void NMI_enable();
void NMI_disable();