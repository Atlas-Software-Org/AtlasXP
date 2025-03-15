#ifndef _axf_h
#define _axf_h 1

/* Standard AXF types.  */

#include <stdint.h>

/* Type for a 16-bit quantity.  */
typedef uint16_t Axf32_Half;
typedef uint16_t Axf64_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Axf32_Word;
typedef	int32_t  Axf32_Sword;
typedef uint32_t Axf64_Word;
typedef	int32_t  Axf64_Sword;

/* Types for signed and unsigned 64-bit quantities.  */
typedef uint64_t Axf32_Xword;
typedef	int64_t  Axf32_Sxword;
typedef uint64_t Axf64_Xword;
typedef	int64_t  Axf64_Sxword;

/* Type of addresses.  */
typedef uint32_t Axf32_Addr;
typedef uint64_t Axf64_Addr;

/* Type of file offsets.  */
typedef uint32_t Axf32_Off;
typedef uint64_t Axf64_Off;

/* Type for section indices, which are 16-bit quantities.  */
typedef uint16_t Axf32_Section;
typedef uint16_t Axf64_Section;

/* Type for version symbol information.  */
typedef Axf32_Half Axf32_Versym;
typedef Axf64_Half Axf64_Versym;

// ; a_ident

#define AI_MAG0 0
#define AIMAG0 0xAB
#define AI_MAG1 1
#define AIMAG1 'A'
#define AI_MAG2 2
#define AIMAG2 'X'
#define AI_MAG3 3
#define AIMAG3 'F'

// ; a_type
#define AI_TYPE_AXF         4

#define AT_EXEC 0xFF
#define AT_LIB  0x7F
#define AT_NONE 0x00

// ; a_machine_type
// Architecture Types

#define AI_TYPE               6

// x86 and x86_64 Architectures
#define AI_Machine_x86_64     0xFF  // 64-bit x86 (x86-64)
#define AI_Machine_i386       0xFE  // 32-bit x86 (i386)
#define AI_Machine_x86        AI_Machine_i386  // Alias for x86 32-bit

// ARM Architectures
#define AI_Machine_Arm64      0xFB  // ARM 64-bit architecture (AArch64)
#define AI_Machine_Arm32      0xFA  // ARM 32-bit architecture (AArch32)
#define AI_Machine_ArmV7      0xF9  // ARMv7 architecture (32-bit)

// RISC-V Architectures
#define AI_Machine_RiscV      0xFC  // RISC-V architecture
#define AI_Machine_RiscV64    0xF8  // RISC-V 64-bit architecture
#define AI_Machine_RiscV32    0xF7  // RISC-V 32-bit architecture

// SPARC Architectures
#define AI_Machine_SPARC      0xF6  // SPARC architecture (32-bit)
#define AI_Machine_SPARC64    0xF5  // SPARC architecture (64-bit)

// MIPS Architectures
#define AI_Machine_MIPS       0xF4  // MIPS architecture (32-bit)
#define AI_Machine_MIPS64     0xF3  // MIPS architecture (64-bit)

// PowerPC Architectures
#define AI_Machine_PowerPC    0xF2  // PowerPC architecture (32-bit)
#define AI_Machine_PowerPC64  0xF1  // PowerPC architecture (64-bit)

// Other Architectures
#define AI_Machine_None       0xF0  // No architecture specified
#define AI_Machine_16B        0xFD  // 16-bit machine
#define AI_Machine_Unknown    0x00  // Unknown machine type

// ; a_mclass

#define AIClass 60

#define AI_Class_M64 0xFF
#define AI_Class_M32 0xEE

// 64-bit version
typedef struct Axf64_Mhdr {
    unsigned char a_ident[4];   // 4
    Axf64_Half a_machine_type;  // 2
    Axf64_Half a_type;          // 2
    Axf64_Xword a_text_size;    // 8
    Axf64_Xword a_data_size;    // 8
    Axf64_Xword a_bss_size;     // 8
    Axf64_Addr a_text_load_addr;// 8
    Axf64_Addr a_data_load_addr;// 8
    Axf64_Addr a_bss_load_addr; // 8
    Axf64_Word a_version;       // 4
    Axf64_Word a_mclass;        // 4
    Axf64_Word a_sizeof_mhdr;   // 4
    uint8_t __pad[0];                 // Using zero-length padding
} __attribute__((packed)) Axf64_Mhdr;

// 32-bit version
typedef struct Axf32_Mhdr {
    unsigned char a_ident[4];   // 4
    Axf32_Half a_type;          // 2
    Axf32_Xword a_text_size;    // 4 (on 32-bit systems)
    Axf32_Xword a_data_size;    // 4
    Axf32_Xword a_bss_size;     // 4
    Axf32_Half a_machine_type;  // 2
    Axf32_Addr a_text_load_addr;// 4 (on 32-bit systems)
    Axf32_Addr a_data_load_addr;// 4
    Axf32_Addr a_bss_load_addr; // 4
    Axf32_Word a_version;       // 4
    Axf32_Word a_mclass;        // 4
    Axf32_Word a_sizeof_mhdr;   // 4
} __attribute__((packed)) Axf32_Mhdr;

Axf64_Mhdr *ParseAXF64Executable(void* data_start);
Axf32_Mhdr *ParseAXF32Executable(void* data_start);
int GetAXF_MClass(void* data_start);
void CallFunctionFromAddr_64(Axf64_Addr address);
void CallFunctionFromAddr_32(Axf32_Addr address);

#endif