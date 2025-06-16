#ifndef PCI_H
#define PCI_H 1

#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

typedef struct {
    uint64_t BAR0;
    uint64_t BAR1;
    uint64_t BAR2;
    uint64_t BAR3;
    uint64_t BAR4;
    uint64_t BAR5;
} BARs64;

typedef struct {
    uint32_t BAR0;
    uint32_t BAR1;
    uint32_t BAR2;
    uint32_t BAR3;
    uint32_t BAR4;
    uint32_t BAR5;
} BARs32;

#define PCI_BAR_32 0
#define PCI_BAR_64 1

typedef struct {
    uint32_t BAR[6];
    uint8_t bus, device, function;
    uint16_t vendor_id;
    uint16_t device_id;
    int bar_type;
} PciDevice;

uint32_t PciConfigRead32(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
void* PciMapBAR(PciDevice* dev, int BarIndex, void* VirtAddrStart);

PciDevice PciFindDevice(uint8_t bus, uint8_t device, uint8_t function);
PciDevice PciFindDeviceClass(uint8_t class_code, uint8_t subclass, uint8_t prog_if, uint16_t vendor);
void* PciGetBARs(PciDevice *device);

#endif /* PCI_H */