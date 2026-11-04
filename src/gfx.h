#ifndef GFX_H
#define GFX_H
#include "efi.h"

#define RGB(r, g, b) (((u32)(r) << 16) | ((u32)(g) << 8) | (u32)(b))

extern int gfx_w, gfx_h;

void gfx_init(void *fb, u32 pitch, int w, int h, int bgr, u32 *backbuf);
void gfx_fill(int x, int y, int w, int h, u32 c);
void gfx_frame(int x, int y, int w, int h, u32 c);
void gfx_pixel(int x, int y, u32 c);
int gfx_text(int x, int y, const char *s, u32 c);
int gfx_text_width(const char *s);
void gfx_present(void);

#endif
