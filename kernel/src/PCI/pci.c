#include "pci.h"

const PciDevice_t PCI_DEVICE_NONE = { .vendor_id = 0xFFFF };

uint8_t PciConfigReadByte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
	uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
		(func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
	outl(0xCF8, address);
	uint32_t tmp = inl(0xCFC);
	return (uint8_t)((tmp >> ((offset & 3) * 8)) & 0xFF);
}

static uint16_t PciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
	uint32_t address;
	uint32_t lbus = (uint32_t)bus;
	uint32_t lslot = (uint32_t)slot;
	uint32_t lfunc = (uint32_t)func;
	uint16_t tmp = 0;

	/* Create configuration address */
	address = (uint32_t)((lbus << 16) | (lslot << 11) |
				(lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

	/* Write out the address */
	outl(0xCF8, address);
	/* Read in the data */
	/* (offset & 2) * 8) = 0 will choose the first word of the 32-bit register */
	tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
	return tmp;
}

uint32_t PciConfigReadDword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
	uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
		(func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
	outl(0xCF8, address);
	return inl(0xCFC);
}

void PciConfigWriteDword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
	uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
		(func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
	outl(0xCF8, address);
	outl(0xCFC, value);
}

uint16_t PciCheckVendor(uint8_t bus, uint8_t slot) {
	uint16_t vendor = PciConfigReadWord(bus, slot, 0, 0);
	return vendor;
}

PciDevice_t PciFindDeviceByID(uint16_t vendor, uint16_t device) {
	for (uint8_t bus = 0; bus < 256; bus++) {
		for (uint8_t slot = 0; slot < 32; slot++) {
			for (uint8_t func = 0; func < 8; func++) {
				uint16_t v = PciConfigReadWord(bus, slot, func, 0x00);
				if (v == 0xFFFF) continue;
				uint16_t d = PciConfigReadWord(bus, slot, func, 0x02);
				if (v == vendor && d == device) {
					PciDevice_t dev = PciReadFullDevice(bus, slot, func);
					return dev;
				}
				if (func == 0) break;
			}
		}
	}
	return PCI_DEVICE_NONE;
}

PciDevice_t PciFindDeviceByClass(uint8_t class_code, uint8_t subclass) {
	for (uint8_t bus = 0; bus < 256; bus++) {
		for (uint8_t slot = 0; slot < 32; slot++) {
			for (uint8_t func = 0; func < 8; func++) {
				uint16_t v = PciConfigReadWord(bus, slot, func, 0x00);
				if (v == 0xFFFF) continue;

				uint8_t cls = PciConfigReadByte(bus, slot, func, 0x0B);
				uint8_t sub = PciConfigReadByte(bus, slot, func, 0x0A);

				if (cls == class_code && (subclass == 0xFF || sub == subclass)) {
					return PciReadFullDevice(bus, slot, func);
				}

				if (func == 0) break;
			}
		}
	}
	return PCI_DEVICE_NONE;
}

PciDevice_t PciFindDeviceByLocation(uint8_t bus, uint8_t slot, uint8_t func) {
	uint16_t vendor = PciConfigReadWord(bus, slot, func, 0x00);
	if (vendor == 0xFFFF) return PCI_DEVICE_NONE;
	return PciReadFullDevice(bus, slot, func);
}

PciDevice_t PciReadFullDevice(uint8_t bus, uint8_t slot, uint8_t func) {
	PciDevice_t dev = {0};
	dev.bus = bus;
	dev.slot = slot;
	dev.function = func;

	dev.vendor_id = PciConfigReadWord(bus, slot, func, 0x00);
	dev.device_id = PciConfigReadWord(bus, slot, func, 0x02);
	dev.command = PciConfigReadWord(bus, slot, func, 0x04);
	dev.status = PciConfigReadWord(bus, slot, func, 0x06);
	dev.revision_id = PciConfigReadByte(bus, slot, func, 0x08);
	dev.prog_if = PciConfigReadByte(bus, slot, func, 0x09);
	dev.subclass = PciConfigReadByte(bus, slot, func, 0x0A);
	dev.class_code = PciConfigReadByte(bus, slot, func, 0x0B);
	dev.class_type = (PciClassCode)dev.class_code;
	dev.cache_line_size = PciConfigReadByte(bus, slot, func, 0x0C);
	dev.latency_timer = PciConfigReadByte(bus, slot, func, 0x0D);
	dev.header_type_raw = PciConfigReadByte(bus, slot, func, 0x0E);
	dev.header_type = (PciHeaderType)(dev.header_type_raw & 0x7F);
	dev.bist = PciConfigReadByte(bus, slot, func, 0x0F);

	for (int i = 0; i < 6; i++)
		dev.bar[i] = PciConfigReadDword(bus, slot, func, 0x10 + i * 4);

	dev.cardbus_cis_pointer = PciConfigReadDword(bus, slot, func, 0x28);
	dev.subsystem_vendor_id = PciConfigReadWord(bus, slot, func, 0x2C);
	dev.subsystem_id = PciConfigReadWord(bus, slot, func, 0x2E);
	dev.expansion_rom_base_addr = PciConfigReadDword(bus, slot, func, 0x30);
	dev.capabilities_pointer = PciConfigReadByte(bus, slot, func, 0x34);
	dev.interrupt_line = PciConfigReadByte(bus, slot, func, 0x3C);
	dev.interrupt_pin = PciConfigReadByte(bus, slot, func, 0x3D);
	dev.min_grant = PciConfigReadByte(bus, slot, func, 0x3E);
	dev.max_latency = PciConfigReadByte(bus, slot, func, 0x3F);

	return dev;
}

void PciGetDeviceMMIORegion(PciDevice_t* dev) {
	for (int i = 0; i < 6; i++) {
		uint32_t bar = dev->bar[i];
		if (bar == 0 || (bar & 0x1)) continue;

		uint32_t base_low = bar & ~0xF;
		dev->Is64BitPciDevice = 0;

		if ((bar & 0x6) == 0x4) {
			uint32_t bar_high = dev->bar[i + 1];
			uint64_t full_base = ((uint64_t)bar_high << 32) | base_low;

			dev->MMIOBase = (void*)(uintptr_t)full_base;
			dev->MMIOBarIndex = i;
			dev->Is64BitPciDevice = 1;

			// Detect size
			PciConfigWriteDword(dev->bus, dev->slot, dev->function, 0x10 + i * 4, 0xFFFFFFFF);
			PciConfigWriteDword(dev->bus, dev->slot, dev->function, 0x10 + (i + 1) * 4, 0xFFFFFFFF);

			uint32_t size_low  = PciConfigReadDword(dev->bus, dev->slot, dev->function, 0x10 + i * 4);
			uint32_t size_high = PciConfigReadDword(dev->bus, dev->slot, dev->function, 0x10 + (i + 1) * 4);

			PciConfigWriteDword(dev->bus, dev->slot, dev->function, 0x10 + i * 4, bar);
			PciConfigWriteDword(dev->bus, dev->slot, dev->function, 0x10 + (i + 1) * 4, bar_high);

			uint64_t size_mask = ((uint64_t)size_high << 32) | (size_low & ~0xF);
			dev->MMIOSize = (uint32_t)(~size_mask + 1);

			return;
		}

		// 32-bit MMIO BAR
		dev->MMIOBase = (void*)(uintptr_t)base_low;
		dev->MMIOBarIndex = i;

		PciConfigWriteDword(dev->bus, dev->slot, dev->function, 0x10 + i * 4, 0xFFFFFFFF);
		uint32_t size_mask = PciConfigReadDword(dev->bus, dev->slot, dev->function, 0x10 + i * 4);
		PciConfigWriteDword(dev->bus, dev->slot, dev->function, 0x10 + i * 4, bar);

		dev->MMIOSize = ~(size_mask & ~0xF) + 1;

		return;
	}

	dev->MMIOBase = NULL;
	dev->MMIOSize = 0;
	dev->MMIOBarIndex = 0xFF;
	dev->Is64BitPciDevice = 0;
}

const char* PciGetDeviceManufacturer(const PciDevice_t* dev) {
	switch (dev->vendor_id) {
		case 0x8086: return "Intel Corporation";
		case 0x10DE: return "NVIDIA Corporation";
		case 0x1002: return "Advanced Micro Devices (AMD)";
		case 0x1022: return "AMD (Advanced Micro Devices)";
		case 0x10EC: return "Realtek Semiconductor Corp.";
		case 0x14E4: return "Broadcom Inc.";
		case 0x104C: return "Texas Instruments";
		case 0x168C: return "Qualcomm Atheros";
		case 0x1969: return "Qualcomm Attansic (Atheros)";
		case 0x10B7: return "3Com Corporation";
		case 0x1217: return "O2 Micro Inc.";
		case 0x1033: return "NEC Corporation";
		case 0x1106: return "VIA Technologies, Inc.";
		case 0x10B9: return "ULi / ALi Corporation";
		case 0x10F1: return "Silicon Integrated Systems (SiS)";
		case 0x1AF4: return "Red Hat, Inc. (VirtIO)";
		case 0x1B36: return "QEMU / Red Hat, Inc.";
		case 0x15AD: return "VMware, Inc.";
		case 0x1274: return "Ensoniq (Creative)";
		case 0x1095: return "Silicon Image, Inc.";
		case 0x105A: return "Promise Technology, Inc.";
		case 0x1C5C: return "TP-Link Technologies Co., Ltd.";
		case 0x13FE: return "Kingston Technology Company, Inc.";
		case 0x1A03: return "ASPEED Technology, Inc.";
		case 0x1B4B: return "Marvell Technology Group Ltd.";
		case 0x14F1: return "Conexant Systems, Inc.";
		case 0x1000: return "LSI Logic / Symbios Logic";
		case 0x111D: return "IDT (Integrated Device Technology)";
		case 0x1912: return "Renesas Electronics Corporation";
		case 0x1AE0: return "Google LLC";
		case 0x1C58: return "ASMedia Technology Inc.";
		case 0x10A9: return "SGI (Silicon Graphics)";
		case 0x17CB: return "Skyworks Solutions, Inc.";
		case 0x11AB: return "Marvell Semiconductor, Inc.";
		case 0x1DE1: return "Tekram Technology Co., Ltd.";
		case 0x1B21: return "ASMedia Technology Inc.";
		case 0x1D6A: return "JMicron Technology Corp.";
		case 0x1E0F: return "Shenzhen Evision Semiconductor Ltd.";
	}
	return "Unknown Vendor";
}

const char* PciGetDeviceName(const PciDevice_t* dev) {
	if (dev->vendor_id == 0x8086) {
		switch (dev->device_id) {
			case 0x100E: return "Intel PRO/1000 MT Desktop (82540EM)";
			case 0x10D3: return "Intel 82574L Gigabit Ethernet";
			case 0x10F5: return "Intel 82547GI Gigabit Ethernet";
			case 0x1502: return "Intel 82579LM Gigabit Ethernet";
			case 0x10FB: return "Intel 82599 10-Gigabit Ethernet";
			case 0x2922: return "Intel ICH9 SATA AHCI Controller";
			case 0x1237: return "Intel 82441FX PMC (Q35)";
			case 0x7000: return "Intel 82371SB PIIX3 ISA Bridge";
			case 0x2415: return "Intel 82801AA AC'97 Audio Controller";
		}
	}

	if (dev->vendor_id == 0x10EC) {
		switch (dev->device_id) {
			case 0x8139: return "Realtek RTL8139 Fast Ethernet";
			case 0x8168: return "Realtek RTL8111/8168/8411 PCIe Gigabit Ethernet";
			case 0x5229: return "Realtek PCI-E Card Reader";
		}
	}

	if (dev->vendor_id == 0x10DE) {
		switch (dev->device_id) {
			case 0x1C82: return "NVIDIA GeForce GTX 1050";
			case 0x1C03: return "NVIDIA GeForce GTX 1060 (6GB)";
			case 0x1C02: return "NVIDIA GeForce GTX 1060 (3GB)";
			case 0x1B80: return "NVIDIA GeForce GTX 1080";
			case 0x2486: return "NVIDIA GeForce RTX 3060";
		}
	}

	if (dev->vendor_id == 0x1002) {
		switch (dev->device_id) {
			case 0x67DF: return "AMD Radeon RX 580";
			case 0x7340: return "AMD Radeon RX 6800";
			case 0x743F: return "AMD Radeon RX 7900 XT";
		}
	}

	if (dev->vendor_id == 0x1AF4) {
		switch (dev->device_id) {
			case 0x1000: return "VirtIO Network Device";
			case 0x1001: return "VirtIO Block Device";
			case 0x1002: return "VirtIO Balloon Device";
			case 0x1003: return "VirtIO Console";
			case 0x1041: return "VirtIO SCSI Device";
			case 0x1050: return "VirtIO Input Device";
		}
	}

	if (dev->vendor_id == 0x15AD) {
		switch (dev->device_id) {
			case 0x07A0: return "VMware VMXNET3 Ethernet Controller";
			case 0x0740: return "VMware SVGA II Adapter";
			case 0x0405: return "VMware Virtual USB Host Controller";
		}
	}

	if (dev->vendor_id == 0x1B36) {
		switch (dev->device_id) {
			case 0x0001: return "QEMU Standard VGA";
			case 0x0002: return "QEMU Ethernet Controller";
		}
	}

	if (dev->vendor_id == 0x14E4) {
		switch (dev->device_id) {
			case 0x1659: return "Broadcom NetXtreme BCM5721 Gigabit Ethernet";
		}
	}

	if (dev->vendor_id == 0x1022) {
		switch (dev->device_id) {
			case 0x2000: return "AMD Lance Ethernet Controller";
			case 0x7901: return "AMD FCH SATA Controller";
		}
	}

	return "Unknown Device";
}

void PciLogDeviceInfo(const PciDevice_t* dev) {
	const char* vendor_name = PciGetDeviceManufacturer(dev);
	const char* device_name = PciGetDeviceName(dev);

	printk("%s / 0x%04X / %s / %s\n",
		vendor_name,
		dev->vendor_id,
		device_name,
		dev->Is64BitPciDevice ? "64-bit" : "32-bit");

	printk("Class: 0x%02X / Subclass: 0x%02X / Location: B:%u S:%u F:%u\n",
		dev->class_code,
		dev->subclass,
		dev->bus,
		dev->slot,
		dev->function);

	if (dev->MMIOBase != NULL && dev->MMIOSize > 0) {
		printk("MMIO Base: 0x%p / Size: 0x%X / BAR%d\n",
			dev->MMIOBase,
			dev->MMIOSize,
			dev->MMIOBarIndex);
	} else {
		printk("No MMIO region mapped\n");
	}
}

void PciListDevicesAttached(void) {
	uint32_t total_possible = 8 * 32 * 8; /* Using 8 buses instead of 256 since thats the average on real hardware */
	uint32_t found = 0;
	
							/* 256 */
	for (uint8_t bus = 0; bus < 8; bus++) {
		for (uint8_t slot = 0; slot < 32; slot++) {
			for (uint8_t func = 0; func < 8; func++) {
				uint16_t vendor = PciConfigReadWord(bus, slot, func, 0x00);
				if (vendor == 0xFFFF) continue;

				PciDevice_t dev = PciReadFullDevice(bus, slot, func);
				PciGetDeviceMMIORegion(&dev);
				PciLogDeviceInfo(&dev);
				found++;

				if (func == 0 && !(dev.header_type_raw & 0x80))
					break;

				printk("PCI scan coverage: %u/%u\n", found, total_possible);
			}
		}
	}
}
