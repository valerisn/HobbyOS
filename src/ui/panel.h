#ifndef UI_PANEL_H
#define UI_PANEL_H
#include "drivers/rtc.h"

void panel_draw(const Time *t);
/* returns 1 if the click was used up by the panel or its menu */
int panel_click(int mx, int my);

#endif
