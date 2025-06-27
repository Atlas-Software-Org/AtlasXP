#ifndef AHCI_H
#define AHCI_H 1

#include <KiSimple.h>
#include <stdint.h>

#define AHCI_CMD_READ_DMA_EXT 0x25

#define AHCI_PORT_CMD_ST   (1 << 0)
#define AHCI_PORT_CMD_FRE  (1 << 4)
#define AHCI_PORT_CMD_FR   (1 << 14)
#define AHCI_PORT_CMD_CR   (1 << 15)

#define HBA_PxIS_TFES      (1 << 30)

void AhciSendZeroRead(volatile uint32_t* abar);

#endif /* AHCI_H */
