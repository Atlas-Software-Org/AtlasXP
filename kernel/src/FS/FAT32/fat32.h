#ifndef FAT32_H
#define FAT32_H 1

#include <stdint.h>

#define FAT_ATTR_READ_ONLY   0x01
#define FAT_ATTR_HIDDEN      0x02
#define FAT_ATTR_SYSTEM      0x04
#define FAT_ATTR_VOLUME_ID   0x08
#define FAT_ATTR_DIRECTORY   0x10
#define FAT_ATTR_ARCHIVE     0x20
#define FAT_ATTR_LFN         (FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOLUME_ID)

#define FAT32_CLUSTER_FREE       0x00000000
#define FAT32_CLUSTER_RESERVED   0x0FFFFFF0
#define FAT32_CLUSTER_BAD        0x0FFFFFF7
#define FAT32_CLUSTER_EOF        0x0FFFFFF8

#pragma pack(push, 1)

typedef struct {
	uint8_t  Jump[3];
	char     OEMName[8];
	uint16_t BytesPerSector;
	uint8_t  SectorsPerCluster;
	uint16_t ReservedSectors;
	uint8_t  NumFATs;
	uint16_t RootEntryCount;
	uint16_t TotalSectors16;
	uint8_t  Media;
	uint16_t FATSize16;
	uint16_t SectorsPerTrack;
	uint16_t NumberOfHeads;
	uint32_t HiddenSectors;
	uint32_t TotalSectors32;

	uint32_t FATSize32;
	uint16_t ExtFlags;
	uint16_t FSVersion;
	uint32_t RootCluster;
	uint16_t FSInfo;
	uint16_t BackupBootSector;
	uint8_t  Reserved[12];

	uint8_t  DriveNumber;
	uint8_t  Reserved1;
	uint8_t  BootSignature;
	uint32_t VolumeID;
	char     VolumeLabel[11];
	char     FileSystemType[8];
	uint8_t  BootCode[420];
	uint16_t Signature;
} Fat32BPB_t;

#pragma pack(pop)

#pragma pack(push, 1)

typedef struct {
	char     Name[11];
	uint8_t  Attr;
	uint8_t  NTRes;
	uint8_t  CrtTimeTenth;
	uint16_t CrtTime;
	uint16_t CrtDate;
	uint16_t LstAccDate;
	uint16_t FstClusHI;
	uint16_t WrtTime;
	uint16_t WrtDate;
	uint16_t FstClusLO;
	uint32_t FileSize;
} Fat32DirEnt_t;

typedef struct {
	uint8_t  Ord;
	uint16_t Name1[5];
	uint8_t  Attr;
	uint8_t  Type;
	uint8_t  Chksum;
	uint16_t Name2[6];
	uint16_t FstClusLO;
	uint16_t Name3[2];
} Fat32LFNEnt_t;

#pragma pack(pop)

typedef struct {
	uint32_t StartLBA;
	uint32_t SectorSize;
	uint32_t SectorsPerCluster;
	uint32_t ReservedSectors;
	uint32_t NumFATs;
	uint32_t FATSize;
	uint32_t RootCluster;
	uint32_t FATStartLBA;
	uint32_t DataStartLBA;
	uint8_t  DriveNumber;
} Fat32Context_t;

extern Fat32Context_t FatCtx;

typedef struct {
    int FileOpen;
    uint32_t FilePosition;
    uint32_t FirstCluster;
    uint32_t Size;
    uint8_t Attr;
    uint8_t IsDirectory;
    char Name[12];
} FatFile_t;

int FatMount(uint32_t lba, uint8_t drive);

FatFile_t* FatOpen(const char* path);
int FatRead(FatFile_t* file, void* buffer, uint32_t size);
int FatWrite(FatFile_t* file, const void* buffer, uint32_t size);
int FatSeek(FatFile_t* file, uint32_t position);
void FatClose(FatFile_t* file);

int FatListDirectory(const char* path, FatFile_t* outArray, int maxCount);
int FatCreateFile(const char* path);
int FatCreateDirectory(const char* path);
int FatDelete(const char* path);

uint32_t FATAllocCluster(void);
void FATFreeChain(uint32_t startCluster);

uint32_t ReadFATEntry(uint32_t cluster);
void WriteFATEntry(uint32_t cluster, uint32_t value);
uint32_t ClusterToLBA(uint32_t cluster);

#endif /* FAT32_H */
