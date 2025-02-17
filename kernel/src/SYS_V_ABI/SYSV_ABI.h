//
// Created by Adam on 2/6/2025.
//

#ifndef SYSV_ABI_H
#define SYSV_ABI_H

#define ELF_SYSV_ABI 0x01

typedef enum {
    rdi,
    rsi,
    rdx,
    rcx,
    r8,
    r9,
    rbx,
    rbp,
    r12,
    r13,
    r14,
    r15,
    rsp,
    STACK_ELEMENT_FIRST,
    STACK_ELEMENT_LAST,
} SysVRegOrder;

// Define an array or a vector of strings representing the registers
const char* regNames[] = {
    "rdi", "rsi", "rdx", "rcx", "r8", "r9", "rbx", "rbp",
    "r12", "r13", "r14", "r15", "rsp"
};

#define SYSV_REG_ORDER(i) regNames[i]

#endif //SYSV_ABI_H
