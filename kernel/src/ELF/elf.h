#ifndef ELF_H
#define ELF_H 1

#include <elf.h>
#include <KiSimple.h>
#include <VMM/vmm.h>
#include <stdint.h>
#include <stddef.h>

extern int CurrentPid;

void LoadKernelElf(void* elf_data, int argc, char** argv, char** envp);
void UnloadKernelElf();

#endif /* ELF_H */