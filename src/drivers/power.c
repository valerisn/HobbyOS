#include "drivers/power.h"
#include "lib/lib.h"

void power_reboot(void)
{
    /* pulse the cpu reset line through the keyboard controller */
    outb(0x64, 0xFE);
    for (;;) __asm__ volatile("pause");
}

void power_off(void)
{
    /* qemu only: q35 and i440fx put the acpi shutdown port in different spots.
       on real hardware this does nothing and we just hang, needs a proper acpi parser */
    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);
    for (;;) __asm__ volatile("pause");
}
