#include "fat32.h"
#include <Drivers/AHCI.h>
#include <KiSimple.h>

#define __CONFIG_FAT32_LOG_PROCEDURES_AND_GENERIC_LOGS 1
#define __CONFIG_DISABLE_LFN 1

#define CALLED printk("Function %s was called\n\r", __PRETTY_FUNCTION__);

Fat32Context_t FatCtx;

static uint8_t SectorBuffer[512];
static FatFile_t FatFiles[128];

static uint16_t ReadU16(const void* p) {
	CALLED
	const uint8_t* b = p;
	return b[0] | (b[1] << 8);
}

static uint32_t ReadU32(const void* p) {
	CALLED
	const uint8_t* b = p;
	return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
}

static void CopyLFNChars(const Fat32DirEnt_t* lfnEntry, uint16_t* out, int* outIndex) {
	CALLED
	const uint16_t* p1 = (const uint16_t*)&lfnEntry->Name[0];
	const uint16_t* p2 = (const uint16_t*)&lfnEntry->Name[10];
	const uint16_t* p3 = (const uint16_t*)&lfnEntry->Name[20];

	for (int i = 0; i < 5; i++) {
		uint16_t c = p1[i];
		if (c == 0x0000 || c == 0xFFFF) break;
		out[(*outIndex)++] = c;
	}
	for (int i = 0; i < 6; i++) {
		uint16_t c = p2[i];
		if (c == 0x0000 || c == 0xFFFF) break;
		out[(*outIndex)++] = c;
	}
	for (int i = 0; i < 2; i++) {
		uint16_t c = p3[i];
		if (c == 0x0000 || c == 0xFFFF) break;
		out[(*outIndex)++] = c;
	}
}

static void UTF16ToUTF8(uint16_t* utf16, int len, char* out, int outSize) {
	CALLED
	int j = 0;
	for (int i = 0; i < len && j + 1 < outSize; i++) {
		uint16_t c = utf16[i];
		if (c < 0x80) {
			out[j++] = (char)c;
		} else {
			out[j++] = '?'; // fallback for non-ASCII
		}
	}
	out[j] = 0;
}

// max LFN entries 20 (max 255 chars)
static int ParseLFNEntries(const Fat32DirEnt_t* entries, int count, char* outName, int outSize) {
	CALLED
	uint16_t utf16Name[260];
	int utf16Len = 0;

	for (int i = count - 1; i >= 0; i--) {
		if ((entries[i].Attr & FAT_ATTR_LFN) != FAT_ATTR_LFN) break;
		CopyLFNChars(&entries[i], utf16Name, &utf16Len);
	}

	UTF16ToUTF8(utf16Name, utf16Len, outName, outSize);
	return utf16Len;
}

uint32_t ClusterToLBA(uint32_t cluster) {
	CALLED
	return FatCtx.DataStartLBA + ((cluster - 2) * FatCtx.SectorsPerCluster);
}

uint32_t ReadFATEntry(uint32_t cluster) {
	CALLED
	uint32_t fatOffset = cluster * 4;
	uint32_t fatSector = FatCtx.FATStartLBA + (fatOffset / FatCtx.SectorSize);
	uint32_t offsetInSector = fatOffset % FatCtx.SectorSize;
	AhciReadSector(fatSector, SectorBuffer);
	return ReadU32(&SectorBuffer[offsetInSector]) & 0x0FFFFFFF;
}

void WriteFATEntry(uint32_t cluster, uint32_t value) {
	CALLED
	uint32_t fatOffset = cluster * 4;
	uint32_t fatSector = FatCtx.FATStartLBA + (fatOffset / FatCtx.SectorSize);
	uint32_t offsetInSector = fatOffset % FatCtx.SectorSize;
	AhciReadSector(fatSector, SectorBuffer);
	SectorBuffer[offsetInSector + 0] = value & 0xFF;
	SectorBuffer[offsetInSector + 1] = (value >> 8) & 0xFF;
	SectorBuffer[offsetInSector + 2] = (value >> 16) & 0xFF;
	SectorBuffer[offsetInSector + 3] = (value >> 24) & 0x0F;
	AhciWriteSector(fatSector, SectorBuffer);
}

uint32_t FATAllocCluster(void) {
	CALLED
	for (uint32_t i = 2; i < 0x0FFFFFF6; ++i) {
		if (ReadFATEntry(i) == FAT32_CLUSTER_FREE) {
			WriteFATEntry(i, FAT32_CLUSTER_EOF);
			return i;
		}
	}
	return 0;
}

void FATFreeChain(uint32_t startCluster) {
	CALLED
	while (startCluster >= 2 && startCluster < 0x0FFFFFF8) {
		uint32_t next = ReadFATEntry(startCluster);
		WriteFATEntry(startCluster, FAT32_CLUSTER_FREE);
		startCluster = next;
	}
}

int FatMount(uint32_t lba, uint8_t drive) {
	CALLED
	AhciReadSector(lba, SectorBuffer);
	Fat32BPB_t* bpb = (Fat32BPB_t*)SectorBuffer;
	if (bpb->Signature != 0xAA55) return 0;
	FatCtx.StartLBA = lba;
	FatCtx.SectorSize = bpb->BytesPerSector;
	FatCtx.SectorsPerCluster = bpb->SectorsPerCluster;
	FatCtx.ReservedSectors = bpb->ReservedSectors;
	FatCtx.NumFATs = bpb->NumFATs;
	FatCtx.FATSize = bpb->FATSize32;
	FatCtx.RootCluster = bpb->RootCluster;
	FatCtx.DriveNumber = drive;
	FatCtx.FATStartLBA = FatCtx.StartLBA + FatCtx.ReservedSectors;
	FatCtx.DataStartLBA = FatCtx.FATStartLBA + FatCtx.FATSize * FatCtx.NumFATs;
	for (int i = 0; i < 128; i++) FatFiles[i].FileOpen = 0;
	return 1;
}

uint32_t FatResolvePath(const char* path) {
	CALLED
	uint32_t current = FatCtx.RootCluster;
	if (!path || !*path || *path != '/') return 0;
	path++;
	while (*path) {
		char name[12] = {0};
		int len = 0;
		while (*path && *path != '/' && len < 11) name[len++] = *path++;
		while (len < 11) name[len++] = ' ';
		if (*path == '/') path++;
		uint8_t buf[FatCtx.SectorsPerCluster * FatCtx.SectorSize];
		int found = 0;
		while (current >= 2 && current < 0x0FFFFFF8) {
			uint32_t lba = ClusterToLBA(current);
			for (uint32_t s = 0; s < FatCtx.SectorsPerCluster; s++) {
				AhciReadSector(lba + s, buf + s * FatCtx.SectorSize);
			}
			for (uint32_t i = 0; i < FatCtx.SectorsPerCluster * FatCtx.SectorSize; i += 32) {
				Fat32DirEnt_t* ent = (Fat32DirEnt_t*)&buf[i];
				if (ent->Name[0] == 0x00) break;
				if ((ent->Attr & FAT_ATTR_LFN) == FAT_ATTR_LFN) continue;
				if (!memcmp(ent->Name, name, 11)) {
					current = ((uint32_t)ent->FstClusHI << 16) | ent->FstClusLO;
					found = 1;
					break;
				}
			}
			if (found) break;
			current = ReadFATEntry(current);
		}
		if (!found) return 0;
	}
	return current;
}

int FatCreateFile(const char* path) {
	CALLED
	char tmp[256];
	strcpy(tmp, path);
	char* last = strrchr(tmp, '/');
	if (!last || last == tmp) return 0;
	*last = 0;
	uint32_t dir = FatResolvePath(tmp);
	if (!dir) return 0;
	uint32_t clus = FATAllocCluster();
	if (!clus) return 0;
	uint32_t lba = ClusterToLBA(dir);
	uint8_t* buf = SectorBuffer;
	for (uint32_t s = 0; s < FatCtx.SectorsPerCluster; s++) {
		AhciReadSector(lba + s, buf);
		for (uint32_t i = 0; i < FatCtx.SectorSize; i += 32) {
			if (buf[i] == 0x00 || buf[i] == 0xE5) {
				Fat32DirEnt_t* ent = (Fat32DirEnt_t*)&buf[i];
				memset(ent, 0, sizeof(Fat32DirEnt_t));
				memcpy(ent->Name, last + 1, strlen(last + 1));
				ent->Attr = FAT_ATTR_ARCHIVE;
				ent->FstClusLO = clus & 0xFFFF;
				ent->FstClusHI = clus >> 16;
				AhciWriteSector(lba + s, buf);
				return 1;
			}
		}
	}
	return 0;
}

int FatCreateDirectory(const char* path) {
	CALLED
	char tmp[256];
	strcpy(tmp, path);
	char* last = strrchr(tmp, '/');
	if (!last || last == tmp) return 0;
	*last = 0;
	uint32_t dir = FatResolvePath(tmp);
	if (!dir) return 0;
	uint32_t clus = FATAllocCluster();
	if (!clus) return 0;
	uint32_t lba = ClusterToLBA(dir);
	uint8_t* buf = SectorBuffer;
	for (uint32_t s = 0; s < FatCtx.SectorsPerCluster; s++) {
		AhciReadSector(lba + s, buf);
		for (uint32_t i = 0; i < FatCtx.SectorSize; i += 32) {
			if (buf[i] == 0x00 || buf[i] == 0xE5) {
				Fat32DirEnt_t* ent = (Fat32DirEnt_t*)&buf[i];
				memset(ent, 0, sizeof(Fat32DirEnt_t));
				memcpy(ent->Name, last + 1, strlen(last + 1));
				ent->Attr = FAT_ATTR_DIRECTORY;
				ent->FstClusLO = clus & 0xFFFF;
				ent->FstClusHI = clus >> 16;
				AhciWriteSector(lba + s, buf);
				// Setup '.' and '..' entries in new directory cluster
				uint8_t dirbuf[FatCtx.SectorsPerCluster * FatCtx.SectorSize];
				for (uint32_t i = 0; i < FatCtx.SectorsPerCluster; i++) {
					AhciReadSector(ClusterToLBA(clus) + i, dirbuf + i * FatCtx.SectorSize);
					memset(dirbuf + i * FatCtx.SectorSize, 0, FatCtx.SectorSize);
				}
				Fat32DirEnt_t* dot = (Fat32DirEnt_t*)dirbuf;
				memcpy(dot->Name, ".          ", 11);
				dot->Attr = FAT_ATTR_DIRECTORY;
				dot->FstClusLO = clus & 0xFFFF;
				dot->FstClusHI = clus >> 16;
				Fat32DirEnt_t* dotdot = (Fat32DirEnt_t*)(dirbuf + 32);
				memcpy(dotdot->Name, "..         ", 11);
				dotdot->Attr = FAT_ATTR_DIRECTORY;
				dotdot->FstClusLO = dir & 0xFFFF;
				dotdot->FstClusHI = dir >> 16;
				for (uint32_t i = 0; i < FatCtx.SectorsPerCluster; i++) {
					AhciWriteSector(ClusterToLBA(clus) + i, dirbuf + i * FatCtx.SectorSize);
				}
				return 1;
			}
		}
	}
	return 0;
}

int FatDelete(const char* path) {
	CALLED
	uint32_t current = FatCtx.RootCluster;
	if (!path || !*path || *path != '/') return 0;
	path++;
	char lastName[12];
	lastName[0] = 0;
	while (*path) {
		char name[12] = {0};
		int len = 0;
		while (*path && *path != '/' && len < 11) name[len++] = *path++;
		while (len < 11) name[len++] = ' ';
		strncpy(lastName, name, 11);
		if (*path == '/') path++;
		uint8_t buf[FatCtx.SectorsPerCluster * FatCtx.SectorSize];
		int found = 0;
		while (current >= 2 && current < 0x0FFFFFF8) {
			uint32_t lba = ClusterToLBA(current);
			for (uint32_t s = 0; s < FatCtx.SectorsPerCluster; s++) {
				AhciReadSector(lba + s, buf + s * FatCtx.SectorSize);
			}
			for (uint32_t i = 0; i < FatCtx.SectorsPerCluster * FatCtx.SectorSize; i += 32) {
				Fat32DirEnt_t* ent = (Fat32DirEnt_t*)&buf[i];
				if (ent->Name[0] == 0x00) break;
				if ((ent->Attr & FAT_ATTR_LFN) == FAT_ATTR_LFN) continue;
				if (!memcmp(ent->Name, lastName, 11)) {
					uint32_t clus = ((uint32_t)ent->FstClusHI << 16) | ent->FstClusLO;
					FATFreeChain(clus);
					ent->Name[0] = 0xE5;
					AhciWriteSector(ClusterToLBA(current) + (i / FatCtx.SectorSize), buf);
					return 1;
				}
			}
			if (found) break;
			current = ReadFATEntry(current);
		}
	}
	return 0;
}

int FatListDirectory(const char* path, FatFile_t* outArray, int maxCount) {
	CALLED
	uint32_t cluster = FatResolvePath(path);
	if (!cluster) return 0;
	int count = 0;
	uint8_t buf[FatCtx.SectorsPerCluster * FatCtx.SectorSize];
	while (cluster >= 2 && cluster < 0x0FFFFFF8 && count < maxCount) {
		uint32_t lba = ClusterToLBA(cluster);
		for (uint32_t s = 0; s < FatCtx.SectorsPerCluster; s++) {
			AhciReadSector(lba + s, buf + s * FatCtx.SectorSize);
		}
		for (uint32_t i = 0; i < FatCtx.SectorsPerCluster * FatCtx.SectorSize && count < maxCount; i += 32) {
			Fat32DirEnt_t* ent = (Fat32DirEnt_t*)&buf[i];
			if (ent->Name[0] == 0x00) return count;
			if ((ent->Attr & FAT_ATTR_LFN) == FAT_ATTR_LFN) continue;
			FatFile_t* f = &outArray[count];
			memcpy(f->Name, ent->Name, 11);
			f->Name[11] = 0;
			f->FirstCluster = ((uint32_t)ent->FstClusHI << 16) | ent->FstClusLO;
			f->Size = ent->FileSize;
			f->Attr = ent->Attr;
			f->IsDirectory = (ent->Attr & FAT_ATTR_DIRECTORY) ? 1 : 0;
			count++;
		}
		cluster = ReadFATEntry(cluster);
	}
	return count;
}

FatFile_t* FatOpen(const char* path) {
	CALLED
	uint32_t cluster = FatResolvePath(path);
	if (!cluster) return NULL;
	uint8_t buf[FatCtx.SectorsPerCluster * FatCtx.SectorSize];
	uint32_t current = FatCtx.RootCluster;
	const char* p = path;
	if (!p || *p != '/') return NULL;
	p++;
	char name[12] = {0};
	int len = 0;
	// Extract filename component from path (last component)
	const char* lastSlash = strrchr(path, '/');
	if (!lastSlash) return NULL;
	const char* fileName = lastSlash + 1;
	memset(name, ' ', 11);
	for (int i = 0; i < 11 && fileName[i]; i++) {
		name[i] = fileName[i];
	}
	while (current >= 2 && current < 0x0FFFFFF8) {
		uint32_t lba = ClusterToLBA(current);
		for (uint32_t s = 0; s < FatCtx.SectorsPerCluster; s++) {
			AhciReadSector(lba + s, buf + s * FatCtx.SectorSize);
		}
		for (uint32_t i = 0; i < FatCtx.SectorsPerCluster * FatCtx.SectorSize; i += 32) {
			Fat32DirEnt_t* ent = (Fat32DirEnt_t*)&buf[i];
			if (ent->Name[0] == 0x00) break;
			if ((ent->Attr & FAT_ATTR_LFN) == FAT_ATTR_LFN) continue;
			if (!memcmp(ent->Name, name, 11)) {
				if (ent->Attr & FAT_ATTR_DIRECTORY) return NULL; // Do not open directories
				for (int fd = 0; fd < 128; fd++) {
					if (!FatFiles[fd].FileOpen) {
						FatFiles[fd].FileOpen = 1;
						FatFiles[fd].FirstCluster = ((uint32_t)ent->FstClusHI << 16) | ent->FstClusLO;
						FatFiles[fd].Size = ent->FileSize;
						FatFiles[fd].Attr = ent->Attr;
						FatFiles[fd].IsDirectory = 0;
						memcpy(FatFiles[fd].Name, ent->Name, 11);
						FatFiles[fd].FilePosition = 0;
						return &FatFiles[i];
					}
				}
				return NULL;
			}
		}
		current = ReadFATEntry(current);
	}
	return NULL;
}

void FatClose(FatFile_t* file) {
	CALLED
    if (!file || !file->FileOpen) return;
    file->FileOpen = 0;
}

int FatSeek(FatFile_t* file, uint32_t position) {
	CALLED
    if (!file || !file->FileOpen) return 0;
    file->FilePosition = position;
    return 1;
}

int FatRead(FatFile_t* file, void* buffer, uint32_t size) {
	CALLED
	if (!file || !file->FileOpen) return 0;
	uint32_t cluster = file->FirstCluster;
	uint8_t* out = (uint8_t*)buffer;
	uint32_t remaining = size;
	while (remaining && cluster >= 2 && cluster < 0x0FFFFFF8) {
		uint32_t lba = ClusterToLBA(cluster);
		for (uint32_t s = 0; s < FatCtx.SectorsPerCluster && remaining; s++) {
			AhciReadSector(lba + s, SectorBuffer);
			uint32_t chunk = remaining > 512 ? 512 : remaining;
			memcpy(out, SectorBuffer, chunk);
			out += chunk;
			remaining -= chunk;
		}
		cluster = ReadFATEntry(cluster);
	}
	return size - remaining;
}

int FatWrite(FatFile_t* file, const void* buffer, uint32_t size) {
	CALLED
	if (!file || !file->FileOpen) return 0;
	uint32_t remaining = size;
	const uint8_t* in = (const uint8_t*)buffer;
	uint32_t cluster = file->FirstCluster;
	while (remaining && cluster >= 2 && cluster < 0x0FFFFFF8) {
		uint32_t lba = ClusterToLBA(cluster);
		for (uint32_t s = 0; s < FatCtx.SectorsPerCluster && remaining; s++) {
			uint32_t chunk = remaining > 512 ? 512 : remaining;
			memcpy(SectorBuffer, in, chunk);
			AhciWriteSector(lba + s, SectorBuffer);
			in += chunk;
			remaining -= chunk;
		}
		if (remaining) {
			uint32_t next = ReadFATEntry(cluster);
			if (next < 2 || next >= 0x0FFFFFF8) {
				next = FATAllocCluster();
				if (!next) break;
				WriteFATEntry(cluster, next);
				WriteFATEntry(next, FAT32_CLUSTER_EOF);
			}
			cluster = next;
		}
	}
	uint32_t written = size - remaining;
	if (file->Size < file->FilePosition + written)
		file->Size = file->FilePosition + written;
	file->FilePosition += written;
	return written;
}
