/*
    Author: Adam Bassem
    Revision 0
    Patch 0
    Minor 0
    Major 0
    Atlas 0.0.7
*/

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

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}