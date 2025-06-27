#ifndef PCI_H
#define PCI_H 1

#include <stdint.h>

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_MAKE_ADDRESS(bus, dev, func, offset) \
    (uint32_t)(0x80000000 | ((bus & 0xFF) << 16) | ((dev & 0x1F) << 11) | ((func & 0x07) << 8) | (offset & 0xFC))

uint8_t PciRead8(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset);
uint16_t PciRead16(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset);
uint32_t PciRead32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset);
void PciWrite32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint32_t value);

uint32_t PciGetBar(uint8_t bus, uint8_t dev, uint8_t func, uint8_t bar_index);
void PciScanAllBuses(void (*callback)(uint8_t bus, uint8_t dev, uint8_t func));

int PciFindDeviceByClass(uint8_t class_code, uint8_t subclass, uint8_t prog_if, uint8_t* out_bus, uint8_t* out_dev, uint8_t* out_func);

#endif /* PCI_H */
