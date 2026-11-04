#include "gfx.h"
#include "font_data.h"

int gfx_w, gfx_h;
static u32 *fb, *bb;
static u32 pitch;
static int is_bgr;

void gfx_init(void *fbase, u32 fpitch, int w, int h, int bgr, u32 *backbuf)
{
    fb = fbase; pitch = fpitch; gfx_w = w; gfx_h = h; is_bgr = bgr; bb = backbuf;
}

void gfx_pixel(int x, int y, u32 c)
{
    if (x >= 0 && y >= 0 && x < gfx_w && y < gfx_h) bb[y * gfx_w + x] = c;
}

void gfx_fill(int x, int y, int w, int h, u32 c)
{
    int x1 = x + w, y1 = y + h;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x1 > gfx_w) x1 = gfx_w;
    if (y1 > gfx_h) y1 = gfx_h;
    for (int j = y; j < y1; j++)
        for (int i = x; i < x1; i++) bb[j * gfx_w + i] = c;
}

void gfx_frame(int x, int y, int w, int h, u32 c)
{
    gfx_fill(x, y, w, 1, c);
    gfx_fill(x, y + h - 1, w, 1, c);
    gfx_fill(x, y, 1, h, c);
    gfx_fill(x + w - 1, y, 1, h, c);
}

static int glyph(int x, int y, char ch, u32 c)
{
    if (ch < FONT_FIRST || ch >= FONT_FIRST + FONT_COUNT) ch = '?';
    int i = ch - FONT_FIRST;
    for (int r = 0; r < FONT_H; r++) {
        u16 bits = font_rows[i][r];
        for (int col = 0; bits && col < 16; col++, bits <<= 1)
            if (bits & 0x8000) gfx_pixel(x + col, y + r, c);
    }
    return font_w[i];
}

int gfx_text(int x, int y, const char *s, u32 c)
{
    int sx = x;
    for (; *s; s++) x += glyph(x, y, *s, c);
    return x - sx;
}

int gfx_text_width(const char *s)
{
    int w = 0;
    for (; *s; s++) {
        char ch = (*s < FONT_FIRST || *s >= FONT_FIRST + FONT_COUNT) ? '?' : *s;
        w += font_w[ch - FONT_FIRST];
    }
    return w;
}

void gfx_present(void)
{
    for (int y = 0; y < gfx_h; y++) {
        u32 *dst = fb + (u64)y * pitch;
        u32 *src = bb + y * gfx_w;
        if (is_bgr) {
            __builtin_memcpy(dst, src, (u64)gfx_w * 4);
        } else {
            for (int x = 0; x < gfx_w; x++) {
                u32 p = src[x];
                dst[x] = (p & 0xff00) | ((p >> 16) & 0xff) | ((p & 0xff) << 16);
            }
        }
    }
}
