#include "acpi.h"

void* CurrentRsdp = NULL;

void AcpiSetRsdp(void* rsdp) {
    CurrentRsdp = rsdp;
}

const char CorrectSignature[8] = "RSD PTR ";

void* AcpiFindTable(char TableSignature[4]) {
    if (CurrentRsdp == NULL) return NULL;
    RSDP_t* RSDP = (RSDP_t*)CurrentRsdp;
    XSDP_t* XSDP = (XSDP_t*)CurrentRsdp;

    for (int i = 0; i < 8; i++) {
        if (RSDP->Signature[i] != CorrectSignature[i])
            return NULL;
    }

    if (RSDP->Revision == 0) {
        goto Acpi1;
    } else if (RSDP->Revision == 2) {
        goto Acpi2_6;
    } else {
        return NULL;
    }

    void* RetValue = NULL;

Acpi1:
    KiPanic(1, "ASNU-Kernel doesn't support ACPI 1.0");
    RetValue = NULL;
Acpi2_6:
    XSDT *xsdt = (XSDT*)XSDP->XsdtAddress;

    int entries = (xsdt->h.Length - sizeof(xsdt->h)) / 8;

    for (int i = 0; i < entries; i++) {
        ACPISDTHeader *h = (ACPISDTHeader*)xsdt->PointerToOtherSDT[i];
        if (!strncmp(h->Signature, TableSignature, 4))
            RetValue = (void*)h;
            goto AcpiEnd;
    }
AcpiEnd:
    return RetValue;
}

void AcpiListTables() {
    if (CurrentRsdp == NULL) return NULL;
    RSDP_t* RSDP = (RSDP_t*)CurrentRsdp;
    XSDP_t* XSDP = (XSDP_t*)CurrentRsdp;

    for (int i = 0; i < 8; i++) {
        if (RSDP->Signature[i] != CorrectSignature[i])
            return NULL;
    }

    if (RSDP->Revision == 0) {
        goto Acpi1;
    } else if (RSDP->Revision == 2) {
        goto Acpi2_6;
    } else {
        return NULL;
    }

    void* RetValue = NULL;

Acpi1:
    KiPanic(1, "ASNU-Kernel doesn't support ACPI 1.0");
    RetValue = NULL;
Acpi2_6:
    XSDT *xsdt = (XSDT*)XSDP->XsdtAddress;

    int entries = (xsdt->h.Length - sizeof(xsdt->h)) / 8;

    for (int i = 0; i < entries; i++) {
        ACPISDTHeader *h = (ACPISDTHeader*)xsdt->PointerToOtherSDT[i];
        printk("Found [%4s] at SDT index %d\n\r", h->Signature, i);
    }
AcpiEnd:
    return RetValue;
}
