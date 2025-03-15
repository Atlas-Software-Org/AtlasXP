#pragma once

#include <stdint.h>

// outb: Write a byte to the specified port
void outb(uint16_t port, uint8_t byte);

// inb: Read a byte from the specified port
uint8_t inb(uint16_t port);

// outw: Write a word (2 bytes) to the specified port
void outw(uint16_t port, uint16_t word);

// inw: Read a word (2 bytes) from the specified port
uint16_t inw(uint16_t port);

// outl: Write a double word (4 bytes) to the specified port
void outl(uint16_t port, uint32_t dword);

// inl: Read a double word (4 bytes) from the specified port
uint32_t inl(uint16_t port);

// Function to wait for port
void IOWait();