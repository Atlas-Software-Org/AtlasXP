#ifndef EA688D5E_B0F9_42AE_83E2_FA72C56A2A5B
#define EA688D5E_B0F9_42AE_83E2_FA72C56A2A5B

struct InterruptFrame;
struct RegFrame {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
};

#endif /* EA688D5E_B0F9_42AE_83E2_FA72C56A2A5B */
