#ifndef HT_KERNEL_TOOLS_H
#define HT_KERNEL_TOOLS_H

#include <limine.h>
//#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <float.h>
#include <stdalign.h>
#include <stdnoreturn.h>

// For Variables
#define GET_VAR_ADDRESS(var) ((void*)&(var))

// For Functions
#define GET_FUNC_ADDRESS(func) ((void*)(func))

/*

typedef struct {
    CPU NAME
    CPU VENDOR
    CPU MANUFACTURER
    CPU SPEED
    CPU CORE COUNT
} cpu_t;

typedef struct {
    GPU NAME
    GPU VENDOR
    GPU MANUFACTURER
    GPU HAS CUDA CORES?
    GPU CUDA CORES COUNT
    GPU FRAMEBUFFER START
} gpu_t;

typedef struct {
    NIC NAME
    NIC VENDOR
    NIC MANUFACTURER
    NIC SEND PACKET FUNCTION
    NIC READ PACKET FUNCTION
    NIC IPV4 ADDRESS
    NIC IPV6 ADDRESS
    NIC MAC ADDRESS
} nic_t;

typedef struct {
    IDE NAME
    IDE VENDOR
    IDE MANUFACTURER
    IDE WRITE FUNCTION
    IDE READ FUNCTION
    IDE SECTOR COUNT
    IDE-SLAVE SECTOR COUNT
} ide_t;

typedef struct {
    AHCI NAME
    AHCI VENDOR
    AHCI MANUFACTURER
    AHCI WRITE FUNCTION
    AHCI READ FUNCTION
    AHCI SECTOR COUNT
    AHCI-SLAVE SECTOR COUNT
} ahci_t;

*/

typedef struct {
    uint8_t KernelVerification[4]; // 0xAFAB9999
    volatile uint32_t* FramebufferPointer;
    struct limine_framebuffer* Framebuffer;
    void* SpareBootInfoAddr;
    bool HtHasGDT;
    bool HtHasIDT;
    bool HtHasSyscall;
    bool HtHasInt80;
    bool isDebugging;
    void* void_rsdp;
    uint64_t u64_rsdp;
    uint64_t rsdp;
    /*
    cpu_t Cpu;
    gpu_t Gpu;
    nic_t Nic;
    ide_t Ide;
    ahci_t Ahci;
    */
} BootInfo;

typedef int KernelStatus;

// Boot and Initialization Errors
#define KERNEL_STARTUP_SUCCESS            0    // Successful kernel startup
#define KERNEL_STARTUP_FAILURE            1    // General startup failure

// Memory-related errors during boot
#define KERNEL_MEMORY_SETUP_FAILED        2    // Failed to set up memory (paging or physical memory)
#define KERNEL_INVALID_MEMORY_LAYOUT      3    // Invalid memory layout detected during boot

// Bootloader and hardware-related errors
#define KERNEL_BOOTLOADER_COMM_ERROR      4    // Bootloader communication error (e.g., passing wrong params)
#define KERNEL_UNSUPPORTED_HARDWARE       5    // Hardware not supported during boot
#define KERNEL_BOOTLOADER_NOT_FOUND       6    // Bootloader not found or failed to load

// CPU and architecture-related errors
#define KERNEL_CPU_NOT_SUPPORTED          7    // Unsupported CPU detected
#define KERNEL_INVALID_CPU_MODE           8    // CPU is in an invalid mode (e.g., protected mode or real mode)

#define KERNEL_BOOT_TIMEOUT               9    // Boot timeout (failed to complete within expected time)
#define KERNEL_INVALID_BOOT_PARAMETERS    10   // Invalid boot parameters passed from bootloader (e.g., kernel image path)
#define KERNEL_DISK_NOT_FOUND             11   // Boot disk not found or inaccessible during startup

// Filesystem errors related to bootloader or early initialization
#define KERNEL_FS_NOT_FOUND               12   // Root filesystem not found
#define KERNEL_FS_READ_ERROR              13   // Error reading root filesystem during boot

// General boot errors
#define KERNEL_UNKNOWN_STARTUP_ERROR      14   // Unknown error occurred during startup process

#endif // HT_KERNEL_TOOLS_H
