#include "ui/wm.h"
#include "ui/theme.h"
#include "drivers/input.h"

#define MAX_APPS 8

static App *apps[MAX_APPS];
static int zorder[MAX_APPS]; /* bottom to top, holds indexes into apps */
static int napps;
static int drag = -1, drag_dx, drag_dy;

void wm_register(App *a)
{
    if (napps == MAX_APPS) return;
    zorder[napps] = napps;
    apps[napps++] = a;
}

int wm_count(void) { return napps; }
App *wm_app(int i) { return apps[i]; }

static void raise_idx(int k)
{
    int i = 0;
    while (zorder[i] != k) i++;
    for (; i < napps - 1; i++) zorder[i] = zorder[i + 1];
    zorder[napps - 1] = k;
}

void wm_open(App *a)
{
    for (int k = 0; k < napps; k++) {
        if (apps[k] != a) continue;
        a->win.open = 1;
        raise_idx(k);
    }
}

int wm_top(void)
{
    for (int i = napps - 1; i >= 0; i--)
        if (apps[zorder[i]]->win.open) return zorder[i];
    return -1;
}

static int inside(int px, int py, int x, int y, int w, int h)
{
    return px >= x && py >= y && px < x + w && py < y + h;
}

int wm_click(int mx, int my)
{
    for (int i = napps - 1; i >= 0; i--) {
        int k = zorder[i];
        Win *w = &apps[k]->win;
        if (!w->open || !inside(mx, my, w->x, w->y, w->w, w->h)) continue;

        raise_idx(k);
        if (my < w->y + TITLE_H) {
            if (mx >= w->x + w->w - 40) w->open = 0;
            else { drag = k; drag_dx = mx - w->x; drag_dy = my - w->y; }
        }
        return 1;
    }
    return 0;
}

int wm_drag(int mx, int my, int down)
{
    if (!down) { drag = -1; return 0; }
    if (drag < 0) return 0;

    Win *w = &apps[drag]->win;
    w->x = mx - drag_dx;
    w->y = my - drag_dy;
    if (w->y < PANEL_H) w->y = PANEL_H;
    if (w->y > gfx_h - TITLE_H) w->y = gfx_h - TITLE_H;
    return 1;
}

void wm_key(int c)
{
    int k = wm_top();
    if (k >= 0 && apps[k]->key) apps[k]->key(c);
}

static void draw_close(int cx, int cy, int hover)
{
    gfx_rrect(cx - 9, cy - 9, 18, 18, 9, C_ALL, hover ? RGB(224, 72, 72) : RGB(72, 72, 80),
              hover ? RGB(224, 72, 72) : RGB(72, 72, 80), 255);
    for (int i = -3; i <= 3; i++) {
        gfx_blend(cx + i, cy + i, COL_WHITE, 230);
        gfx_blend(cx + i, cy - i, COL_WHITE, 230);
        gfx_blend(cx + i + 1, cy + i, COL_WHITE, 70);
        gfx_blend(cx + i + 1, cy - i, COL_WHITE, 70);
    }
}

static void draw_one(App *a, int focused)
{
    Win *w = &a->win;
    gfx_shadow(w->x, w->y, w->w, w->h, WIN_RAD, 26, focused ? 120 : 70);

    u32 head = focused ? RGB(46, 46, 53) : RGB(36, 36, 42);
    gfx_rrect(w->x, w->y, w->w, TITLE_H, WIN_RAD, C_TOP, head, head, 255);
    gfx_rrect(w->x, w->y + TITLE_H, w->w, w->h - TITLE_H, WIN_RAD, C_BOTTOM,
              a->body_col, a->body_col, a->body_alpha);
    gfx_rect(w->x, w->y + TITLE_H - 1, w->w, 1, 0, 70);
    gfx_rrect_border(w->x, w->y, w->w, w->h, WIN_RAD, C_ALL, COL_WHITE, focused ? 42 : 24);

    int tw = gfx_text_width(a->title, F_BOLD);
    int th = gfx_font_h(F_BOLD);
    gfx_text(w->x + (w->w - tw) / 2, w->y + (TITLE_H - th) / 2, a->title,
             focused ? COL_FG : COL_DIM, F_BOLD);

    int cx = w->x + w->w - 24, cy = w->y + TITLE_H / 2;
    draw_close(cx, cy, inside(mouse_x, mouse_y, cx - 12, cy - 12, 24, 24));

    a->draw(a, w->x, w->y + TITLE_H, w->w, w->h - TITLE_H);
}

void wm_draw(void)
{
    int focus = wm_top();
    for (int i = 0; i < napps; i++) {
        int k = zorder[i];
        if (apps[k]->win.open) draw_one(apps[k], k == focus);
    }
}
