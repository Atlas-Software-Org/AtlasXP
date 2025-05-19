#include <KiSimple.h>
#include <stdint.h>

void outb(uint16_t port, uint8_t byte) {
    asm volatile ("outb %0, %1" : : "a"(byte), "dN"(port));
}
uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "dN"(port));
    return result;
}
void outw(uint16_t port, uint16_t word) {
    asm volatile ("outw %0, %1" : : "a"(word), "dN"(port));
}
uint16_t inw(uint16_t port) {
    uint16_t result;
    asm volatile ("inw %1, %0" : "=a"(result) : "dN"(port));
    return result;
}
void outl(uint16_t port, uint32_t dword) {
    asm volatile ("outl %0, %1" : : "a"(dword), "dN"(port));
}
uint32_t inl(uint16_t port) {
    uint32_t result;
    asm volatile ("inl %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

void IOWait() {
    outb(0x80, 0x00);
}

inline void insw(uint16_t port, void* addr, int count) {
    asm volatile ("rep insw"
                  : "+D"(addr), "+c"(count) // Destination address and counter
                  : "d"(port)               // Source port
                  : "memory");              // Clobbers memory
}

inline void outsw(uint16_t port, const void* addr, int count) {
    asm volatile ("rep outsw"
                  : "+S"(addr), "+c"(count) // Source address and counter
                  : "d"(port));             // Destination port
}