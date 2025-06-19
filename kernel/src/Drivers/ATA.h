#ifndef ATA_H
#define ATA_H 1

#include <KiSimple.h>
#include <stdint.h>

#define ATA_PRIMARY_IO  0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

#define ATA_REG_DATA       0
#define ATA_REG_ERROR      1
#define ATA_REG_FEATURES   1
#define ATA_REG_SECCOUNT0  2
#define ATA_REG_LBA0       3
#define ATA_REG_LBA1       4
#define ATA_REG_LBA2       5
#define ATA_REG_HDDEVSEL   6
#define ATA_REG_COMMAND    7
#define ATA_REG_STATUS     7

#define ATA_CMD_IDENTIFY     0xEC
#define ATA_CMD_READ_PIO     0x20
#define ATA_CMD_WRITE_PIO    0x30

#define ATA_SR_BSY  0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01
#define ATA_SR_DF   0x20

#define ATA_SECTOR_SIZE 512

void KiAtaSelectDrive(uint8_t drive);
void KiAtaReadSector(uint32_t lba, uint8_t* buffer);
void KiAtaWriteSector(uint32_t lba, uint8_t* buffer);
void KiAtaReadSectors(uint32_t startSector, uint32_t sectors, uint8_t* out);
void KiAtaWriteSectors(uint32_t startSector, uint32_t sectors, uint8_t* in);

#endif /* ATA_H */
