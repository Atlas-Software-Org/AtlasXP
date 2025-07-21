#include "ramfs.h"
#include <KiSimple.h>

static RamFileEntry_t RamFsFiles[RAMFS_MAX_FILES];
static size_t RamFsFileCount = 0;
static RamFileHandle_t RamFsHandles[RAMFS_MAX_FILES];

bool RamFsInit(void) {
	RamFsFileCount = 0;
	for (size_t i = 0; i < RAMFS_MAX_FILES; i++) {
		RamFsHandles[i].Open = false;
	}
	return true;
}

static bool RamFsInternalRegister(const char* path, const uint8_t* data, size_t size, bool isDir) {
	if (RamFsFileCount >= RAMFS_MAX_FILES) {
		return false;
	}
	if (strlen(path) >= RAMFS_PATH_MAX) {
		return false;
	}
	for (size_t i = 0; i < RamFsFileCount; i++) {
		if (strcmp(RamFsFiles[i].Path, path) == 0) {
			return false;
		}
	}
	strcpy(RamFsFiles[RamFsFileCount].Path, path);
	RamFsFiles[RamFsFileCount].Data = data;
	RamFsFiles[RamFsFileCount].Size = size;
	RamFsFiles[RamFsFileCount].IsDirectory = isDir;
	RamFsFileCount++;
	return true;
}

bool RamFsRegister(const char* path, const uint8_t* data, size_t size) {
	return RamFsInternalRegister(path, data, size, false);
}

bool RamFsRegisterDir(const char* path) {
	size_t len = strlen(path);
	if (len == 0 || path[len - 1] != '/') return false;
	return RamFsInternalRegister(path, NULL, 0, true);
}

RamFileHandle_t* RamFsOpen(const char* path) {
	const RamFileEntry_t* e = RamFsResolvePath(path);
	if (!e || e->IsDirectory) return NULL;
	for (size_t j = 0; j < RAMFS_MAX_FILES; j++) {
		if (!RamFsHandles[j].Open) {
			RamFsHandles[j].Entry = e;
			RamFsHandles[j].Offset = 0;
			RamFsHandles[j].Open = true;
			return &RamFsHandles[j];
		}
	}
	return NULL;
}

size_t RamFsRead(RamFileHandle_t* handle, void* buffer, size_t count) {
	if (!handle || !handle->Open || !handle->Entry || handle->Entry->IsDirectory) return 0;
	if (handle->Offset >= handle->Entry->Size) return 0;
	size_t remaining = handle->Entry->Size - handle->Offset;
	size_t toRead = (count < remaining) ? count : remaining;
	memcpy(buffer, handle->Entry->Data + handle->Offset, toRead);
	handle->Offset += toRead;
	return toRead;
}

bool RamFsSeek(RamFileHandle_t* handle, size_t offset) {
	if (!handle || !handle->Open || !handle->Entry || offset > handle->Entry->Size) return false;
	handle->Offset = offset;
	return true;
}

bool RamFsLseek(RamFileHandle_t* handle, long offset, int whence) {
	if (!handle || !handle->Open || !handle->Entry) return false;

	size_t newOffset = 0;

	switch (whence) {
		case SEEK_SET:
			if (offset < 0) return false;
			newOffset = (size_t)offset;
			break;

		case SEEK_CUR:
			if ((long)handle->Offset + offset < 0) return false;
			newOffset = handle->Offset + offset;
			break;

		case SEEK_END:
			if ((long)handle->Entry->Size + offset < 0) return false;
			newOffset = handle->Entry->Size + offset;
			break;

		default:
			return false;
	}

	if (newOffset > handle->Entry->Size) return false;

	handle->Offset = newOffset;
	return true;
}

size_t RamFsTell(RamFileHandle_t* handle) {
	if (!handle || !handle->Open || !handle->Entry) return 0;
	return handle->Offset;
}


void RamFsClose(RamFileHandle_t* handle) {
	if (!handle) return;
	handle->Open = false;
	handle->Entry = NULL;
	handle->Offset = 0;
}

size_t RamFsGetSize(RamFileHandle_t* handle) {
	if (!handle || !handle->Entry) return 0;
	return handle->Entry->Size;
}

const char* RamFsGetPath(RamFileHandle_t* handle) {
	if (!handle || !handle->Entry) return NULL;
	return handle->Entry->Path;
}

const RamFileEntry_t* RamFsList(size_t index) {
	if (index >= RamFsFileCount) return NULL;
	return &RamFsFiles[index];
}

const RamFileEntry_t* RamFsResolvePath(const char* path) {
	for (size_t i = 0; i < RamFsFileCount; i++) {
		if (strcmp(RamFsFiles[i].Path, path) == 0) {
			return &RamFsFiles[i];
		}
	}
	return NULL;
}

RamFileHandle_t* RamFsGetHandleFromFd(int fd) {
	return &RamFsHandles[fd];
}

int RamFsGetFdFromHandle(RamFileHandle_t* handle) {
	for (int i = 0; i < 64; i++) {
		if (handle == &RamFsHandles[i]) {
			return i;
		}
	}
	return -1;
}
