#ifndef INPUT_H
#define INPUT_H
#include "lib/types.h"

extern int mouse_x, mouse_y, mouse_btn;

void input_init(int w, int h);
/* called from the irq handlers */
void input_irq(void);
/* nonzero if anything arrived since the last call: mouse move, button, key */
int input_poll(void);
/* next typed char, 0 if none */
int input_key(void);

#endif
