#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

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

int sizeof_hdr;

void WriteHeader(FILE* fd, int class, int machine, int version, size_t sizeoftext, size_t sizeofdata, size_t sizeofbss) {
    void* mhdr_addr = NULL;
    size_t hdr_size = 0;

    if (class == AI_Class_M64) {
        Axf64_Mhdr* axf64_mhdr = (Axf64_Mhdr*)malloc(sizeof(Axf64_Mhdr));

        axf64_mhdr->a_ident[AI_MAG0] = AIMAG0;
        axf64_mhdr->a_ident[AI_MAG1] = AIMAG1;
        axf64_mhdr->a_ident[AI_MAG2] = AIMAG2;
        axf64_mhdr->a_ident[AI_MAG3] = AIMAG3;

        axf64_mhdr->a_type = AT_EXEC;
        axf64_mhdr->a_machine_type = machine;

        axf64_mhdr->a_text_size = sizeoftext;
        axf64_mhdr->a_data_size = sizeofdata;
        axf64_mhdr->a_bss_size = sizeofbss;

        axf64_mhdr->a_text_load_addr = 0x401000;
        axf64_mhdr->a_data_load_addr = 0x401000 + sizeoftext;
        axf64_mhdr->a_bss_load_addr  = 0x401000 + sizeoftext + sizeofdata;

        axf64_mhdr->a_version = version;
        axf64_mhdr->a_mclass = class;
        axf64_mhdr->a_sizeof_mhdr = sizeof(Axf64_Mhdr);

        mhdr_addr = axf64_mhdr;
        hdr_size = sizeof(Axf64_Mhdr);
    } 
    else if (class == AI_Class_M32) {
        Axf32_Mhdr* axf32_mhdr = (Axf32_Mhdr*)malloc(sizeof(Axf32_Mhdr));

        axf32_mhdr->a_ident[AI_MAG0] = AIMAG0;
        axf32_mhdr->a_ident[AI_MAG1] = AIMAG1;
        axf32_mhdr->a_ident[AI_MAG2] = AIMAG2;
        axf32_mhdr->a_ident[AI_MAG3] = AIMAG3;

        axf32_mhdr->a_type = AT_EXEC;
        axf32_mhdr->a_machine_type = machine;

        axf32_mhdr->a_text_size = sizeoftext;
        axf32_mhdr->a_data_size = sizeofdata;
        axf32_mhdr->a_bss_size = sizeofbss;

        axf32_mhdr->a_text_load_addr = 0x801000;
        axf32_mhdr->a_data_load_addr = 0x801000 + sizeoftext;
        axf32_mhdr->a_bss_load_addr  = 0x801000 + sizeoftext + sizeofdata;

        axf32_mhdr->a_version = version;
        axf32_mhdr->a_mclass = class;
        axf32_mhdr->a_sizeof_mhdr = sizeof(Axf32_Mhdr);

        mhdr_addr = axf32_mhdr;
        hdr_size = sizeof(Axf32_Mhdr);
    } else {
        fprintf(stderr, "Invalid class type.\n");
        return;
    }

    fseek(fd, 0, SEEK_SET);
    fwrite(mhdr_addr, hdr_size, 1, fd);
    sizeof_hdr = hdr_size;

    free(mhdr_addr);  // Now safe to free after writing
}

void WriteDotText(FILE* fd, char* textdata, size_t text_size) {
    fseek(fd, sizeof_hdr, SEEK_SET);
    fwrite(textdata, text_size, 1, fd);
}

void WriteDotData(FILE* fd, char* datadata, size_t data_size) {
    fseek(fd, sizeof_hdr + sizeof_hdr, SEEK_SET);
    fwrite(datadata, data_size, 1, fd);
}

void WriteDotBss(FILE* fd, char* bssdata, size_t bss_size) {
    fseek(fd, sizeof_hdr + sizeof_hdr + sizeof_hdr, SEEK_SET);
    fwrite(bssdata, bss_size, 1, fd);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-text TEXTBIN] [-bss BSSBIN] [-data DATABIN] [-d/--debug] [-m machine] [-mc class] [-o output.axf]\n", argv[0]);
        return 1;
    }

    char dottext_fd[255] = {0};
    char dotdata_fd[255] = {0};
    char dotbss_fd[255] = {0};

    char outputfile[255] = {0};

    bool debug = false;

    char machine_type[255] = {0};
    char mclass_type[255] = {0};

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-text") == 0) {
            strncpy(dottext_fd, argv[i + 1], sizeof(dottext_fd) - 1);
            i++;
        } else if (strcmp(argv[i], "-bss") == 0) {
            strncpy(dotbss_fd, argv[i + 1], sizeof(dotbss_fd) - 1);
            i++;
        } else if (strcmp(argv[i], "-data") == 0) {
            strncpy(dotdata_fd, argv[i + 1], sizeof(dotdata_fd) - 1);
            i++;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            debug = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            strncpy(outputfile, argv[i + 1], sizeof(outputfile) - 1);
            i++;
        } else if (strcmp(argv[i], "-m") == 0) {
            strncpy(machine_type, argv[i + 1], sizeof(machine_type) - 1);
            i++;
        } else if (strcmp(argv[i], "-mc") == 0) {
            strncpy(mclass_type, argv[i + 1], sizeof(mclass_type));
            i++;
        }
    }

    int mclass;
    
    if (strcmp(mclass_type, "m64") == 0) {
        mclass = AI_Class_M64;
    } else if (strcmp(mclass_type, "m32") == 0) {
        mclass = AI_Class_M32;
    } else {
        mclass = AI_Class_M64;
    }

    int machine = atoi(machine_type);

    // Open binary files
    FILE *ftext = fopen(dottext_fd, "rb");
    FILE *fdata = fopen(dotdata_fd, "rb");
    FILE *fbss = fopen(dotbss_fd, "rb");

    if (!ftext || !fdata || !fbss) {
        fprintf(stderr, "Error opening one or more input files.\n");
        return 1;
    }

    // Get file sizes
    fseek(ftext, 0, SEEK_END);
    fseek(fdata, 0, SEEK_END);
    fseek(fbss, 0, SEEK_END);

    size_t sizetext = ftell(ftext);
    size_t sizedata = ftell(fdata);
    size_t sizebss = ftell(fbss);

    rewind(ftext);
    rewind(fdata);
    rewind(fbss);

    // Allocate memory for section data
    char* textdata = (char*)malloc(sizetext);
    char* datadata = (char*)malloc(sizedata);
    char* bssdata = (char*)malloc(sizebss);

    if (!textdata || !datadata || !bssdata) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(ftext);
        fclose(fdata);
        fclose(fbss);
        return 1;
    }

    // Read section data
    fread(textdata, sizetext, 1, ftext);
    fread(datadata, sizedata, 1, fdata);
    fread(bssdata, sizebss, 1, fbss);

    fclose(ftext);
    fclose(fdata);
    fclose(fbss);

    if (debug) {
        // Debugging: Print part of the binary data
        printf("Text Section (first 16 bytes):\n");
        for (int i = 0; i < 16 && i < sizetext; i++) {
            printf("%02x ", (unsigned char)textdata[i]);
        }
        printf("\nData Section (first 16 bytes):\n");
        for (int i = 0; i < 16 && i < sizedata; i++) {
            printf("%02x ", (unsigned char)datadata[i]);
        }
        printf("\nBSS Section (first 16 bytes):\n");
        for (int i = 0; i < 16 && i < sizebss; i++) {
            printf("%02x ", (unsigned char)bssdata[i]);
        }
        printf("\n");
    }

    FILE *fd = fopen(outputfile, "wb");  // Use "wb" instead of "r+" to create a new file
    if (!fd) {
        fprintf(stderr, "Error: Failed to open output file %s\n", outputfile);
        return 1;
    }

    WriteHeader(fd, mclass, machine, /*Version*/ 1, sizeof(textdata), sizeof(datadata), sizeof(bssdata));
    WriteDotText(fd, textdata, sizeof(textdata));
    WriteDotData(fd, datadata, sizeof(datadata));
    WriteDotBss(fd, bssdata, sizeof(bssdata));

    fclose(fd);

    // Clean up
    free(textdata);
    free(datadata);
    free(bssdata);

    return 0;
}
