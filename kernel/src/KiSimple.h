#ifndef KISIMPLE_H
#define KISIMPLE_H 1

#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>
#include <printk/printk.h>

void outb(uint16_t port, uint8_t byte);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t word);
uint16_t inw(uint16_t port);
void outl(uint16_t port, uint32_t dword);
uint32_t inl(uint16_t port);
void IOWait();
void insw(uint16_t port, void* addr, int count);
void outsw(uint16_t port, const void* addr, int count);

#endif /* KISIMPLE_H */