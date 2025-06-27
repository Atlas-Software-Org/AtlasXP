#include "AxpApi.h"
#include <stdint.h>

static inline uint64_t DispatchSyscall_0(uint64_t vector) {
    uint64_t ret;
    __asm__ volatile (
        "mov %%rax, %[vec]\n\t"
        "int $0x80\n\t"
        : "=a"(ret)
        : [vec] "r"(vector)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline uint64_t DispatchSyscall_1(uint64_t vector, uint64_t a1) {
    uint64_t ret;
    __asm__ volatile (
        "mov %%rax, %[vec]\n\t"
        "mov %%rsi, %[p1]\n\t"
        "int $0x80\n\t"
        : "=a"(ret)
        : [vec] "r"(vector), [p1] "r"(a1)
        : "rdi", "rcx", "r11", "memory"
    );
    return ret;
}

static inline uint64_t DispatchSyscall_2(uint64_t vector, uint64_t a1, uint64_t a2) {
    uint64_t ret;
    __asm__ volatile (
        "mov %%rax, %[vec]\n\t"
        "mov %%rsi, %[p1]\n\t"
        "mov %%rdx, %[p2]\n\t"
        "int $0x80\n\t"
        : "=a"(ret)
        : [vec] "r"(vector), [p1] "r"(a1), [p2] "r"(a2)
        : "rdi", "rsi", "rcx", "r11", "memory"
    );
    return ret;
}

static inline uint64_t DispatchSyscall_3(uint64_t vector, uint64_t a1, uint64_t a2, uint64_t a3) {
    uint64_t ret;
    __asm__ volatile (
        "mov %%rax, %[vec]\n\t"
        "mov %%rsi, %[p1]\n\t"
        "mov %%rdx, %[p2]\n\t"
        "mov %%r10, %[p3]\n\t"
        "int $0x80\n\t"
        : "=a"(ret)
        : [vec] "r"(vector), [p1] "r"(a1), [p2] "r"(a2), [p3] "r"(a3)
        : "rdi", "rsi", "rdx", "rcx", "r11", "memory"
    );
    return ret;
}

int write(int fd, void* buf, int count) {
	return (int)DispatchSyscall_3(2, (uint64_t)fd, (uint64_t)buf, (uint64_t)count);
}

int read(int fd, void* buf, int count) {
	return (int)DispatchSyscall_3(3, (uint64_t)fd, (uint64_t)buf, (uint64_t)count);
}

int axp_api_getpid() {
	return (int)DispatchSyscall_0(6);
}

void axp_api_exit(int code) {
	DispatchSyscall_1(15, (uint64_t)code);
}

void mmap(void* virt, void* phys, uint64_t attr) {
	uint64_t attr_ = attr;
#ifndef __SYSCALL_ALLOW_KERNEL_MAPPING__
	if (attr & ~(1 << 2))
		attr |= (1 << 2);
#endif
	DispatchSyscall_3(17, (uint64_t)virt, (uint64_t)phys, attr_);
}

void umap(void* virt) {
	DispatchSyscall_1(18, (uint64_t)virt);
}

void* memalloc() {
	return (void*)DispatchSyscall_0(19);
}

void memfree(void* ptr) {
	DispatchSyscall_1(20, (uint64_t)ptr);
}

uint64_t api_get_term_width() {
	return DispatchSyscall_0(28);
}

uint64_t api_get_term_height() {
	return DispatchSyscall_0(29);
}
