#include <Drivers/AHCI.h>

#pragma pack(1, push)

typedef struct {
	uint8_t SuperBlockSignature[5];
	uint32_t LBA_DataBegin;
	uint32_t LBA_DataEnd;
	uint32_t Clus_DataBegin;
	uint32_t Clus_DataEnd;
	uint8_t SectorsPerCluster;
	uint32_t TotalAllocatedMOTClusters;
	uint32_t TotalClusters;
	uint32_t LBA_MOT;
	uint32_t Clus_MOT;
	uint16_t FSVersion;
	uint32_t LBA_ReservedSuperBlockBackup0;
	uint32_t LBA_ReservedSuperBlockBackup1;
	uint32_t LBA_SuperBlockCacheBackup2;
	uint16_t EndOfSuperBlockSignature;
} UfsSuperBlock;

typedef struct{
	uint8_t ObjectType;
	uint64_t UnixCreateTime;
	uint8_t ObjectPermissions;
	uint16_t OwnerID;
	uint16_t GroupID;
	uint8_t ObjectFlags;
	uint64_t ObjectDataSize;
	uint32_t ObjectClusterStart;
	uint32_t ObjectClusterEnd;
	uint8_t ObjectNameLength;
	uint32_t ParentObjectIndex;
	char* ObjectName;
	uint16_t EndOfObjectSignature;
} MotObject;

typedef struct {
	uint32_t TotalObjects;
	MotObject* Objects;
	uint16_t EndOfMOTSignature;
} MasterObjectTable;

typedef struct {
	uint32_t ClusterObjectOwner;
} Cluster;

typedef struct {
	MotObject _OBJECT_HEADER;
	uint32_t AllocatedClusters;
	Cluster Clusters[];
} AllocationTable_Object;

#define SbSig "UFSSB"
#define EndOfSBSig 0xFFEF
#define EndOfMOTSig 0xFF09
#define EndOfObjSig 0xFF0F

#define ObjType_NONE 0b00000000
#define ObjType_FILE 0b00000001
#define ObjType_DIR 0b00000010
#define ObjType_FSGenericObject 0b00000100
#define ObjType_UnknownObject 0b00001000
#define ObjType_OSSpecificObject 0b00010000
#define ObjType_MOT 0b00100000
#define ObjType_FSAllocationTable 0b01000000
#define ObjType_NonDataFileDescriptor 0b10000000 /* use this for sockets, mounts, similar */

#pragma pack(pop)
