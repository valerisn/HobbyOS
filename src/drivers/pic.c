#include "drivers/pic.h"
#include "lib/lib.h"

static void wait(void) { outb(0x80, 0); }

void pic_init(void)
{
    /* irqs 0-15 go to vectors 32-47 so they stay clear of cpu exceptions */
    outb(0x20, 0x11); wait();
    outb(0xA0, 0x11); wait();
    outb(0x21, 0x20); wait();
    outb(0xA1, 0x28); wait();
    outb(0x21, 0x04); wait();
    outb(0xA1, 0x02); wait();
    outb(0x21, 0x01); wait();
    outb(0xA1, 0x01); wait();

    /* timer, keyboard, cascade on the master. mouse is irq 12, line 4 on the slave */
    outb(0x21, 0xF8);
    outb(0xA1, 0xEF);
}

void pic_eoi(int irq)
{
    if (irq >= 8) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
