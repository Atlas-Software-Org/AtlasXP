#include "io.h"
#include <stdint.h>

// outb: Write a byte to the specified port
void outb(uint16_t port, uint8_t byte) {
    asm volatile ("outb %0, %1" : : "a"(byte), "dN"(port));
}

// inb: Read a byte from the specified port
uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

// outw: Write a word (2 bytes) to the specified port
void outw(uint16_t port, uint16_t word) {
    asm volatile ("outw %0, %1" : : "a"(word), "dN"(port));
}

// inw: Read a word (2 bytes) from the specified port
uint16_t inw(uint16_t port) {
    uint16_t result;
    asm volatile ("inw %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

// outl: Write a double word (4 bytes) to the specified port
void outl(uint16_t port, uint32_t dword) {
    asm volatile ("outl %0, %1" : : "a"(dword), "dN"(port));
}

// inl: Read a double word (4 bytes) from the specified port
uint32_t inl(uint16_t port) {
    uint32_t result;
    asm volatile ("inl %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

void IOWait() {
    outb(0x80, 0x00);
}