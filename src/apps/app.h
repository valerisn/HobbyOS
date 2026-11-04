#ifndef APP_H
#define APP_H
#include "lib/types.h"

typedef struct {
    int x, y, w, h;
    int open;
} Win;

typedef struct App {
    const char *title;
    Win win;
    u32 body_col;
    int body_alpha;
    /* draws the content area, the window manager handles the frame */
    void (*draw)(const struct App *self, int x, int y, int w, int h);
    void (*key)(int c);
    /* 48x48 tile for the dock */
    void (*icon)(int x, int y, int hover);
} App;

#endif
