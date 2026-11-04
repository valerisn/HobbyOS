#include "desktop.h"
#include "gfx.h"
#include "input.h"
#include "lib.h"

#define TASKBAR_H 30
#define TITLE_H 22
#define LINE_H 16
#define LINE_LEN 96
#define WRAP 70
#define MAX_LINES 200

enum { W_TERM, W_ABOUT, W_COUNT };

typedef struct {
    int x, y, w, h;
    const char *title;
    int open;
} Win;

static Win wins[W_COUNT] = {
    { 150, 50, 520, 340, "Terminal", 1 },
    { 300, 160, 340, 190, "About", 0 },
};
static int zorder[W_COUNT] = { W_ABOUT, W_TERM };
static int drag = -1, drag_dx, drag_dy;
static int prev_down;
static int clock_h, clock_m, clock_s, clock_ticks;

static char lines[MAX_LINES][LINE_LEN];
static int nlines = 1;
static char cmdbuf[LINE_LEN];
static int cmdlen;

static const char *cursor_art[] = {
    "X           ",
    "XX          ",
    "X.X         ",
    "X..X        ",
    "X...X       ",
    "X....X      ",
    "X.....X     ",
    "X......X    ",
    "X.......X   ",
    "X........X  ",
    "X.....XXXXX ",
    "X..X..X     ",
    "X.X X..X    ",
    "XX  X..X    ",
    "X    X..X   ",
    "     X..X   ",
    "      X..X  ",
    "      X..X  ",
    "       XX   ",
};

static int cmos(int reg)
{
    outb(0x70, reg);
    return inb(0x71);
}

static int bcd(int v) { return (v & 0x0f) + (v >> 4) * 10; }

static void rtc_read(void)
{
    for (int i = 0; i < 1000 && (cmos(0x0A) & 0x80); i++) ;
    int s = cmos(0x00), m = cmos(0x02), h = cmos(0x04);
    if (!(cmos(0x0B) & 4)) { s = bcd(s); m = bcd(m); h = bcd(h & 0x7f); }
    clock_s = s; clock_m = m; clock_h = h;
}

static void two(char *out, int v)
{
    out[0] = '0' + v / 10 % 10;
    out[1] = '0' + v % 10;
}

static void newline(void)
{
    if (nlines == MAX_LINES) {
        memmove(lines[0], lines[1], (MAX_LINES - 1) * LINE_LEN);
        nlines--;
    }
    lines[nlines++][0] = 0;
}

static void term_putc(char c)
{
    char *cur = lines[nlines - 1];
    int n = strlen(cur);
    if (c == '\n') { newline(); return; }
    if (n >= WRAP) { newline(); cur = lines[nlines - 1]; n = 0; }
    cur[n] = c;
    cur[n + 1] = 0;
}

static void term_print(const char *s)
{
    while (*s) term_putc(*s++);
}

static void term_run(const char *cmd)
{
    while (*cmd == ' ') cmd++;
    if (!*cmd) return;

    if (streq(cmd, "help")) {
        term_print("commands: help clear echo time res ver reboot\n");
    } else if (streq(cmd, "clear")) {
        nlines = 1;
        lines[0][0] = 0;
    } else if (startswith(cmd, "echo ") || streq(cmd, "echo")) {
        term_print(cmd[4] ? cmd + 5 : "");
        term_print("\n");
    } else if (streq(cmd, "ver")) {
        term_print(OS_NAME " " OS_VERSION "\n");
    } else if (streq(cmd, "time")) {
        char t[] = "00:00:00 UTC\n";
        rtc_read();
        two(t, clock_h); two(t + 3, clock_m); two(t + 6, clock_s);
        term_print(t);
    } else if (streq(cmd, "res")) {
        char a[12], b[12];
        utoa(gfx_w, a); utoa(gfx_h, b);
        term_print(a); term_print("x"); term_print(b); term_print("\n");
    } else if (streq(cmd, "reboot")) {
        /* pulse the cpu reset line through the keyboard controller */
        outb(0x64, 0xFE);
        for (;;) __asm__ volatile("pause");
    } else {
        term_print("unknown command: ");
        term_print(cmd);
        term_print("\n");
    }
}

static void term_key(int c)
{
    if (c == '\n') {
        term_print("> ");
        term_print(cmdbuf);
        term_print("\n");
        term_run(cmdbuf);
        cmdlen = 0;
        cmdbuf[0] = 0;
    } else if (c == '\b') {
        if (cmdlen) cmdbuf[--cmdlen] = 0;
    } else if (c >= ' ' && c < 127 && cmdlen < WRAP - 3) {
        cmdbuf[cmdlen++] = c;
        cmdbuf[cmdlen] = 0;
    }
}

static void raise_win(int k)
{
    int i = 0;
    while (zorder[i] != k) i++;
    for (; i < W_COUNT - 1; i++) zorder[i] = zorder[i + 1];
    zorder[W_COUNT - 1] = k;
}

static void open_win(int k)
{
    wins[k].open = 1;
    raise_win(k);
}

static int top_win(void)
{
    for (int i = W_COUNT - 1; i >= 0; i--)
        if (wins[zorder[i]].open) return zorder[i];
    return -1;
}

static int inside(int px, int py, int x, int y, int w, int h)
{
    return px >= x && py >= y && px < x + w && py < y + h;
}

static const char *icon_label[W_COUNT] = { "Terminal", "About" };
static const char *icon_glyph[W_COUNT] = { ">_", "i" };

static int click(int mx, int my)
{
    if (my >= gfx_h - TASKBAR_H) {
        int bx = 100;
        for (int k = 0; k < W_COUNT; k++) {
            if (!wins[k].open) continue;
            if (inside(mx, my, bx, gfx_h - TASKBAR_H + 3, 110, TASKBAR_H - 6)) {
                raise_win(k);
                return 1;
            }
            bx += 116;
        }
        return 0;
    }

    for (int i = W_COUNT - 1; i >= 0; i--) {
        int k = zorder[i];
        Win *w = &wins[k];
        if (!w->open || !inside(mx, my, w->x, w->y, w->w, w->h)) continue;
        raise_win(k);
        if (my < w->y + TITLE_H) {
            if (mx >= w->x + w->w - 22) w->open = 0;
            else { drag = k; drag_dx = mx - w->x; drag_dy = my - w->y; }
        }
        return 1;
    }

    for (int k = 0; k < W_COUNT; k++) {
        if (inside(mx, my, 24, 24 + k * 92, 64, 80)) {
            open_win(k);
            return 1;
        }
    }
    return 0;
}

void desktop_init(void)
{
    rtc_read();
    lines[0][0] = 0;
    term_print(OS_NAME " " OS_VERSION "\n");
    term_print("type help for commands\n");
}

int desktop_step(void)
{
    int dirty = 0;
    int c;

    while ((c = input_key())) {
        if (top_win() == W_TERM) term_key(c);
        dirty = 1;
    }

    int down = mouse_btn & 1;
    if (down && !prev_down) dirty |= click(mouse_x, mouse_y);
    if (down && drag >= 0) {
        Win *w = &wins[drag];
        w->x = mouse_x - drag_dx;
        w->y = mouse_y - drag_dy;
        if (w->y < 0) w->y = 0;
        if (w->y > gfx_h - TASKBAR_H - TITLE_H) w->y = gfx_h - TASKBAR_H - TITLE_H;
        dirty = 1;
    }
    if (!down) drag = -1;
    prev_down = down;
    return dirty;
}

int desktop_tick(void)
{
    /* cmos reads trap in a vm, so don't do it every loop */
    if (++clock_ticks < 4096) return 0;
    clock_ticks = 0;
    int old = clock_m;
    rtc_read();
    return old != clock_m;
}

static void draw_term(Win *w)
{
    int bx = w->x + 8, by = w->y + TITLE_H + 6;
    int rows = (w->h - TITLE_H - 12) / LINE_H;
    int total = nlines;
    if (!lines[total - 1][0]) total--;

    int first = total - (rows - 1);
    if (first < 0) first = 0;
    int row = 0;
    for (int i = first; i < total; i++, row++)
        gfx_text(bx, by + row * LINE_H, lines[i], RGB(170, 230, 170));

    int px = bx + gfx_text(bx, by + row * LINE_H, "> ", RGB(255, 255, 255));
    px += gfx_text(px, by + row * LINE_H, cmdbuf, RGB(255, 255, 255));
    gfx_fill(px + 1, by + row * LINE_H + 12, 7, 2, RGB(255, 255, 255));
}

static void draw_about(Win *w)
{
    char buf[40], a[12], b[12];
    int x = w->x + 14, y = w->y + TITLE_H + 14;
    gfx_text(x, y, OS_NAME " " OS_VERSION, RGB(255, 255, 255));
    gfx_text(x, y + 24, "x86_64, UEFI framebuffer desktop", RGB(190, 200, 220));
    gfx_text(x, y + 42, "polled PS/2 input, no interrupts yet", RGB(190, 200, 220));
    utoa(gfx_w, a); utoa(gfx_h, b);
    buf[0] = 0;
    int n = 0;
    for (const char *s = a; *s; s++) buf[n++] = *s;
    buf[n++] = 'x';
    for (const char *s = b; *s; s++) buf[n++] = *s;
    buf[n] = 0;
    gfx_text(x, y + 66, "display:", RGB(130, 190, 255));
    gfx_text(x + 60, y + 66, buf, RGB(255, 255, 255));
}

void desktop_draw(void)
{
    for (int y = 0; y < gfx_h; y++)
        gfx_fill(0, y, gfx_w, 1, RGB(16 + y * 24 / gfx_h, 26 + y * 44 / gfx_h, 56 + y * 70 / gfx_h));

    for (int k = 0; k < W_COUNT; k++) {
        int ix = 24 + 4, iy = 24 + k * 92;
        gfx_fill(ix, iy, 56, 56, RGB(36, 44, 66));
        gfx_frame(ix, iy, 56, 56, RGB(120, 140, 190));
        int gw = gfx_text_width(icon_glyph[k]);
        gfx_text(ix + (56 - gw) / 2, iy + 20, icon_glyph[k], RGB(255, 255, 255));
        int lw = gfx_text_width(icon_label[k]);
        gfx_text(ix + (56 - lw) / 2, iy + 62, icon_label[k], RGB(255, 255, 255));
    }

    int focus = top_win();
    for (int i = 0; i < W_COUNT; i++) {
        int k = zorder[i];
        Win *w = &wins[k];
        if (!w->open) continue;

        gfx_fill(w->x + 5, w->y + 5, w->w, w->h, RGB(6, 8, 16));
        gfx_fill(w->x, w->y, w->w, w->h, k == W_TERM ? RGB(14, 16, 22) : RGB(30, 34, 46));
        gfx_frame(w->x, w->y, w->w, w->h, RGB(110, 120, 150));
        gfx_fill(w->x + 1, w->y + 1, w->w - 2, TITLE_H - 1, k == focus ? RGB(52, 92, 170) : RGB(66, 72, 90));
        gfx_text(w->x + 8, w->y + 4, w->title, RGB(255, 255, 255));
        gfx_fill(w->x + w->w - 20, w->y + 4, 16, 14, RGB(190, 64, 64));
        gfx_text(w->x + w->w - 16, w->y + 3, "x", RGB(255, 255, 255));

        if (k == W_TERM) draw_term(w);
        else draw_about(w);
    }

    int ty = gfx_h - TASKBAR_H;
    gfx_fill(0, ty, gfx_w, TASKBAR_H, RGB(24, 28, 40));
    gfx_fill(0, ty, gfx_w, 1, RGB(70, 80, 110));
    gfx_text(12, ty + 8, OS_NAME, RGB(130, 190, 255));

    int bx = 100;
    for (int k = 0; k < W_COUNT; k++) {
        if (!wins[k].open) continue;
        gfx_fill(bx, ty + 3, 110, TASKBAR_H - 6, k == focus ? RGB(52, 92, 170) : RGB(48, 54, 72));
        gfx_text(bx + 8, ty + 8, wins[k].title, RGB(255, 255, 255));
        bx += 116;
    }

    char t[] = "00:00 UTC";
    two(t, clock_h); two(t + 3, clock_m);
    gfx_text(gfx_w - gfx_text_width(t) - 12, ty + 8, t, RGB(200, 210, 230));

    for (int r = 0; r < 19; r++) {
        const char *row = cursor_art[r];
        for (int c = 0; row[c]; c++) {
            if (row[c] == 'X') gfx_pixel(mouse_x + c, mouse_y + r, RGB(0, 0, 0));
            else if (row[c] == '.') gfx_pixel(mouse_x + c, mouse_y + r, RGB(255, 255, 255));
        }
    }
}
