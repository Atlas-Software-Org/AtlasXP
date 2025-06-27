#include "../AHCI.h"
#include <PMM/pmm.h>
#include <VMM/vmm.h>

typedef struct {
    uint8_t  cfis[64];
    uint8_t  acmd[16];
    uint8_t  reserved[48];
} __attribute__((packed)) HBACommandTable;

typedef struct {
    uint16_t flags;
    uint16_t prdt_length;
    uint32_t prdbc;
    uint32_t ctba;
    uint32_t ctbau;
    uint32_t reserved[4];
} __attribute__((packed)) HBACommandHeader;

typedef struct {
    uint32_t dba;
    uint32_t dbau;
    uint32_t reserved0;
    uint32_t dbc_i;
} __attribute__((packed)) HBAPRDTEntry;

void AhciSendZeroRead(volatile uint32_t* abar) {
    volatile uint32_t* port = (volatile uint32_t*)(abar + (0x100 / 4)); // Port 0

    port[0] &= ~(AHCI_PORT_CMD_ST | AHCI_PORT_CMD_FRE);
    while (port[0] & (AHCI_PORT_CMD_CR | AHCI_PORT_CMD_FR)); // Wait until engine is fully stopped

    void* clb = KiPmmAlloc();
    void* ct  = KiPmmAlloc();
    memset(clb, 0, 4096);
    memset(ct,  0, 4096);

    KiMMap((void*)clb, (void*)clb, MMAP_PRESENT | MMAP_RW);
    KiMMap((void*)ct,  (void*)ct,  MMAP_PRESENT | MMAP_RW);

    port[1] = (uintptr_t)clb;
    port[2] = 0;

    HBACommandHeader* cmd = (HBACommandHeader*)clb;
    cmd->flags = (0 << 6) | (1 << 0); // PRDT length = 0, read
    cmd->prdt_length = 0;
    cmd->ctba  = (uintptr_t)ct;
    cmd->ctbau = 0;

    HBACommandTable* table = (HBACommandTable*)ct;
    uint8_t* cfis = table->cfis;

    cfis[0] = 0x27;               // FIS type: Register – Host to Device
    cfis[1] = 1 << 7;             // Command FIS
    cfis[2] = AHCI_CMD_READ_DMA_EXT;
    cfis[7] = 1 << 6;             // Device bit set (LBA mode)

    port[0] |= AHCI_PORT_CMD_FRE;
    port[0] |= AHCI_PORT_CMD_ST;

    port[6] = 1; // issue command slot 0

    while (port[6] & 1); // wait for command to complete

    if (port[4] & HBA_PxIS_TFES) {
        printk("AHCI read-zero failed\n");
        return;
    }

    printk("AHCI read-zero success\n");
}
