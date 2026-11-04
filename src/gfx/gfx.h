#ifndef GFX_H
#define GFX_H
#include "lib/types.h"

#define RGB(r, g, b) (((u32)(r) << 16) | ((u32)(g) << 8) | (u32)(b))

enum { F_UI, F_SMALL, F_BOLD, F_BIG };

/* which corners get rounded */
#define C_TL 1
#define C_TR 2
#define C_BL 4
#define C_BR 8
#define C_ALL 15
#define C_TOP (C_TL | C_TR)
#define C_BOTTOM (C_BL | C_BR)

extern int gfx_w, gfx_h;

/* mem needs room for two full frames: the draw buffer and the cached wallpaper */
void gfx_init(void *fb, u32 pitch, int w, int h, int bgr, u32 *mem);
void gfx_wall_save(void);
void gfx_wall_restore(void);

u32 gfx_mix(u32 a, u32 b, int t);
void gfx_pixel(int x, int y, u32 c);
void gfx_blend(int x, int y, u32 c, int a);
void gfx_rect(int x, int y, int w, int h, u32 c, int a);
void gfx_rrect(int x, int y, int w, int h, int r, int mask, u32 c1, u32 c2, int a);
void gfx_rrect_border(int x, int y, int w, int h, int r, int mask, u32 c, int a);
void gfx_shadow(int x, int y, int w, int h, int r, int spread, int a);
void gfx_wallpaper(void);

int gfx_text(int x, int y, const char *s, u32 c, int font);
int gfx_text_width(const char *s, int font);
int gfx_font_h(int font);

void gfx_present(void);

#endif
