#include "lib/lib.h"

/* without this gcc turns the loops back into calls to themselves */
#define NOLOOPCALL __attribute__((optimize("no-tree-loop-distribute-patterns")))

void *memcpy(void *d, const void *s, u64 n)
{
    void *r = d;
    __asm__ volatile("rep movsb" : "+D"(d), "+S"(s), "+c"(n) :: "memory");
    return r;
}

NOLOOPCALL void *memmove(void *d, const void *s, u64 n)
{
    u8 *dp = d;
    const u8 *sp = s;
    if (dp < sp) {
        while (n--) *dp++ = *sp++;
    } else {
        dp += n; sp += n;
        while (n--) *--dp = *--sp;
    }
    return d;
}

void *memset(void *d, int c, u64 n)
{
    void *r = d;
    __asm__ volatile("rep stosb" : "+D"(d), "+c"(n) : "a"(c) : "memory");
    return r;
}

u64 strlen(const char *s)
{
    u64 n = 0;
    while (s[n]) n++;
    return n;
}

int streq(const char *a, const char *b)
{
    while (*a && *a == *b) { a++; b++; }
    return *a == *b;
}

int startswith(const char *s, const char *p)
{
    while (*p) if (*s++ != *p++) return 0;
    return 1;
}

void utohex(u64 v, char *out)
{
    out[0] = '0';
    out[1] = 'x';
    for (int i = 0; i < 16; i++)
        out[2 + i] = "0123456789abcdef"[(v >> (60 - 4 * i)) & 15];
    out[18] = 0;
}

void utoa(u32 v, char *out)
{
    char tmp[12];
    int n = 0;
    do { tmp[n++] = '0' + v % 10; v /= 10; } while (v);
    while (n) *out++ = tmp[--n];
    *out = 0;
}
