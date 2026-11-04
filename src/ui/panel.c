#include "ui/panel.h"
#include "ui/theme.h"
#include "ui/wm.h"
#include "apps/about.h"
#include "drivers/input.h"
#include "drivers/power.h"
#include "lib/lib.h"

#define MENU_W 210
#define ITEM_H 34
#define MENU_PAD 6
#define NITEMS 3

static int menu_open;
static const char *items[NITEMS] = { "About HobbyOS", "Restart", "Power Off" };

static int inside(int px, int py, int x, int y, int w, int h)
{
    return px >= x && py >= y && px < x + w && py < y + h;
}

static int pill_x(void) { return gfx_w - 42; }
static int menu_x(void) { return gfx_w - MENU_W - 8; }
static int menu_y(void) { return PANEL_H + 6; }
static int menu_h(void) { return NITEMS * ITEM_H + 2 * MENU_PAD; }

static int left_w(void) { return gfx_text_width("HobbyOS", F_BOLD) + 24; }

int panel_click(int mx, int my)
{
    if (menu_open && inside(mx, my, menu_x(), menu_y(), MENU_W, menu_h())) {
        int i = (my - menu_y() - MENU_PAD) / ITEM_H;
        menu_open = 0;
        if (i == 0) wm_open(&about_app);
        else if (i == 1) power_reboot();
        else if (i == 2) power_off();
        return 1;
    }

    int was_open = menu_open;
    menu_open = 0;

    if (my < PANEL_H) {
        if (mx >= pill_x() - 4) menu_open = !was_open;
        else if (mx < 6 + left_w()) wm_open(&about_app);
        return 1;
    }
    return was_open;
}

static void power_icon(int cx, int cy)
{
    gfx_rrect_border(cx - 7, cy - 7, 14, 14, 7, C_ALL, COL_WHITE, 230);
    gfx_rect(cx - 1, cy - 9, 2, 8, COL_WHITE, 255);
}

static void draw_menu(void)
{
    int x = menu_x(), y = menu_y(), h = menu_h();
    gfx_shadow(x, y, MENU_W, h, 14, 20, 110);
    gfx_rrect(x, y, MENU_W, h, 14, C_ALL, RGB(34, 34, 40), RGB(34, 34, 40), 246);
    gfx_rrect_border(x, y, MENU_W, h, 14, C_ALL, COL_WHITE, 36);

    for (int i = 0; i < NITEMS; i++) {
        int iy = y + MENU_PAD + i * ITEM_H;
        if (inside(mouse_x, mouse_y, x, iy, MENU_W, ITEM_H))
            gfx_rrect(x + 6, iy + 1, MENU_W - 12, ITEM_H - 2, 8, C_ALL, COL_WHITE, COL_WHITE, 28);
        gfx_text(x + 20, iy + (ITEM_H - gfx_font_h(F_UI)) / 2, items[i], COL_FG, F_UI);
    }
}

void panel_draw(const Time *t)
{
    static const char *days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    static const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    gfx_rect(0, 0, gfx_w, PANEL_H, RGB(12, 12, 16), 205);
    gfx_rect(0, PANEL_H - 1, gfx_w, 1, COL_WHITE, 18);

    int hover_l = inside(mouse_x, mouse_y, 6, 3, left_w(), PANEL_H - 6);
    if (hover_l)
        gfx_rrect(6, 3, left_w(), PANEL_H - 6, 12, C_ALL, COL_WHITE, COL_WHITE, 30);
    gfx_text(18, (PANEL_H - gfx_font_h(F_BOLD)) / 2, "HobbyOS", COL_FG, F_BOLD);

    char buf[40];
    char *p = buf;
    const char *parts[] = { days[t->wday % 7], " ", months[(t->mon - 1) % 12], " " };
    for (int i = 0; i < 4; i++) for (const char *s = parts[i]; *s; s++) *p++ = *s;
    char num[12];
    utoa(t->day, num);
    for (char *s = num; *s; s++) *p++ = *s;
    *p++ = ' '; *p++ = ' ';
    *p++ = '0' + t->h / 10 % 10; *p++ = '0' + t->h % 10; *p++ = ':';
    *p++ = '0' + t->m / 10 % 10; *p++ = '0' + t->m % 10;
    *p = 0;
    int tw = gfx_text_width(buf, F_BOLD);
    gfx_text((gfx_w - tw) / 2, (PANEL_H - gfx_font_h(F_BOLD)) / 2, buf, COL_FG, F_BOLD);

    int px = pill_x();
    int hover_r = inside(mouse_x, mouse_y, px - 4, 3, 36, PANEL_H - 6);
    if (hover_r || menu_open)
        gfx_rrect(px - 4, 3, 36, PANEL_H - 6, 12, C_ALL, COL_WHITE, COL_WHITE, menu_open ? 46 : 30);
    power_icon(px + 14, PANEL_H / 2 + 1);
    gfx_text(px - 4 - gfx_text_width("UTC", F_SMALL) - 8,
             (PANEL_H - gfx_font_h(F_SMALL)) / 2, "UTC", COL_DIM, F_SMALL);

    if (menu_open) draw_menu();
}
