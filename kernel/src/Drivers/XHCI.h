#ifndef XHCI_H
#define XHCI_H 1

#include <PCI/pci.h>

void xHciInit(PciDevice_t* UsbController);

#endif /* XHCI_H */
