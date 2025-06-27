#include <stdint.h>
#include <KiSimple.h>
#include <PMM/pmm.h>
#include <VMM/vmm.h>

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_MAKE_ADDRESS(bus, dev, func, offset) \
    (uint32_t)(0x80000000 | ((bus & 0xFF) << 16) | ((dev & 0x1F) << 11) | ((func & 0x07) << 8) | (offset & 0xFC))

uint8_t PciRead8(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    uint32_t address = PCI_MAKE_ADDRESS(bus, dev, func, offset);
    outl(PCI_CONFIG_ADDR, address);
    return (inl(PCI_CONFIG_DATA) >> ((offset & 3) * 8)) & 0xFF;
}

uint16_t PciRead16(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    uint32_t address = PCI_MAKE_ADDRESS(bus, dev, func, offset);
    outl(PCI_CONFIG_ADDR, address);
    return (inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF;
}

uint32_t PciRead32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    uint32_t address = PCI_MAKE_ADDRESS(bus, dev, func, offset);
    outl(PCI_CONFIG_ADDR, address);
    return inl(PCI_CONFIG_DATA);
}

void PciWrite32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = PCI_MAKE_ADDRESS(bus, dev, func, offset);
    outl(PCI_CONFIG_ADDR, address);
    outl(PCI_CONFIG_DATA, value);
}

uint32_t PciGetBar(uint8_t bus, uint8_t dev, uint8_t func, uint8_t bar_index) {
    return PciRead32(bus, dev, func, 0x10 + (bar_index * 4));
}

void PciScanAllBuses(void (*callback)(uint8_t, uint8_t, uint8_t)) {
    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t dev = 0; dev < 32; dev++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint16_t vendor = PciRead16(bus, dev, func, 0x00);
                if (vendor == 0xFFFF)
                    continue;
                callback(bus, dev, func);
                uint8_t header_type = PciRead8(bus, dev, func, 0x0E);
                if ((header_type & 0x80) == 0 && func > 0)
                    break;
            }
        }
    }
}

int PciFindDeviceByClass(uint8_t class_code, uint8_t subclass, uint8_t prog_if,
                         uint8_t* out_bus, uint8_t* out_dev, uint8_t* out_func) {
    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t dev = 0; dev < 32; dev++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint16_t vendor = PciRead16(bus, dev, func, 0x00);
                if (vendor == 0xFFFF)
                    continue;

                uint8_t classcode = PciRead8(bus, dev, func, 0x0B);
                uint8_t subclass_code = PciRead8(bus, dev, func, 0x0A);
                uint8_t progif = PciRead8(bus, dev, func, 0x09);

                if (classcode == class_code && subclass_code == subclass && progif == prog_if) {
                    if (out_bus) *out_bus = bus;
                    if (out_dev) *out_dev = dev;
                    if (out_func) *out_func = func;
                    return 1;
                }

                uint8_t headertype = PciRead8(bus, dev, func, 0x0E);
                if ((headertype & 0x80) == 0 && func > 0)
                    break;
            }
        }
    }
    return 0;
}
