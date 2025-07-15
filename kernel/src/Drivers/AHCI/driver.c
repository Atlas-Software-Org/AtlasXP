#include "../AHCI.h"

#define AHCI_DEV_SATA        0x00000001
#define HBA_PxCMD_ST         (1 << 0)
#define HBA_PxCMD_FRE        (1 << 4)
#define HBA_PxCMD_FR         (1 << 14)
#define HBA_PxCMD_CR         (1 << 15)
#define FIS_TYPE_REG_H2D     0x27
#define ATA_CMD_READ_DMA     0x25
#define ATA_CMD_WRITE_DMA     0xCA

static HBA_MEM* hba = 0;
static HBAPort* active_port = 0;

static void PrintPortStatus(HBAPort* port) {
    printk("PxCMD  = %08X\n\r", port->cmd);
    printk("PxTFD  = %08X\n\r", port->tfd);
    printk("PxIS   = %08X\n\r", port->is);
    printk("PxSERR = %08X\n\r", port->serr);
    printk("PxCI   = %08X\n\r", port->ci);
}

static int check_port_type(HBAPort* port) {
    uint32_t ssts = port->ssts;
    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;
    return (det == 3 && ipm == 1);
}

void AhciInit(PciDevice_t* dev) {
    if (dev->class_code != PCI_CLASS_MASS_STORAGE || dev->subclass != AHCI_SUBCLASS || dev->prog_if != AHCI_PROGIF)
        return;

    uintptr_t phys = dev->bar[dev->MMIOBarIndex] & ~0xF;
    uintptr_t size = dev->MMIOSize;
    for (uintptr_t offset = 0; offset < size; offset += 0x1000)
        KiMMap((void*)(phys + offset), (void*)(phys + offset), MMAP_PRESENT | MMAP_RW);

    hba = (HBA_MEM*)phys;
    uint32_t pi = hba->pi;
    for (int i = 0; i < 32; i++) {
        if ((pi & (1 << i)) && check_port_type(&hba->ports[i])) {
            active_port = &hba->ports[i];
            break;
        }
    }
}

static void stop_cmd(HBAPort* port) {
    port->cmd &= ~HBA_PxCMD_ST;
    port->cmd &= ~HBA_PxCMD_FRE;
    while (port->cmd & (HBA_PxCMD_FR | HBA_PxCMD_CR));
}

static void start_cmd(HBAPort* port) {
    while (port->cmd & HBA_PxCMD_CR);
    port->cmd |= HBA_PxCMD_FRE | HBA_PxCMD_ST;
}

void AhciReadSector(uint64_t lba, void* buffer) {
    if (!active_port) return;

    stop_cmd(active_port);

    uintptr_t clb = 0x100000;
    uintptr_t fb = 0x101000;
    uintptr_t ctba = 0x102000;

    KiMMap((void*)clb, (void*)clb, MMAP_PRESENT | MMAP_RW);
    KiMMap((void*)fb, (void*)fb, MMAP_PRESENT | MMAP_RW);
    KiMMap((void*)ctba, (void*)ctba, MMAP_PRESENT | MMAP_RW);

    HBA_CMD_HEADER* cmd_hdr = (HBA_CMD_HEADER*)clb;
    HBA_CMD_TABLE* cmd_tbl = (HBA_CMD_TABLE*)ctba;

    active_port->clb = clb;
    active_port->fb = fb;

    for (int i = 0; i < sizeof(HBA_CMD_TABLE); i++)
        ((uint8_t*)cmd_tbl)[i] = 0;

    cmd_hdr[0].cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
    cmd_hdr[0].w = 0;
    cmd_hdr[0].prdtl = 1;
    cmd_hdr[0].ctba = ctba;

    cmd_tbl->prdt_entry[0].dba = (uintptr_t)buffer;
    cmd_tbl->prdt_entry[0].dbau = 0;
    cmd_tbl->prdt_entry[0].dbc = 512 - 1;
    cmd_tbl->prdt_entry[0].i = 1;

    FIS_REG_H2D* fis = (FIS_REG_H2D*)cmd_tbl->cfis;
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->c = 1;
    fis->command = ATA_CMD_READ_DMA;
    fis->lba0 = (uint8_t)lba;
    fis->lba1 = (uint8_t)(lba >> 8);
    fis->lba2 = (uint8_t)(lba >> 16);
    fis->device = 1 << 6;
    fis->lba3 = (uint8_t)(lba >> 24);
    fis->lba4 = (uint8_t)(lba >> 32);
    fis->lba5 = (uint8_t)(lba >> 40);
    fis->countl = 1;
    fis->counth = 0;

    start_cmd(active_port);
    active_port->ci = 1;

    int timeout = 500000;
    for (int i = 0; i < timeout; i++) {
        if ((active_port->ci & 1) == 0)
            break;
        if (i == 100000 || i == 250000 || i == 400000)
            printk("AHCI: still waiting... (%d)\n\r", i);
    }

	if (active_port->ci & 1) {
	    printk("AHCI: timeout, dumping status...\n\r");
	    PrintPortStatus(active_port);
	}

    KiUMap((void*)clb);
    KiUMap((void*)fb);
    KiUMap((void*)ctba);
}

void AhciWriteSector(uint64_t lba, void* buffer) {
    if (!active_port) return;

    stop_cmd(active_port);

    uintptr_t clb = 0x100000;
    uintptr_t fb = 0x101000;
    uintptr_t ctba = 0x102000;

    KiMMap((void*)clb, (void*)clb, MMAP_PRESENT | MMAP_RW);
    KiMMap((void*)fb, (void*)fb, MMAP_PRESENT | MMAP_RW);
    KiMMap((void*)ctba, (void*)ctba, MMAP_PRESENT | MMAP_RW);

    HBA_CMD_HEADER* cmd_hdr = (HBA_CMD_HEADER*)clb;
    HBA_CMD_TABLE* cmd_tbl = (HBA_CMD_TABLE*)ctba;

    active_port->clb = clb;
    active_port->fb = fb;

    for (int i = 0; i < sizeof(HBA_CMD_TABLE); i++)
        ((uint8_t*)cmd_tbl)[i] = 0;

    cmd_hdr[0].cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
    cmd_hdr[0].w = 1; /* the most important thing that distincs write from read is this bit */
    cmd_hdr[0].prdtl = 1;
    cmd_hdr[0].ctba = ctba;

    cmd_tbl->prdt_entry[0].dba = (uintptr_t)buffer;
    cmd_tbl->prdt_entry[0].dbau = 0;
    cmd_tbl->prdt_entry[0].dbc = 512 - 1;
    cmd_tbl->prdt_entry[0].i = 1;

    FIS_REG_H2D* fis = (FIS_REG_H2D*)cmd_tbl->cfis;
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->c = 1;
    fis->command = ATA_CMD_WRITE_DMA;
    fis->lba0 = (uint8_t)lba;
    fis->lba1 = (uint8_t)(lba >> 8);
    fis->lba2 = (uint8_t)(lba >> 16);
    fis->device = 1 << 6;
    fis->lba3 = (uint8_t)(lba >> 24);
    fis->lba4 = (uint8_t)(lba >> 32);
    fis->lba5 = (uint8_t)(lba >> 40);
    fis->countl = 1;
    fis->counth = 0;

    start_cmd(active_port);
    active_port->ci = 1;

    int timeout = 500000;
    for (int i = 0; i < timeout; i++) {
        if ((active_port->ci & 1) == 0)
            break;
        if (i == 100000 || i == 250000 || i == 400000)
            printk("AHCI: still waiting... (%d)\n\r", i);
    }

	if (active_port->ci & 1) {
	    printk("AHCI: timeout, dumping status...\n\r");
	    PrintPortStatus(active_port);
	}

    KiUMap((void*)clb);
    KiUMap((void*)fb);
    KiUMap((void*)ctba);
}

int AhciBlindEchoTest(uint32_t lba, int verbose) {
	char buf[512] = {0};
	const char* msg = "Hello, World!\n\rASNU-Kernel AhciBlindEchoTest\n\r-THIS MESSAGE IS THE ECHO DATA SENT BY THE AHCI-DRIVER FOR THE TEST-\n\r\0"; /* \0 just to be 100% sure */
	for (int i = 0; i < 512; i++) {
		if (msg[i] == 0)
			break;
		else
			buf[i] = msg[i];
	}
	if (verbose)
		printk("Performing blind AHCI echo test\n\r");

	uint8_t lba_old_data[512] = {0}; /* POST TEST */
	uint8_t lba_new_data[512] = {0}; /* PAST TEST */

	AhciReadSector(lba, lba_new_data);
	AhciWriteSector(lba, (uint8_t*)buf);
	AhciReadSector(lba, lba_new_data);
	AhciWriteSector(lba, lba_old_data);

	int errors = 0;
	for (int i = 0; i < 512; i++) {
		if (buf[i] != lba_new_data[i]) {
			errors++;
			printk("Error %d found at offset %X, difference: %X, opposites: %X / %X (%c / %c)\n\r", errors, i, buf[i] - lba_new_data[i], buf[i], lba_new_data[i], buf[i], lba_new_data[i]);
		}
	}

	if (errors < 1 && verbose) {
		printk("AHCI blind echo test succeeded\n\r");
	}

	return errors;
}
