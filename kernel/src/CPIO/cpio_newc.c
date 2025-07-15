#include "cpio_newc.h"

#define ALIGN4(x) (((x) + 3) & ~3)

static uint32_t hex_to_u32(const char *str) {
	uint32_t r = 0;
	for (int i = 0; i < 8; i++) {
		char c = str[i];
		r <<= 4;
		if (c >= '0' && c <= '9') r |= c - '0';
		else if (c >= 'A' && c <= 'F') r |= c - 'A' + 10;
		else if (c >= 'a' && c <= 'f') r |= c - 'a' + 10;
	}
	return r;
}

int CpioLoadArchive(CpioArchive *archive, const void *base, size_t size) {
	if (!archive || !base || size < 110) return 0;
	archive->base = base;
	archive->size = size;
	return 1;
}

void CpioUnloadArchive(CpioArchive *archive) {
	archive->base = NULL;
	archive->size = 0;
}

void CpioLsArchive(const CpioArchive *archive) {
	const uint8_t *p = archive->base;
	const uint8_t *end = p + archive->size;

	while (p + 110 <= end) {
		if (memcmp(p, "070701", 6) != 0 && memcmp(p, "070100", 6) != 0)
			break;

		uint32_t namesize = hex_to_u32((const char *)(p + 94));
		uint32_t filesize = hex_to_u32((const char *)(p + 54));
		const char *name = (const char *)(p + 110);

		if (p + 110 + ALIGN4(namesize) > end)
			break;

		if (strcmp(name, "TRAILER!!!") == 0)
			break;

		const char *clean = name;
		if (clean[0] == '.' && clean[1] == '/')
			clean += 2;

		if (strcmp(clean, ".") != 0)
			printk("%s\n\r", clean);

		size_t entry_size = 110 + ALIGN4(namesize) + ALIGN4(filesize);
		if (p + entry_size > end)
			break;

		p += entry_size;
	}
}

int CpioFindFileArchive(const CpioArchive *archive, const char *path, CpioFile *out) {
	const uint8_t *p = archive->base;
	const uint8_t *end = p + archive->size;

	while (p + 110 <= end) {
		if (memcmp(p, "070701", 6) != 0 && memcmp(p, "070100", 6) != 0)
			break;

		uint32_t namesize = hex_to_u32((const char *)(p + 94));
		uint32_t filesize = hex_to_u32((const char *)(p + 54));
		const char *name = (const char *)(p + 110);

		if (p + 110 + ALIGN4(namesize) > end)
			break;

		if (strcmp(name, "TRAILER!!!") == 0)
			break;

		const char *clean = name;
		if (clean[0] == '.' && clean[1] == '/')
			clean += 2;

		if (!strcmp(clean, path)) {
			if (out) {
				out->name = clean;
				out->data = p + 110 + ALIGN4(namesize);
				out->size = filesize;
			}
			return 1;
		}

		size_t entry_size = 110 + ALIGN4(namesize) + ALIGN4(filesize);
		if (p + entry_size > end)
			break;

		p += entry_size;
	}
	return 0;
}

int CpioReadFileArchiveA(const CpioArchive *archive, const char *path, char *buf, size_t buf_size) {
	CpioFile file;
	if (!CpioFindFileArchive(archive, path, &file)) return 0;
	if (file.size >= buf_size) return 0;
	memcpy(buf, file.data, file.size);
	buf[file.size] = '\0';
	return 1;
}

int CpioReadFileArchiveB(const CpioArchive *archive, const char *path, uint8_t *buf, size_t buf_size) {
	CpioFile file;
	if (!CpioFindFileArchive(archive, path, &file)) return 0;
	if (file.size > buf_size) return 0;
	memcpy(buf, file.data, file.size);
	return 1;
}
