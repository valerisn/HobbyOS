#include "gfx/gfx.h"
#include "gfx/font_data.h"

int gfx_w, gfx_h;
static u32 *fb, *bb, *wall;
static u32 pitch;
static int is_bgr;

static inline float fsqrt(float v)
{
    float r;
    __asm__("sqrtss %1, %0" : "=x"(r) : "x"(v));
    return r;
}

void gfx_init(void *fbase, u32 fpitch, int w, int h, int bgr, u32 *mem)
{
    fb = fbase; pitch = fpitch; gfx_w = w; gfx_h = h; is_bgr = bgr;
    bb = mem;
    wall = mem + (u64)w * h;
}

void gfx_wall_save(void) { __builtin_memcpy(wall, bb, (u64)gfx_w * gfx_h * 4); }
void gfx_wall_restore(void) { __builtin_memcpy(bb, wall, (u64)gfx_w * gfx_h * 4); }

/* a is 0..255, result leans toward c as a grows */
static inline u32 mixc(u32 p, u32 c, int a)
{
    if (a <= 0) return p;
    if (a >= 255) return c;
    int ia = 255 - a;
    u32 r = (((c >> 16) & 255) * a + ((p >> 16) & 255) * ia) / 255;
    u32 g = (((c >> 8) & 255) * a + ((p >> 8) & 255) * ia) / 255;
    u32 b = ((c & 255) * a + (p & 255) * ia) / 255;
    return (r << 16) | (g << 8) | b;
}

u32 gfx_mix(u32 a, u32 b, int t) { return mixc(a, b, t); }

void gfx_pixel(int x, int y, u32 c)
{
    if (x >= 0 && y >= 0 && x < gfx_w && y < gfx_h) bb[y * gfx_w + x] = c;
}

void gfx_blend(int x, int y, u32 c, int a)
{
    if (x < 0 || y < 0 || x >= gfx_w || y >= gfx_h) return;
    u32 *p = &bb[y * gfx_w + x];
    *p = mixc(*p, c, a);
}

static void span(int y, int x0, int x1, u32 c, int a)
{
    if (y < 0 || y >= gfx_h || a <= 0) return;
    if (x0 < 0) x0 = 0;
    if (x1 > gfx_w) x1 = gfx_w;
    u32 *row = bb + y * gfx_w;
    if (a >= 255) for (int i = x0; i < x1; i++) row[i] = c;
    else for (int i = x0; i < x1; i++) row[i] = mixc(row[i], c, a);
}

void gfx_rect(int x, int y, int w, int h, u32 c, int a)
{
    for (int j = 0; j < h; j++) span(y + j, x, x + w, c, a);
}

static int ccov(float dx, float dy, int r)
{
    float v = r + 0.5f - fsqrt(dx * dx + dy * dy);
    if (v <= 0) return 0;
    if (v >= 1) return 255;
    return (int)(v * 255);
}

void gfx_rrect(int x, int y, int w, int h, int r, int mask, u32 c1, u32 c2, int a)
{
    if (r * 2 > w) r = w / 2;
    if (r * 2 > h) r = h / 2;
    for (int j = 0; j < h; j++) {
        int py = y + j;
        u32 c = (c1 == c2) ? c1 : mixc(c1, c2, h > 1 ? j * 255 / (h - 1) : 0);
        int top = j < r, bot = j >= h - r;
        if (!top && !bot) { span(py, x, x + w, c, a); continue; }

        span(py, x + r, x + w - r, c, a);
        float dy = top ? (j + 0.5f - r) : (j + 0.5f - (h - r));
        int lb = top ? C_TL : C_BL, rb = top ? C_TR : C_BR;
        for (int i = 0; i < r; i++) {
            int cl = (mask & lb) ? ccov(i + 0.5f - r, dy, r) : 255;
            int cr = (mask & rb) ? ccov(i + 0.5f, dy, r) : 255;
            gfx_blend(x + i, py, c, a * cl / 255);
            gfx_blend(x + w - r + i, py, c, a * cr / 255);
        }
    }
}

static int cov_at(int px, int py, int x, int y, int w, int h, int r, int mask)
{
    if (px < x || py < y || px >= x + w || py >= y + h) return 0;
    if (r * 2 > w) r = w / 2;
    if (r * 2 > h) r = h / 2;
    int l = px < x + r, rt = px >= x + w - r, t = py < y + r, b = py >= y + h - r;
    if (!((l || rt) && (t || b))) return 255;
    int bit = t ? (l ? C_TL : C_TR) : (l ? C_BL : C_BR);
    if (!(mask & bit)) return 255;
    float cx = l ? x + r : x + w - r;
    float cy = t ? y + r : y + h - r;
    return ccov(px + 0.5f - cx, py + 0.5f - cy, r);
}

/* 1px ring: outer coverage minus the same shape shrunk by one pixel */
void gfx_rrect_border(int x, int y, int w, int h, int r, int mask, u32 c, int a)
{
    for (int j = 0; j < h; j++) {
        int py = y + j;
        if (py < 0 || py >= gfx_h) continue;
        int edge_row = j < r + 1 || j >= h - r - 1;
        for (int i = 0; i < w; i++) {
            if (!edge_row && i == 1) i = w - 2;
            int px = x + i;
            int o = cov_at(px, py, x, y, w, h, r, mask);
            int n = cov_at(px, py, x + 1, y + 1, w - 2, h - 2, r > 0 ? r - 1 : 0, mask);
            int cv = o - n;
            if (cv > 0) gfx_blend(px, py, c, a * cv / 255);
        }
    }
}

/* soft drop shadow, skips the area under the window itself since it gets covered anyway */
void gfx_shadow(int x, int y, int w, int h, int r, int spread, int a)
{
    int oy = spread / 3;
    float hw = w / 2.0f, hh = h / 2.0f;
    float cx = x + hw, cy = y + oy + hh;
    float ix = hw - r, iy = hh - r;

    for (int j = y + oy - spread; j < y + oy + h + spread; j++) {
        if (j < 0 || j >= gfx_h) continue;
        /* only the straight part, corner rows have wallpaper peeking through the rounding */
        int inrow = j >= y + r && j < y + h - r;
        float qy = j + 0.5f - cy;
        qy = (qy < 0 ? -qy : qy) - iy;
        for (int i = x - spread; i < x + w + spread; i++) {
            if (inrow && i == x) { i = x + w - 1; continue; }
            if (i < 0 || i >= gfx_w) continue;
            float qx = i + 0.5f - cx;
            qx = (qx < 0 ? -qx : qx) - ix;
            float d;
            if (qx > 0 && qy > 0) d = fsqrt(qx * qx + qy * qy) - r;
            else d = (qx > qy ? qx : qy) - r;
            if (d >= spread) continue;
            float t = 1.0f - d / spread;
            if (t > 1.0f) t = 1.0f;
            int al = (int)(a * t * t);
            if (al > 0) {
                u32 *p = &bb[j * gfx_w + i];
                *p = mixc(*p, 0, al);
            }
        }
    }
}

static float clampf(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }

/* purple to deep blue with two soft glows. runs once, the result gets cached */
void gfx_wallpaper(void)
{
    float W = gfx_w, H = gfx_h;
    float g1x = W * 0.78f, g1y = H * 0.22f, g1r = W * 0.55f;
    float g2x = W * 0.18f, g2y = H * 0.88f, g2r = W * 0.5f;
    for (int y = 0; y < gfx_h; y++) {
        for (int x = 0; x < gfx_w; x++) {
            float t = clampf((x / W) * 0.55f + (y / H) * 0.45f);
            float r = 38 + (14 - 38) * t;
            float g = 22 + (48 - 22) * t;
            float b = 78 + (92 - 78) * t;

            float dx = x - g1x, dy = y - g1y;
            float f = clampf(1.0f - (dx * dx + dy * dy) / (g1r * g1r));
            f = f * f * 0.5f;
            r += 150 * f; g += 70 * f; b += 120 * f;

            dx = x - g2x; dy = y - g2y;
            f = clampf(1.0f - (dx * dx + dy * dy) / (g2r * g2r));
            f = f * f * 0.4f;
            r += 20 * f; g += 150 * f; b += 150 * f;

            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            bb[y * gfx_w + x] = RGB((int)r, (int)g, (int)b);
        }
    }
}

static int glyph_index(char ch)
{
    if (ch < FONT_FIRST || ch >= FONT_FIRST + FONT_COUNT) ch = '?';
    return ch - FONT_FIRST;
}

int gfx_text(int x, int y, const char *s, u32 c, int font)
{
    const Font *f = &fonts[font];
    int sx = x;
    for (; *s; s++) {
        int gi = glyph_index(*s);
        const unsigned char *g = f->d + gi * f->h * f->cw;
        for (int r = 0; r < f->h; r++, g += f->cw)
            for (int col = 0; col < f->cw; col++)
                if (g[col]) gfx_blend(x + col - FONT_LP, y + r, c, g[col]);
        x += f->w[gi];
    }
    return x - sx;
}

int gfx_text_width(const char *s, int font)
{
    int w = 0;
    for (; *s; s++) w += fonts[font].w[glyph_index(*s)];
    return w;
}

int gfx_font_h(int font) { return fonts[font].h; }

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
