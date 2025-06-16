#ifndef ELF_H
#define ELF_H 1

#include <elf.h>
#include <KiSimple.h>
#include <VMM/vmm.h>
#include <stdint.h>

void LoadKernelElf(void* elf_data);

#endif /* ELF_H */