#include "drivers/timer.h"
#include "cpu/idt.h"
#include "lib/lib.h"

static volatile u64 ticks;

static void on_tick(void) { ticks++; }

void timer_init(void)
{
    u32 div = 1193182 / TIMER_HZ;
    outb(0x43, 0x36);
    outb(0x40, div & 0xFF);
    outb(0x40, div >> 8);
    irq_register(0, on_tick);
}

u64 timer_ticks(void) { return ticks; }
u64 timer_uptime(void) { return ticks / TIMER_HZ; }
