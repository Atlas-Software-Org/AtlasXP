#ifndef SYSCALLS_H
#define SYSCALLS_H 1

#include <KiSimple.h>
#include <Drivers/PS2Keyboard.h>
#include <PMM/pmm.h>
#include <VMM/vmm.h>
#include <ELF/elf.h>
#include <stdint.h>

#define AxpOpen         0
#define AxpClose        1
#define AxpWrite        2
#define AxpRead         3
#define AxpLseek        4
#define AxpSeek         5
#define AxpGetPid       6
#define AxpExec         7
#define AxpListDir      8
#define AxpChdir        9
#define AxpGetCwd       10
#define AxpCreateDir    11
#define AxpRemoveDir    12
#define AxpRemoveFile   13
#define AxpCreateFile   14
#define AxpExit         15
#define AxpSleep        16
#define AxpMMap         17
#define AxpMUmap        18
#define AxpAlloc        19
#define AxpFree         20
#define AxpRename       21
#define AxpDup          22 /* Duplicate: <FILE> -> <FILE> (IDX)*/
#define AxpCut          23 /* Move file */
#define AxpTime         24
#define AxpSystemUptime 25
#define AxpGetDeviceHandle 26
#define AxpPowerModeSet 27

__attribute__((interrupt)) void KiSyscallHandler(int *__unused);

#endif /* SYSCALLS_H */
