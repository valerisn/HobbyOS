#include "boot/panic.h"
#include "gfx/gfx.h"
#include "lib/lib.h"
#include "ui/theme.h"

static const char *names[32] = {
    "Divide error", "Debug", "Non-maskable interrupt", "Breakpoint",
    "Overflow", "Bound range exceeded", "Invalid opcode", "Device not available",
    "Double fault", "Coprocessor overrun", "Invalid TSS", "Segment not present",
    "Stack fault", "General protection fault", "Page fault", "Reserved",
    "x87 floating point", "Alignment check", "Machine check", "SIMD floating point",
    "Virtualization", "Control protection", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
};

static void row(int x, int y, const char *label, u64 v)
{
    char h[19];
    utohex(v, h);
    gfx_text(x, y, label, COL_DIM, F_UI);
    gfx_text(x + 70, y, h, COL_FG, F_UI);
}

void panic_screen(const Regs *r)
{
    __asm__ volatile("cli");

    u64 cr2;
    __asm__ volatile("movq %%cr2, %0" : "=r"(cr2));

    gfx_rect(0, 0, gfx_w, gfx_h, RGB(34, 10, 16), 255);

    int w = 640, h = 360;
    int x = (gfx_w - w) / 2, y = (gfx_h - h) / 2;
    gfx_shadow(x, y, w, h, 14, 30, 140);
    gfx_rrect(x, y, w, h, 14, C_ALL, RGB(30, 30, 36), RGB(30, 30, 36), 255);
    gfx_rrect_border(x, y, w, h, 14, C_ALL, RGB(255, 110, 110), 90);

    gfx_text(x + 30, y + 24, "Kernel panic", RGB(255, 120, 120), F_BIG);
    const char *name = r->vec < 32 ? names[r->vec] : "Unknown";
    char v[12];
    utoa((u32)r->vec, v);
    int nx = x + 30;
    nx += gfx_text(nx, y + 62, name, COL_FG, F_BOLD);
    nx += gfx_text(nx, y + 62, "  (vector ", COL_DIM, F_UI);
    nx += gfx_text(nx, y + 62, v, COL_DIM, F_UI);
    gfx_text(nx, y + 62, ")", COL_DIM, F_UI);
    gfx_rect(x + 30, y + 94, w - 60, 1, COL_WHITE, 26);

    int cx = x + 30, cy = y + 112;
    row(cx, cy, "RIP", r->rip);
    row(cx, cy + 26, "RSP", r->rsp);
    row(cx, cy + 52, "RFLAGS", r->rflags);
    row(cx, cy + 78, "ERR", r->err);
    row(cx, cy + 104, "CR2", cr2);
    row(cx, cy + 130, "CS", r->cs);

    cx = x + 330;
    row(cx, cy, "RAX", r->rax);
    row(cx, cy + 26, "RBX", r->rbx);
    row(cx, cy + 52, "RCX", r->rcx);
    row(cx, cy + 78, "RDX", r->rdx);
    row(cx, cy + 104, "RSI", r->rsi);
    row(cx, cy + 130, "RDI", r->rdi);

    gfx_text(x + 30, y + h - 34, "The system has been halted. Restart the machine to continue.", COL_DIM, F_UI);
    gfx_present();

    for (;;) __asm__ volatile("hlt");
}
