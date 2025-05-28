/*
    Author: Adam Bassem
    Revision 0
    Patch 0
    Minor 0
    Major 0
    Atlas 0.0.7
*/

#ifndef GDT_H
#define GDT_H 1

#include <stdint.h>
#include <KiSimple.h>

typedef struct GDTDescriptor {
    uint16_t Size;
    uint64_t Offset;
} __attribute__((packed)) GDTDescriptor;

typedef struct GDTEntry {
    uint16_t Limit0;
    uint16_t Base0;
    uint8_t Base1;
    uint8_t AccessByte;
    uint8_t Limit1_Flags;
    uint8_t Base2;
}__attribute__((packed)) GDTEntry;

typedef struct GDT {
    GDTEntry Null; //0x00
    GDTEntry KernelCode; //0x08
    GDTEntry KernelData; //0x10
    GDTEntry UserNull;
    GDTEntry UserCode;
    GDTEntry UserData;
} __attribute__((packed)) 
__attribute((aligned(0x1000))) GDT;

extern GDT DefaultGDT;

extern void KiGdtLoad(GDTDescriptor* gdtDescriptor);

void KiGdtInit();

#endif /* GDT_H */