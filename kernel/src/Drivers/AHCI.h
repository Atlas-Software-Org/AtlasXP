#ifndef AHCI_H
#define AHCI_H 1

#include <KiSimple.h>
#include <PMM/pmm.h>
#include <VMM/vmm.h>
#include <PCI/pci.h>
#include <stdint.h>
#include <stddef.h>

#define HBA_PORT_DEV_PRESENT 0x3
#define HBA_PORT_IPM_ACTIVE  0x1
#define AHCI_DEV_SATA        0x00000001

#define HBA_PxCMD_ST         (1 << 0)
#define HBA_PxCMD_FRE        (1 << 4)
#define HBA_PxCMD_FR         (1 << 14)
#define HBA_PxCMD_CR         (1 << 15)

#define AHCI_BASE_CLASS      0x01
#define AHCI_SUBCLASS        0x06
#define AHCI_PROGIF          0x01

#define FIS_TYPE_REG_H2D 0x27

typedef struct {
    uint8_t  fis_type;
    uint8_t  pmport:4;
    uint8_t  rsv0:3;
    uint8_t  c:1;
    uint8_t  command;
    uint8_t  featurel;

    uint8_t  lba0;
    uint8_t  lba1;
    uint8_t  lba2;
    uint8_t  device;

    uint8_t  lba3;
    uint8_t  lba4;
    uint8_t  lba5;
    uint8_t  featureh;

    uint8_t  countl;
    uint8_t  counth;
    uint8_t  icc;
    uint8_t  control;

    uint8_t  rsv1[4];
} FIS_REG_H2D;

typedef struct {
    uint32_t dba;
    uint32_t dbau;
    uint32_t reserved0;
    uint32_t dbc:22;
    uint32_t reserved1:9;
    uint32_t i:1;
} HBA_PRDT_ENTRY;

typedef struct {
    uint8_t cfis[64];
    uint8_t acmd[16];
    uint8_t reserved[48];
    HBA_PRDT_ENTRY prdt_entry[1]; // for 512 bytes only, more can be added dynamically
} HBA_CMD_TABLE;

typedef struct {
    uint8_t  cfl:5;
    uint8_t  a:1;
    uint8_t  w:1;
    uint8_t  p:1;

    uint8_t  r:1;
    uint8_t  b:1;
    uint8_t  c:1;
    uint8_t  rsv0:1;
    uint8_t  pmp:4;

    uint16_t prdtl;
    uint32_t prdbc;
    uint32_t ctba;
    uint32_t ctbau;
    uint32_t reserved[4];
} HBA_CMD_HEADER;

typedef struct {
    uint32_t clb;
    uint32_t clbu;
    uint32_t fb;
    uint32_t fbu;
    uint32_t is;
    uint32_t ie;
    uint32_t cmd;
    uint32_t reserved0;
    uint32_t tfd;
    uint32_t sig;
    uint32_t ssts;
    uint32_t sctl;
    uint32_t serr;
    uint32_t sact;
    uint32_t ci;
    uint32_t sntf;
    uint32_t fbs;
    uint32_t reserved1[11];
    uint32_t vendor[4];
} HBAPort;

typedef struct {
    uint32_t cap;
    uint32_t ghc;
    uint32_t is;
    uint32_t pi;
    uint32_t vs;
    uint32_t ccc_ctl;
    uint32_t ccc_pts;
    uint32_t em_loc;
    uint32_t em_ctl;
    uint32_t cap2;
    uint32_t bohc;
    uint8_t  reserved[0xA0 - 0x2C];
    uint8_t  vendor[0x100 - 0xA0];
    HBAPort  ports[32];
} HBA_MEM;

void AhciInit(PciDevice_t* dev);
void AhciReadSector(uint64_t lba, void* buffer);
void AhciWriteSector(uint64_t lba, void* buffer);

int AhciBlindEchoTest(uint32_t lba, int verbose);

#endif /* AHCI_H */

