#ifndef UNREALFS_H
#define UNREALFS_H 1

#include <Drivers/AHCI.h>

#pragma pack(push, 1)

typedef struct {
	uint8_t SuperBlockSignature[5];            // Must be "UFSSB"
	uint32_t LBA_DataBegin;
	uint32_t LBA_DataEnd;
	uint32_t Clus_DataBegin;
	uint32_t Clus_DataEnd;
	uint8_t SectorsPerCluster;                 // 2 sectors = 1 cluster (1 KiB)
	uint32_t TotalAllocatedMOTClusters;
	uint32_t TotalClusters;
	uint32_t LBA_MOT;
	uint32_t Clus_MOT;
	uint32_t Clus_RootDirectory;
	uint16_t FSVersion;
	uint32_t LBA_ReservedSuperBlockBackup0;
	uint32_t LBA_ReservedSuperBlockBackup1;
	uint32_t LBA_SuperBlockCacheBackup2;       // Optional runtime-modifiable SB
	uint32_t Clus_FreeClusterBitmap;           // Optional: for free cluster tracking
	uint32_t Clus_JournalLog;                  // Optional: journaling support
	uint16_t EndOfSuperBlockSignature;         // Must be 0xFFEF
} UfsSuperBlock;

typedef struct {
	uint8_t ObjectType;
	uint64_t UnixCreateTime;
	uint64_t UnixModifiedTime;
	uint64_t UnixAccessedTime;
	uint64_t UnixChangeTime;
	uint8_t ObjectPermissions;                 // e.g., 0b111 for RWX
	uint16_t OwnerID;
	uint16_t GroupID;
	uint8_t ObjectFlags;                       // e.g., hidden, symlink, system
	uint64_t LogicalSize;                      // Actual file size in bytes
	uint64_t AllocatedSize;                    // Total space in clusters
	uint32_t ObjectClusterStart;
	uint32_t ObjectClusterEnd;
	uint8_t ObjectNameLength;
	uint32_t ParentObjectIndex;
	char* ObjectName;
	uint32_t CRC32;                            // Optional integrity check
	uint16_t EndOfObjectSignature;             // Must be 0xFF0F
} MotObject;

typedef struct {
	uint32_t TotalObjects;
	MotObject Objects[];
	uint16_t EndOfMOTSignature;                // Must be 0xFF09
} MasterObjectTable;

typedef struct {
	uint32_t ClusterObjectOwner;
} Cluster;

typedef struct {
	MotObject _OBJECT_HEADER;
	uint32_t AllocatedClusters;
	Cluster Clusters[];
} AllocationTable_Object;

// --- Signatures ---
#define SbSig "UFSSB"
#define EndOfSBSig  0xFFEF
#define EndOfMOTSig 0xFF09
#define EndOfObjSig 0xFF0F

// --- Object Types ---
#define ObjType_NONE                 0b00000000
#define ObjType_FILE                 0b00000001
#define ObjType_DIR                  0b00000010
#define ObjType_FSGenericObject      0b00000100
#define ObjType_UnknownObject        0b00001000
#define ObjType_OSSpecificObject     0b00010000
#define ObjType_MOT                  0b00100000
#define ObjType_FSAllocationTable   0b01000000
#define ObjType_NonDataFileDescriptor 0b10000000
#define ObjType_SYMLINK             0xA0       // Optional

// --- Permissions ---
#define OBJ_PERM_R 0x1
#define OBJ_PERM_W 0x2
#define OBJ_PERM_X 0x4

// --- Flags ---
#define OBJ_FLAG_HIDDEN     0x01
#define OBJ_FLAG_SYSTEM     0x02
#define OBJ_FLAG_SYMLINK    0x04
#define OBJ_FLAG_MOUNTPOINT 0x08

#pragma pack(pop)

#endif /* UNREALFS_H */
