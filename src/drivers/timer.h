#ifndef DRIVERS_TIMER_H
#define DRIVERS_TIMER_H
#include "lib/types.h"

#define TIMER_HZ 100

void timer_init(void);
u64 timer_ticks(void);
u64 timer_uptime(void);

#endif
