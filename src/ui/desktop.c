#include "ui/desktop.h"
#include "ui/dock.h"
#include "ui/panel.h"
#include "ui/wm.h"
#include "apps/about.h"
#include "apps/terminal.h"
#include "drivers/input.h"
#include "drivers/rtc.h"
#include "drivers/timer.h"
#include "gfx/gfx.h"

static Time now;
static int prev_down;
static u64 last_sec;

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

void desktop_init(void)
{
    rtc_read(&now);
    gfx_wallpaper();
    gfx_wall_save();

    terminal_app.win.open = 1;
    wm_register(&terminal_app);
    wm_register(&about_app);
    terminal_init();
}

int desktop_step(void)
{
    int dirty = 0;
    int c;

    while ((c = input_key())) {
        wm_key(c);
        dirty = 1;
    }

    int down = mouse_btn & 1;
    if (down && !prev_down) {
        dirty = 1;
        if (!panel_click(mouse_x, mouse_y) && !dock_click(mouse_x, mouse_y))
            wm_click(mouse_x, mouse_y);
    }
    dirty |= wm_drag(mouse_x, mouse_y, down);
    prev_down = down;
    return dirty;
}

int desktop_tick(void)
{
    u64 sec = timer_uptime();
    if (sec == last_sec) return 0;
    last_sec = sec;

    int old = now.m;
    rtc_read(&now);
    /* the about window shows uptime, so it needs a repaint every second */
    return old != now.m || about_app.win.open;
}

static void draw_cursor(void)
{
    for (int r = 0; r < 19; r++) {
        const char *row = cursor_art[r];
        for (int c = 0; row[c]; c++) {
            if (row[c] == ' ') continue;
            gfx_blend(mouse_x + c + 1, mouse_y + r + 2, 0, 70);
        }
    }
    for (int r = 0; r < 19; r++) {
        const char *row = cursor_art[r];
        for (int c = 0; row[c]; c++) {
            if (row[c] == 'X') gfx_pixel(mouse_x + c, mouse_y + r, RGB(10, 10, 14));
            else if (row[c] == '.') gfx_pixel(mouse_x + c, mouse_y + r, RGB(250, 250, 252));
        }
    }
}

void desktop_draw(void)
{
    gfx_wall_restore();
    wm_draw();
    panel_draw(&now);
    dock_draw();
    draw_cursor();
}
