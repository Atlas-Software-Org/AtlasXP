/*
 * ===============================
 *        cpio_newc.h usage
 *         For Dummies™
 * ===============================
 *
 * This header provides simple functions to work with
 * CPIO archives in the "newc" format, commonly used
 * for Linux initramfs and embedded boot filesystems.
 *
 * -------- Basic Types --------
 *
 * CpioArchive:
 *   Holds metadata and memory pointer for a loaded archive.
 *
 * CpioFile:
 *   Used to represent a found file inside the archive,
 *   includes name, data pointer, and size.
 *
 * -------- Initialization --------
 *
 * Load a memory-mapped cpio archive (e.g., from a module):
 *
 *   CpioArchive archive;
 *   if (!CpioLoadArchive(&archive, cpio_data, cpio_size)) {
 *       // Invalid archive, wrong format or corrupted
 *   }
 *
 * -------- List Files --------
 *
 *   CpioLsArchive(&archive);
 *   // Prints out all file names in the archive
 *
 * -------- Find a File --------
 *
 *   CpioFile file;
 *   if (CpioFindFileArchive(&archive, "etc/init", &file)) {
 *       // file.data points to the content
 *       // file.size gives its length
 *   }
 *
 * -------- Read File to Buffer (ASCII/text) --------
 *
 *   char buffer[256];
 *   if (CpioReadFileArchiveA(&archive, "hello.txt", buffer, sizeof(buffer))) {
 *       printf("Contents: %s\n", buffer);
 *   }
 *
 * -------- Read File to Buffer (binary) --------
 *
 *   uint8_t raw_data[512];
 *   if (CpioReadFileArchiveB(&archive, "bin/data", raw_data, sizeof(raw_data))) {
 *       // raw_data now contains file contents
 *   }
 *
 * -------- Cleanup --------
 *
 *   CpioUnloadArchive(&archive);
 *   // Optional in this case (no heap allocation),
 *   // but good practice to zero the pointer.
 *
 * -------- Notes --------
 *
 * - Only supports "newc" format (ASCII header, fixed field widths)
 * - Archive must be loaded into memory (e.g., from file or bootloader)
 * - No filesystem semantics (no directories, no symlinks, etc.)
 * - Not endian-sensitive; works on little-endian systems
 *
 */


#ifndef CPIO_NEWC_H
#define CPIO_NEWC_H 1

#include <KiSimple.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
	const void *base;
	size_t size;
} CpioArchive;

typedef struct {
	const char *name;
	const void *data;
	size_t size;
} CpioFile;

int CpioLoadArchive(CpioArchive *archive, const void *base, size_t size);
void CpioUnloadArchive(CpioArchive *archive);
void CpioLsArchive(const CpioArchive *archive);
int CpioFindFileArchive(const CpioArchive *archive, const char *path, CpioFile *out);
int CpioReadFileArchiveA(const CpioArchive *archive, const char *path, char *buf, size_t buf_size);
int CpioReadFileArchiveB(const CpioArchive *archive, const char *path, uint8_t *buf, size_t buf_size);

#endif /* CPIO_NEWC_H */
