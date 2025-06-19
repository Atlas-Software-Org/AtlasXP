#include "../ATA.h"

extern uint8_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t val);
extern uint16_t inw(uint16_t port);
extern void outw(uint16_t port, uint16_t val);

static void KiAtaWaitBusy() {
    while (inb(ATA_PRIMARY_IO + ATA_REG_STATUS) & ATA_SR_BSY);
}

static void KiAtaWaitDrq() {
    while (!(inb(ATA_PRIMARY_IO + ATA_REG_STATUS) & ATA_SR_DRQ));
}

void KiAtaSelectDrive(uint8_t drive) {
    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | (drive << 4));
}

void KiAtaReadSector(uint32_t lba, uint8_t* buffer) {
    KiAtaWaitBusy();
    KiAtaSelectDrive(0);

    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 1);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, (uint8_t)(lba));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    KiAtaWaitBusy();
    KiAtaWaitDrq();

    for (int i = 0; i < 256; i++) {
        ((uint16_t*)buffer)[i] = inw(ATA_PRIMARY_IO + ATA_REG_DATA);
    }
}

void KiAtaWriteSector(uint32_t lba, uint8_t* buffer) {
    KiAtaWaitBusy();
    KiAtaSelectDrive(0);

    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 1);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, (uint8_t)lba);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));

    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

    KiAtaWaitBusy();
    KiAtaWaitDrq();

    for (int i = 0; i < 256; i++) {
        outw(ATA_PRIMARY_IO + ATA_REG_DATA, ((uint16_t*)buffer)[i]);
    }

    KiAtaWaitBusy();
}

void KiAtaReadSectors(uint32_t startSector, uint32_t sectors, uint8_t* out) {
    for (uint32_t i = 0; i < sectors; i++) {
        KiAtaReadSector(startSector + i, out + (i * ATA_SECTOR_SIZE));
    }
}

void KiAtaWriteSectors(uint32_t startSector, uint32_t sectors, uint8_t* in) {
    for (uint32_t i = 0; i < sectors; i++) {
        KiAtaWriteSector(startSector + i, in + (i * ATA_SECTOR_SIZE));
    }
}
