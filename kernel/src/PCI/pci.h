#ifndef PCI_H
#define PCI_H 1

#include <stdint.h>
#include <stddef.h>
#include <KiSimple.h>

typedef enum {
	PCI_CLASS_UNCLASSIFIED        = 0x00,
	PCI_CLASS_MASS_STORAGE        = 0x01,
	PCI_CLASS_NETWORK_CONTROLLER  = 0x02,
	PCI_CLASS_DISPLAY_CONTROLLER  = 0x03,
	PCI_CLASS_MULTIMEDIA          = 0x04,
	PCI_CLASS_MEMORY_CONTROLLER   = 0x05,
	PCI_CLASS_BRIDGE_DEVICE       = 0x06,
	PCI_CLASS_SIMPLE_COMM         = 0x07,
	PCI_CLASS_BASE_SYSTEM         = 0x08,
	PCI_CLASS_INPUT_DEVICE        = 0x09,
	PCI_CLASS_DOCKING_STATION     = 0x0A,
	PCI_CLASS_PROCESSOR           = 0x0B,
	PCI_CLASS_SERIAL_BUS          = 0x0C,
	PCI_CLASS_WIRELESS            = 0x0D,
	PCI_CLASS_INTELLIGENT_IO      = 0x0E,
	PCI_CLASS_SATELLITE_COMM      = 0x0F,
	PCI_CLASS_ENCRYPTION          = 0x10,
	PCI_CLASS_SIGNAL_PROCESSING   = 0x11,
	PCI_CLASS_PROCESSING_ACCEL    = 0x12,
	PCI_CLASS_NON_ESSENTIAL_INSTR = 0x13,
	PCI_CLASS_RESERVED            = 0x14,
	PCI_CLASS_UNASSIGNED          = 0xFF
} PciClassCode;

typedef enum {
	PCI_HEADER_TYPE_DEVICE        = 0x00,
	PCI_HEADER_TYPE_BRIDGE        = 0x01,
	PCI_HEADER_TYPE_CARDBUS       = 0x02,
	PCI_HEADER_TYPE_MULTI_FUNCTION= 0x80
} PciHeaderType;

typedef struct {
	uint8_t  bus;
	uint8_t  slot;
	uint8_t  function;

	uint16_t vendor_id;
	uint16_t device_id;

	uint16_t command;
	uint16_t status;

	uint8_t  revision_id;
	uint8_t  prog_if;
	uint8_t  subclass;
	uint8_t  class_code;

	PciClassCode class_type;

	uint8_t  cache_line_size;
	uint8_t  latency_timer;
	uint8_t  header_type_raw;
	PciHeaderType header_type;

	uint8_t  bist;

	uint32_t bar[6];
	void*    MMIOBase;
	uint32_t MMIOSize;
	uint8_t  MMIOBarIndex;
	uint8_t  Is64BitPciDevice : 1;

	uint32_t cardbus_cis_pointer;
	uint16_t subsystem_vendor_id;
	uint16_t subsystem_id;

	uint32_t expansion_rom_base_addr;

	uint8_t  capabilities_pointer;

	uint8_t  interrupt_line;
	uint8_t  interrupt_pin;

	uint8_t  min_grant;
	uint8_t  max_latency;
} PciDevice_t;
extern const PciDevice_t PCI_DEVICE_NONE;

uint16_t PciCheckVendor(uint8_t bus, uint8_t slot);
PciDevice_t PciFindDeviceByID(uint16_t vendor, uint16_t device);
PciDevice_t PciFindDeviceByClass(uint8_t class_code, uint8_t subclass);
PciDevice_t PciFindDeviceByLocation(uint8_t bus, uint8_t slot, uint8_t func);
PciDevice_t PciReadFullDevice(uint8_t bus, uint8_t slot, uint8_t func);
void PciGetDeviceMMIORegion(PciDevice_t* dev);
const char* PciGetDeviceManufacturer(const PciDevice_t* dev);
const char* PciGetDeviceName(const PciDevice_t* dev);
void PciLogDeviceInfo(const PciDevice_t* dev);
void PciListDevicesAttached(void);

#endif /* PCI_H */
