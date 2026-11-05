#include "apps/terminal.h"
#include "drivers/power.h"
#include "drivers/rtc.h"
#include "drivers/timer.h"
#include "gfx/gfx.h"
#include "lib/lib.h"
#include "lib/version.h"
#include "ui/theme.h"

#define LINE_H 19
#define LINE_LEN 96
#define WRAP 72
#define MAX_LINES 200

#define PROMPT_USER "hobby@hobbyos"
#define PROMPT_TAIL ":~$ "
#define PROMPT_LEN 17

static char lines[MAX_LINES][LINE_LEN];
static u8 is_cmd[MAX_LINES]; /* line starts with the prompt, drawn in colour */
static int nlines = 1;
static char cmdbuf[LINE_LEN];
static int cmdlen;

static void newline(void)
{
    if (nlines == MAX_LINES) {
        memmove(lines[0], lines[1], (MAX_LINES - 1) * LINE_LEN);
        memmove(is_cmd, is_cmd + 1, MAX_LINES - 1);
        nlines--;
    }
    lines[nlines][0] = 0;
    is_cmd[nlines++] = 0;
}

static void putc_(char c)
{
    char *cur = lines[nlines - 1];
    int n = strlen(cur);
    if (c == '\n') { newline(); return; }
    if (n >= WRAP) { newline(); cur = lines[nlines - 1]; n = 0; }
    cur[n] = c;
    cur[n + 1] = 0;
}

static void print(const char *s)
{
    while (*s) putc_(*s++);
}

static void print_num(u32 v)
{
    char b[12];
    utoa(v, b);
    print(b);
}

static void two(char *out, int v)
{
    out[0] = '0' + v / 10 % 10;
    out[1] = '0' + v % 10;
}

static void run(const char *cmd)
{
    while (*cmd == ' ') cmd++;
    if (!*cmd) return;

    if (streq(cmd, "help")) {
        print("help clear echo time uptime res ver neofetch crash reboot poweroff\n");
    } else if (streq(cmd, "clear")) {
        nlines = 1;
        lines[0][0] = 0;
        is_cmd[0] = 0;
    } else if (startswith(cmd, "echo")) {
        print(cmd[4] ? cmd + 5 : "");
        print("\n");
    } else if (streq(cmd, "ver")) {
        print(OS_NAME " " OS_VERSION "\n");
    } else if (streq(cmd, "time")) {
        Time t;
        char s[] = "00:00:00 UTC\n";
        rtc_read(&t);
        two(s, t.h); two(s + 3, t.m); two(s + 6, t.s);
        print(s);
    } else if (streq(cmd, "uptime")) {
        print_num(timer_uptime());
        print(" seconds, ");
        print_num(timer_ticks());
        print(" ticks\n");
    } else if (streq(cmd, "crash")) {
        /* on purpose, proves the exception path and crash screen work */
        __asm__ volatile("ud2");
    } else if (streq(cmd, "res")) {
        print_num(gfx_w); print("x"); print_num(gfx_h); print("\n");
    } else if (streq(cmd, "neofetch")) {
        print(PROMPT_USER "\n-------------\n");
        print("OS: " OS_NAME " " OS_VERSION "\n");
        print("Arch: x86_64\nFirmware: UEFI\nResolution: ");
        print_num(gfx_w); print("x"); print_num(gfx_h);
        print("\nShell: none, this is the whole shell\n");
    } else if (streq(cmd, "reboot")) {
        power_reboot();
    } else if (streq(cmd, "poweroff")) {
        power_off();
    } else {
        print(cmd);
        print(": command not found\n");
    }
}

static void key(int c)
{
    if (c == '\n') {
        is_cmd[nlines - 1] = 1;
        print(PROMPT_USER PROMPT_TAIL);
        print(cmdbuf);
        print("\n");
        run(cmdbuf);
        cmdlen = 0;
        cmdbuf[0] = 0;
    } else if (c == '\b') {
        if (cmdlen) cmdbuf[--cmdlen] = 0;
    } else if (c >= ' ' && c < 127 && cmdlen < WRAP - PROMPT_LEN) {
        cmdbuf[cmdlen++] = c;
        cmdbuf[cmdlen] = 0;
    }
}

static void draw_prompt(int x, int y)
{
    x += gfx_text(x, y, PROMPT_USER, COL_GREEN, F_UI);
    gfx_text(x, y, PROMPT_TAIL, RGB(120, 170, 255), F_UI);
}

static void draw(const App *self, int x, int y, int w, int h)
{
    (void)self; (void)w;
    int bx = x + 16, by = y + 12;
    int rows = (h - 24) / LINE_H;
    int total = nlines;
    if (!lines[total - 1][0]) total--;

    int first = total - (rows - 1);
    if (first < 0) first = 0;
    int row = 0;
    for (int i = first; i < total; i++, row++) {
        int ly = by + row * LINE_H;
        if (is_cmd[i]) {
            draw_prompt(bx, ly);
            gfx_text(bx + gfx_text_width(PROMPT_USER PROMPT_TAIL, F_UI), ly,
                     lines[i] + PROMPT_LEN, COL_FG, F_UI);
        } else {
            gfx_text(bx, ly, lines[i], RGB(204, 210, 222), F_UI);
        }
    }

    int ly = by + row * LINE_H;
    draw_prompt(bx, ly);
    int px = bx + gfx_text_width(PROMPT_USER PROMPT_TAIL, F_UI);
    px += gfx_text(px, ly, cmdbuf, COL_FG, F_UI);
    gfx_rect(px + 1, ly + 1, 8, 15, COL_ACCENT, 210);
}

static void icon(int x, int y, int hover)
{
    u32 c1 = RGB(70, 72, 84), c2 = RGB(30, 31, 38);
    if (hover) { c1 = gfx_mix(c1, COL_WHITE, 36); c2 = gfx_mix(c2, COL_WHITE, 36); }
    gfx_rrect(x, y, 48, 48, 12, C_ALL, c1, c2, 255);
    gfx_rrect_border(x, y, 48, 48, 12, C_ALL, COL_WHITE, 46);
    int gw = gfx_text_width(">_", F_BIG);
    gfx_text(x + (48 - gw) / 2, y + (48 - gfx_font_h(F_BIG)) / 2, ">_", COL_GREEN, F_BIG);
}

void terminal_init(void)
{
    print(OS_NAME " " OS_VERSION "\n");
    print("type help for commands\n");
}

App terminal_app = {
    .title = "Terminal",
    .win = { 170, 70, 600, 380, 0 },
    .body_col = RGB(22, 22, 27),
    .body_alpha = 238,
    .draw = draw,
    .key = key,
    .icon = icon,
};
