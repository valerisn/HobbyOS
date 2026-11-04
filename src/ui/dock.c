#include "ui/dock.h"
#include "ui/theme.h"
#include "ui/wm.h"
#include "drivers/input.h"

#define ICON 48
#define GAP 14
#define PAD 12
#define DOCK_H 72

static int dock_w(void) { return wm_count() * ICON + (wm_count() - 1) * GAP + 2 * PAD; }
static int dock_x(void) { return (gfx_w - dock_w()) / 2; }
static int dock_y(void) { return gfx_h - DOCK_H - 10; }
static int icon_x(int k) { return dock_x() + PAD + k * (ICON + GAP); }
static int icon_y(void) { return dock_y() + (DOCK_H - ICON) / 2; }

static int in_dock(int px, int py)
{
    return px >= dock_x() && px < dock_x() + dock_w() && py >= dock_y() && py < dock_y() + DOCK_H;
}

static int icon_at(int px, int py)
{
    if (!in_dock(px, py)) return -1;
    for (int k = 0; k < wm_count(); k++)
        if (px >= icon_x(k) - GAP / 2 && px < icon_x(k) + ICON + GAP / 2) return k;
    return -1;
}

int dock_click(int mx, int my)
{
    if (!in_dock(mx, my)) return 0;
    int k = icon_at(mx, my);
    if (k >= 0) wm_open(wm_app(k));
    return 1;
}

void dock_draw(void)
{
    int n = wm_count();
    if (!n) return;

    int dx = dock_x(), dy = dock_y(), dw = dock_w();
    gfx_shadow(dx, dy, dw, DOCK_H, 20, 22, 100);
    gfx_rrect(dx, dy, dw, DOCK_H, 20, C_ALL, RGB(24, 24, 30), RGB(24, 24, 30), 205);
    gfx_rrect_border(dx, dy, dw, DOCK_H, 20, C_ALL, COL_WHITE, 34);

    int hover = icon_at(mouse_x, mouse_y);
    int focus = wm_top();
    for (int k = 0; k < n; k++) {
        App *a = wm_app(k);
        int lift = (k == hover) ? 4 : 0;
        a->icon(icon_x(k), icon_y() - lift, k == hover);
        if (a->win.open)
            gfx_rrect(icon_x(k) + ICON / 2 - 2, dy + DOCK_H - 9, 5, 5, 2, C_ALL,
                      COL_WHITE, COL_WHITE, k == focus ? 230 : 120);
    }

    if (hover >= 0) {
        const char *t = wm_app(hover)->title;
        int tw = gfx_text_width(t, F_UI) + 22;
        int tx = icon_x(hover) + ICON / 2 - tw / 2;
        int ty = dy - 38;
        gfx_shadow(tx, ty, tw, 28, 10, 12, 80);
        gfx_rrect(tx, ty, tw, 28, 10, C_ALL, RGB(10, 10, 14), RGB(10, 10, 14), 230);
        gfx_rrect_border(tx, ty, tw, 28, 10, C_ALL, COL_WHITE, 30);
        gfx_text(tx + 11, ty + (28 - gfx_font_h(F_UI)) / 2, t, COL_FG, F_UI);
    }
}
