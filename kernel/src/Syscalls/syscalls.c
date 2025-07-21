#include "syscalls.h"
#include <FS/RamFS/ramfs.h>

extern uint64_t FB_WIDTH, FB_HEIGHT, FB_FLANTERM_CHAR_WIDTH, FB_FLANTERM_CHAR_HEIGHT;

typedef struct {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
} x86SysVRegState;

void x86GetRegSysV(x86SysVRegState* out) {
    __asm__ volatile (
        "mov %%rax, 0x00(%0)\n\t"
        "mov %%rbx, 0x08(%0)\n\t"
        "mov %%rcx, 0x10(%0)\n\t"
        "mov %%rdx, 0x18(%0)\n\t"
        "mov %%rsi, 0x20(%0)\n\t"
        "mov %%rdi, 0x28(%0)\n\t"
        "mov %%rbp, 0x30(%0)\n\t"
        "mov %%rsp, 0x38(%0)\n\t"
        "mov %%r8,  0x40(%0)\n\t"
        "mov %%r9,  0x48(%0)\n\t"
        "mov %%r10, 0x50(%0)\n\t"
        "mov %%r11, 0x58(%0)\n\t"
        "mov %%r12, 0x60(%0)\n\t"
        "mov %%r13, 0x68(%0)\n\t"
        "mov %%r14, 0x70(%0)\n\t"
        "mov %%r15, 0x78(%0)\n\t"
        :
        : "r"(out)
        : "memory"
    );
}

void x86SetRegSysV(x86SysVRegState* in) {
    __asm__ volatile (
        "mov 0x00(%0), %%rax\n\t"
        "mov 0x08(%0), %%rbx\n\t"
        "mov 0x10(%0), %%rcx\n\t"
        "mov 0x18(%0), %%rdx\n\t"
        "mov 0x20(%0), %%rsi\n\t"
        "mov 0x28(%0), %%rdi\n\t"
        "mov 0x30(%0), %%rbp\n\t"
        "mov 0x38(%0), %%rsp\n\t"
        "mov 0x40(%0), %%r8\n\t"
        "mov 0x48(%0), %%r9\n\t"
        "mov 0x50(%0), %%r10\n\t"
        "mov 0x58(%0), %%r11\n\t"
        "mov 0x60(%0), %%r12\n\t"
        "mov 0x68(%0), %%r13\n\t"
        "mov 0x70(%0), %%r14\n\t"
        "mov 0x78(%0), %%r15\n\t"
        :
        : "r"(in)
        : "memory"
    );
}

extern int CurrentPid;

void do_exit(uint64_t exit_code, int pid);

static void SyscallResetIF() {
	asm volatile ("sti");
}

char CurrentWorkingDirectory[RAMFS_PATH_MAX] = "/";

void SyscallHandler() {
    x86SysVRegState Registers;
    x86GetRegSysV(&Registers);
    
    SyscallResetIF();

    switch (Registers.rax) {
        case AsnuOpen:
        	Registers.rax = RamFsGetFdFromHandle(RamFsOpen((char*)Registers.rsi));
        	break;
        case AsnuClose:
        	RamFsClose(RamFsGetHandleFromFd(Registers.rsi));
        	Registers.rax = 0;
            break;
        case AsnuWrite:
            if (Registers.rsi == 1 || Registers.rsi == 2) {
                int len = Registers.r10;
                if (len <= 0) {
                    Registers.rax = 0;
                    break;
                }

                char* string = (char*)(void*)Registers.rdx;

                for (int i = 0; i < len; i++) {
                    printk("%c", (int)string[i]);
                }

                Registers.rax = len;
            } else {
                Registers.rax = -1;
            }
            break;
        case AsnuRead:
            if (Registers.rsi == 0) {
                char* out = (char*)(void*)Registers.rdx;
                uint64_t count = Registers.r10;
                Registers.rax = KiReadHidSN(out, count);
            } else {
            	if (Registers.rsi == 1 || Registers.rsi == 2) Registers.rax = -1;
            	else {
            		Registers.rax = RamFsRead(RamFsGetHandleFromFd(Registers.rsi), (void*)Registers.rdx, Registers.r10);
            	}
            }
            break;
        case AsnuLseek:
        	Registers.rax = RamFsLseek(RamFsGetHandleFromFd(Registers.rsi), Registers.rdx, Registers.r10);
        	break;
        case AsnuSeek:
        	Registers.rax = RamFsSeek(RamFsGetHandleFromFd(Registers.rsi), Registers.rdx);
            break;
        case AsnuGetPid:
            Registers.rax = CurrentPid;
            break;
        case AsnuExec:
        case AsnuListDir:
        	Registers.rax = (uint64_t)RamFsList(Registers.rsi);
        	break;
        case AsnuChdir:
        	Registers.rax = -1; /* CHDIR is deprecated, for now */
        	break;
        case AsnuGetCwd:
        	Registers.rax = (uint64_t)CurrentWorkingDirectory;
        	break;
        case AsnuCreateDir:
        	Registers.rax = RamFsRegisterDir((char*)Registers.rsi);
        	break;
        case AsnuRemoveDir:
        	Registers.rax = -1;
        	break;
        case AsnuRemoveFile:
        	Registers.rax = -1;
        	break;
        case AsnuCreateFile:
        	Registers.rax = RamFsRegister((char*)Registers.rsi, (uint8_t*)Registers.rdx, Registers.r10);
        	break;
        case AsnuExit:
            do_exit(Registers.rsi, CurrentPid);
            break;
        case AsnuSleep:
        case AsnuMMap:
            uint64_t virt = Registers.rsi;
            uint64_t phys = Registers.rdx;
            uint64_t attr = Registers.r10;
            attr |= MMAP_USER;
            attr |= MMAP_RW;
            attr |= MMAP_PRESENT;

            if (0x400000 <= virt && virt < 0x1000000) {}
            else {
                KiPanic("SBD: Detected security breach by process, quitting process", 0);
                do_exit(0xFFFFFFFFFFFFFFFF, CurrentPid);
                break;
            }

            if (0x400000 <= phys && phys < 0x1000000) {}
            else {
                KiPanic("SBD: Detected security breach by process, quitting process", 0);
                do_exit(0xFFFFFFFFFFFFFFFF, CurrentPid);
                break;
            }

            KiMMap((void*)virt, (void*)phys, attr);
            break;
        case AsnuMUmap:
            KiUMap((void*)Registers.rsi);
            break;
        case AsnuAlloc:
        	void* address_alloc = KiPmmAlloc();
        	Registers.rax = (uint64_t)address_alloc;
        	break;
        case AsnuFree:
        	KiPmmFree((void*)Registers.rsi);
        	break;
        case AsnuRename:
        case AsnuDup:
        	/* This is easy, just get the file the program wants to duplicate,
        	 * and create another copy of it, but make sure that the copy name is "{ORIGINAL_NAME} (Number of copies - 1)" */
			Registers.rax = -1;
			break;
        case AsnuCut:
        	/* Unless RamFsUnlink exists, this wont be a thing */
        	Registers.rax = -1;
        	break;
        case AsnuTime:
        case AsnuSystemUptime:
        case AsnuGetDeviceHandle:
        case AsnuPowerModeSet:
        	Registers.rax = -1;
            break;
        case AsnuGetTermWidth:
        	Registers.rax = FB_FLANTERM_CHAR_WIDTH;
        	break;
        case AsnuGetTermHeight:
        	Registers.rax = FB_FLANTERM_CHAR_HEIGHT;
    }

    x86SetRegSysV(&Registers);
}

void do_exit(uint64_t exit_code, int pid) {
    //KiPmmClearPidTracedResources(pid);

    UnloadKernelElf();

    if (exit_code != 0) {
        printk("\r[ Process %d quit with error code: %llu (0x%016X)]\n\r", CurrentPid, exit_code, exit_code);
    }

    extern void _kernel_idle_process();
    CurrentPid = 1;
    _kernel_idle_process();
}
