#ifndef SYSCALLS_H
#define SYSCALLS_H 1

#include <KiSimple.h>
#include <Drivers/PS2Keyboard.h>
#include <PMM/pmm.h>
#include <VMM/vmm.h>
#include <ELF/elf.h>
#include <stdint.h>

#define AsnuOpen         0
#define AsnuClose        1
#define AsnuWrite        2
#define AsnuRead         3
#define AsnuLseek        4
#define AsnuSeek         5
#define AsnuGetPid       6
#define AsnuExec         7
#define AsnuListDir      8
#define AsnuChdir        9
#define AsnuGetCwd       10
#define AsnuCreateDir    11
#define AsnuRemoveDir    12
#define AsnuRemoveFile   13
#define AsnuCreateFile   14
#define AsnuExit         15
#define AsnuSleep        16
#define AsnuMMap         17
#define AsnuMUmap        18
#define AsnuAlloc        19
#define AsnuFree         20
#define AsnuRename       21
#define AsnuDup          22 /* Duplicate: <FILE> -> <FILE> (IDX)*/
#define AsnuCut          23 /* Move file */
#define AsnuTime         24
#define AsnuSystemUptime 25
#define AsnuGetDeviceHandle 26
#define AsnuPowerModeSet 27
#define AsnuGetTermWidth 28
#define AsnuGetTermHeight 29

extern void KiSyscallHandler();

#endif /* SYSCALLS_H */
