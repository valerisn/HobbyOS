#include "apps/about.h"
#include "gfx/gfx.h"
#include "lib/lib.h"
#include "lib/version.h"
#include "ui/theme.h"

static void draw(const App *self, int x, int y, int w, int h)
{
    (void)self; (void)w; (void)h;
    int ix = x + 30, iy = y + 26;

    gfx_shadow(ix, iy, 68, 68, 18, 12, 90);
    gfx_rrect(ix, iy, 68, 68, 18, C_ALL, RGB(98, 166, 250), RGB(40, 92, 210), 255);
    gfx_rrect_border(ix, iy, 68, 68, 18, C_ALL, COL_WHITE, 50);
    int gw = gfx_text_width("H", F_BIG);
    gfx_text(ix + (68 - gw) / 2, iy + (68 - gfx_font_h(F_BIG)) / 2, "H", COL_WHITE, F_BIG);

    gfx_text(ix + 90, iy + 8, OS_NAME, COL_FG, F_BIG);
    gfx_text(ix + 90, iy + 40, "Version " OS_VERSION, COL_DIM, F_UI);

    gfx_rect(x + 24, y + 112, w - 48, 1, COL_WHITE, 22);

    char res[28], a[12], b[12];
    utoa(gfx_w, a); utoa(gfx_h, b);
    char *p = res;
    for (char *s = a; *s; s++) *p++ = *s;
    *p++ = 'x';
    for (char *s = b; *s; s++) *p++ = *s;
    *p = 0;

    const char *keys[] = { "Architecture", "Firmware", "Display", "Input" };
    const char *vals[] = { "x86_64", "UEFI", res, "PS/2, polled" };
    for (int i = 0; i < 4; i++) {
        int ry = y + 126 + i * 26;
        gfx_text(x + 30, ry, keys[i], COL_DIM, F_UI);
        gfx_text(x + 160, ry, vals[i], COL_FG, F_UI);
    }
}

static void icon(int x, int y, int hover)
{
    u32 c1 = RGB(98, 166, 250), c2 = RGB(40, 92, 210);
    if (hover) { c1 = gfx_mix(c1, COL_WHITE, 40); c2 = gfx_mix(c2, COL_WHITE, 40); }
    gfx_rrect(x, y, 48, 48, 12, C_ALL, c1, c2, 255);
    gfx_rrect_border(x, y, 48, 48, 12, C_ALL, COL_WHITE, 46);
    int gw = gfx_text_width("i", F_BIG);
    gfx_text(x + (48 - gw) / 2, y + (48 - gfx_font_h(F_BIG)) / 2, "i", COL_WHITE, F_BIG);
}

App about_app = {
    .title = "About",
    .win = { 330, 170, 420, 290, 0 },
    .body_col = RGB(36, 36, 42),
    .body_alpha = 255,
    .draw = draw,
    .icon = icon,
};
