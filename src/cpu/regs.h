#ifndef CPU_REGS_H
#define CPU_REGS_H
#include "lib/types.h"

/* layout mirrors what isr_common pushes, first field is the last thing pushed */
typedef struct {
    u64 r15, r14, r13, r12, r11, r10, r9, r8;
    u64 rdi, rsi, rbp, rbx, rdx, rcx, rax;
    u64 vec, err;
    u64 rip, cs, rflags, rsp, ss;
} Regs;

#endif
