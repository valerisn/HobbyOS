/* nothing but assembly lives here. one 16 byte stub per vector, all funnel into isr_common */

__asm__(
    ".text\n"
    ".globl isr_stubs\n"
    ".balign 16\n"
    "isr_stubs:\n"
    ".set vec, 0\n"
    ".rept 48\n"
    ".balign 16\n"
    /* these vectors already get an error code from the cpu, everything else needs a fake one */
    ".if (vec==8)|(vec==10)|(vec==11)|(vec==12)|(vec==13)|(vec==14)|(vec==17)|(vec==21)|(vec==29)|(vec==30)\n"
    ".else\n"
    "pushq $0\n"
    ".endif\n"
    "pushq $vec\n"
    "jmp isr_common\n"
    ".set vec, vec+1\n"
    ".endr\n"

    "isr_common:\n"
    "pushq %rax\n pushq %rcx\n pushq %rdx\n pushq %rbx\n pushq %rbp\n pushq %rsi\n pushq %rdi\n"
    "pushq %r8\n pushq %r9\n pushq %r10\n pushq %r11\n pushq %r12\n pushq %r13\n pushq %r14\n pushq %r15\n"
    /* the cpu aligns rsp to 16 on entry, so the stack is aligned right here */
    "movq %rsp, %rcx\n"
    /* handlers are plain C and the compiler is free to use xmm, so keep the interrupted code's copy */
    "subq $512, %rsp\n"
    "fxsave (%rsp)\n"
    "subq $32, %rsp\n"
    "call isr_handler\n"
    "addq $32, %rsp\n"
    "fxrstor (%rsp)\n"
    "addq $512, %rsp\n"
    "popq %r15\n popq %r14\n popq %r13\n popq %r12\n popq %r11\n popq %r10\n popq %r9\n popq %r8\n"
    "popq %rdi\n popq %rsi\n popq %rbp\n popq %rbx\n popq %rdx\n popq %rcx\n popq %rax\n"
    "addq $16, %rsp\n"
    "iretq\n"
);
