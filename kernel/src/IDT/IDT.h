#pragma once

#include <stdint.h>
#include <stddef.h>

__attribute__((noreturn)) extern "C" void exception_handler(void);
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);
void idt_init(void);
void idt_reinit(void);
