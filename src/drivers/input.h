#ifndef INPUT_H
#define INPUT_H
#include "lib/types.h"

extern int mouse_x, mouse_y, mouse_btn;

void input_init(int w, int h);
/* nonzero if the mouse moved, a button changed, or a key got queued */
int input_poll(void);
/* next typed char, 0 if none */
int input_key(void);

#endif
