#ifndef RTC_H
#define RTC_H

typedef struct {
    int h, m, s;
    int day, mon, year;
    int wday; /* 0 = sunday */
} Time;

void rtc_read(Time *t);

#endif
