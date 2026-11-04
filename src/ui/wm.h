#ifndef WM_H
#define WM_H
#include "apps/app.h"

void wm_register(App *a);
int wm_count(void);
App *wm_app(int i);

void wm_open(App *a);
/* index of the focused app, -1 if everything is closed */
int wm_top(void);

/* returns 1 when the click landed on a window */
int wm_click(int mx, int my);
/* call every step, returns 1 while a window is being dragged */
int wm_drag(int mx, int my, int down);
void wm_key(int c);
void wm_draw(void);

#endif
