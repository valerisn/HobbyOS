#include "cpu/gdt.h"
#include "lib/types.h"

typedef struct __attribute__((packed)) {
    u32 rsvd0;
    u64 rsp[3];
    u64 rsvd1;
    u64 ist[7];
    u64 rsvd2;
    u16 rsvd3;
    u16 iomap;
} Tss;

static u64 gdt[5] __attribute__((aligned(16)));
static Tss tss;

/* double fault and friends land here so a blown stack still gets a crash screen */
static u8 ist1_stack[16384] __attribute__((aligned(16)));
static u8 ring0_stack[16384] __attribute__((aligned(16)));

void gdt_init(void)
{
    gdt[0] = 0;
    gdt[1] = 0x00AF9A000000FFFFULL;
    gdt[2] = 0x00CF92000000FFFFULL;

    tss.rsp[0] = (u64)(ring0_stack + sizeof(ring0_stack));
    tss.ist[0] = (u64)(ist1_stack + sizeof(ist1_stack));
    tss.iomap = sizeof(Tss);

    u64 base = (u64)&tss;
    u64 limit = sizeof(Tss) - 1;
    gdt[3] = (limit & 0xFFFF) | ((base & 0xFFFFFF) << 16) | (0x89ULL << 40) |
             (((limit >> 16) & 0xF) << 48) | (((base >> 24) & 0xFF) << 56);
    gdt[4] = base >> 32;

    struct __attribute__((packed)) { u16 limit; u64 base; } gdtr = { sizeof(gdt) - 1, (u64)gdt };
    __asm__ volatile("lgdt %0" :: "m"(gdtr));

    /* cs can only be reloaded with a far return */
    __asm__ volatile(
        "pushq %0\n"
        "leaq 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"
        "1:\n"
        "movw %1, %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%ss\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        :: "i"((u64)SEL_CODE), "i"((u16)SEL_DATA) : "rax", "memory");

    __asm__ volatile("ltr %0" :: "r"((u16)SEL_TSS));
}
