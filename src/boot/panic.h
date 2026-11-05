#ifndef BOOT_PANIC_H
#define BOOT_PANIC_H
#include "cpu/regs.h"

/* paints the crash screen and halts, never returns */
void panic_screen(const Regs *r) __attribute__((noreturn));

#endif
