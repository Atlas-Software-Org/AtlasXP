#include "pci.h"
#include <KiSimple.h>
#include <VMM/vmm.h>

uint32_t PciConfigRead32(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address =
        (1U << 31) |
        ((uint32_t)bus << 16) |
        ((uint32_t)device << 11) |
        ((uint32_t)func << 8) |
        (offset & 0xFC);
    outl(address, PCI_CONFIG_ADDRESS);
    return inl(PCI_CONFIG_DATA);    
}

static uint64_t PciBarGetPhysAddr(uint32_t BarLow) {
    if (BarLow & 0x1) return 0;

    return (uint64_t)(BarLow & ~0xFULL);
}

void* PciMapBAR(PciDevice* dev, int BarIndex, void* VirtAddrStart) {
    if (!dev || BarIndex < 0 || BarIndex >= 6) return NULL;

    uint32_t bar_low = dev->BAR[BarIndex];
    if (bar_low == 0 || (bar_low & 0x1)) return NULL;

    uint64_t phys = PciBarGetPhysAddr(bar_low);
    KiMapPresent(VirtAddrStart, (void*)phys);
    return VirtAddrStart;
}

PciDevice PciFindDevice(uint8_t bus, uint8_t device, uint8_t function) {
    PciDevice dev = {0};
    dev.bus = bus;
    dev.device = device;
    dev.function = function;

    uint32_t vendor_device = PciConfigRead32(bus, device, function, 0x00);
    if ((vendor_device & 0xFFFF) == 0xFFFF) {
        dev.vendor_id = 0xFFFF;
        dev.device_id = 0xFFFF;
        return dev;
    }

    dev.vendor_id = vendor_device & 0xFFFF;
    dev.device_id = (vendor_device >> 16) & 0xFFFF;

    for (int i = 0; i < 6; i++) {
        uint8_t offset = 0x10 + (i * 4);
        uint32_t bar_low = PciConfigRead32(bus, device, function, offset);
        dev.BAR[i] = bar_low;
    }

    dev.bar_type = PCI_BAR_32;
    for (int i = 0; i < 6; i++) {
        uint32_t bar = dev.BAR[i];
        if ((bar & 0x1) == 0) {
            uint8_t type = (bar >> 1) & 0x3;
            if (type == 0x2) {
                if (i < 5) {
                    uint32_t bar_high = PciConfigRead32(bus, device, function, 0x10 + (i + 1)*4);
                    uint64_t full_bar = ((uint64_t)bar_high << 32) | (bar & ~0xF);
                    dev.BAR[i] = (uint32_t)(full_bar & 0xFFFFFFFF);
                    dev.BAR[i+1] = (uint32_t)(full_bar >> 32);
                    dev.bar_type = PCI_BAR_64;
                    i++;
                }
            }
        }
    }

    return dev;
}

PciDevice PciFindDeviceClass(uint8_t class_code, uint8_t subclass, uint8_t prog_if, uint16_t vendor) {
    PciDevice dev = {0};
    dev.vendor_id = 0xFFFF;
    dev.device_id = 0xFFFF;

    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint32_t vendor_device = PciConfigRead32(bus, device, function, 0x00);
                if ((vendor_device & 0xFFFF) == 0xFFFF)
                    continue;

                uint16_t current_vendor = vendor_device & 0xFFFF;
                if (vendor != 0xFFFF && current_vendor != vendor)
                    continue;

                uint32_t class_data = PciConfigRead32(bus, device, function, 0x08);
                uint8_t current_class    = (class_data >> 24) & 0xFF;
                uint8_t current_subclass = (class_data >> 16) & 0xFF;
                uint8_t current_prog_if  = (class_data >> 8)  & 0xFF;

                if (current_class == class_code &&
                    current_subclass == subclass &&
                    current_prog_if == prog_if) {
                    return PciFindDevice(bus, device, function);
                }
            }
        }
    }

    return dev;
}

void* PciGetBARs(PciDevice *device) {
    if (!device) return NULL;
    return (void*)device->BAR;
}