#include "cpu/idt.h"
#include "cpu/gdt.h"
#include "cpu/regs.h"
#include "boot/panic.h"
#include "drivers/pic.h"

typedef struct __attribute__((packed)) {
    u16 lo;
    u16 sel;
    u8 ist;
    u8 attr;
    u16 mid;
    u32 hi;
    u32 zero;
} IdtEntry;

#define NVEC 48

extern char isr_stubs[];
static IdtEntry idt[256] __attribute__((aligned(16)));
static void (*irq_fn[16])(void);

void idt_init(void)
{
    for (int i = 0; i < NVEC; i++) {
        u64 a = (u64)isr_stubs + 16 * i;
        idt[i].lo = a & 0xFFFF;
        idt[i].sel = SEL_CODE;
        /* nmi, double fault, machine check each get their own known good stack */
        idt[i].ist = (i == 2 || i == 8 || i == 18) ? 1 : 0;
        idt[i].attr = 0x8E;
        idt[i].mid = (a >> 16) & 0xFFFF;
        idt[i].hi = a >> 32;
        idt[i].zero = 0;
    }

    struct __attribute__((packed)) { u16 limit; u64 base; } idtr = { sizeof(idt) - 1, (u64)idt };
    __asm__ volatile("lidt %0" :: "m"(idtr));
}

void irq_register(int irq, void (*fn)(void))
{
    irq_fn[irq] = fn;
}

void isr_handler(Regs *r)
{
    if (r->vec < 32) panic_screen(r);

    int irq = r->vec - 32;
    if (irq_fn[irq]) irq_fn[irq]();
    pic_eoi(irq);
}
