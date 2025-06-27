#ifndef ACPI_H
#define ACPI_H 1

#include <KiSimple.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
} __attribute__((packed)) RSDP_t;

typedef struct {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
    
    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t _rsv[3];
} __attribute__((packed)) XSDP_t;

typedef struct {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
} __attribute__((packed)) ACPISDTHeader;

typedef struct {
    ACPISDTHeader h;
    uint32_t *PointerToOtherSDT;
} __attribute__((packed)) RSDT;

typedef struct {
    ACPISDTHeader h;
    uint64_t *PointerToOtherSDT;
} __attribute__((packed)) XSDT;

void AcpiSetRsdp(void* rsdp);
void* AcpiFindTable(char TableSignature[4]);
void AcpiListTables();

#endif /* ACPI_H */