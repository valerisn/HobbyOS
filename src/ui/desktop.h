#ifndef DESKTOP_H
#define DESKTOP_H

void desktop_init(void);
/* consume pending input, returns nonzero when something needs a redraw */
int desktop_step(void);
void desktop_draw(void);
/* polls the rtc now and then, nonzero when the minute rolled over */
int desktop_tick(void);

#endif
