#include "drivers/rtc.h"
#include "lib/lib.h"

static int cmos(int reg)
{
    outb(0x70, reg);
    return inb(0x71);
}

static int bcd(int v) { return (v & 0x0f) + (v >> 4) * 10; }

void rtc_read(Time *t)
{
    /* bit 7 of reg A means an update is in flight, values are garbage until it clears */
    for (int i = 0; i < 1000 && (cmos(0x0A) & 0x80); i++) ;

    int s = cmos(0x00), m = cmos(0x02), h = cmos(0x04);
    int d = cmos(0x07), mo = cmos(0x08), y = cmos(0x09);
    if (!(cmos(0x0B) & 4)) {
        s = bcd(s); m = bcd(m); h = bcd(h & 0x7f);
        d = bcd(d); mo = bcd(mo); y = bcd(y);
    }
    t->s = s; t->m = m; t->h = h;
    t->day = d; t->mon = mo; t->year = 2000 + y;

    /* the weekday register is unreliable on some firmware, so work it out */
    static const int off[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    int yy = t->year - (mo < 3);
    t->wday = (yy + yy / 4 - yy / 100 + yy / 400 + off[(mo - 1) % 12] + d) % 7;
}
