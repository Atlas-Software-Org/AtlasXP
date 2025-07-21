#ifndef RAMFS_H
#define RAMFS_H 1

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define RAMFS_MAX_FILES 64
#define RAMFS_PATH_MAX 128

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct {
	char Path[RAMFS_PATH_MAX];
	const uint8_t* Data;
	size_t Size;
	bool IsDirectory;
} RamFileEntry_t;

typedef struct {
	const RamFileEntry_t* Entry;
	size_t Offset;
	bool Open;
} RamFileHandle_t;

bool RamFsInit(void);
bool RamFsRegister(const char* path, const uint8_t* data, size_t size);
bool RamFsRegisterDir(const char* path);

RamFileHandle_t* RamFsOpen(const char* path);
void RamFsClose(RamFileHandle_t* handle);

size_t RamFsRead(RamFileHandle_t* handle, void* buffer, size_t count);

bool RamFsSeek(RamFileHandle_t* handle, size_t offset);
bool RamFsLseek(RamFileHandle_t* handle, long offset, int whence);

size_t RamFsTell(RamFileHandle_t* handle);

size_t RamFsGetSize(RamFileHandle_t* handle);

const char* RamFsGetPath(RamFileHandle_t* handle);
const RamFileEntry_t* RamFsList(size_t index);
const RamFileEntry_t* RamFsResolvePath(const char* path);

RamFileHandle_t* RamFsGetHandleFromFd(int fd);
int RamFsGetFdFromHandle(RamFileHandle_t* handle);

#endif /* RAMFS_H */
