#ifndef DESKTOP_H
#define DESKTOP_H

#define OS_NAME "HobbyOS"
#define OS_VERSION "0.1.0"

void desktop_init(void);
/* consume pending input, returns nonzero when something needs a redraw */
int desktop_step(void);
void desktop_draw(void);
/* polls the rtc, nonzero when the minute rolled over */
int desktop_tick(void);

#endif
