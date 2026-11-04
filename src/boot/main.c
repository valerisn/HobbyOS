#include "boot/efi.h"
#include "gfx/gfx.h"
#include "drivers/input.h"
#include "ui/desktop.h"

/* parked here because the map must not be allocated between GetMemoryMap and Exit */
static u8 mmap[1 << 16] __attribute__((aligned(8)));

static int pick_mode(Gop *gop)
{
    u32 best = gop->mode->mode;
    for (u32 i = 0; i < gop->mode->max_mode; i++) {
        GopInfo *info;
        u64 sz;
        if (gop->QueryMode(gop, i, &sz, &info)) continue;
        if (info->format > 1) continue;
        if (info->hres == 1024 && info->vres == 768) { best = i; break; }
    }
    if (best != gop->mode->mode) gop->SetMode(gop, best);
    return 0;
}

u64 efi_main(void *image, EfiSystemTable *st)
{
    EfiBootServices *bs = st->boot;
    EfiGuid guid = EFI_GOP_GUID;
    Gop *gop;

    bs->SetWatchdogTimer(0, 0, 0, 0);
    if (bs->LocateProtocol(&guid, 0, (void **)&gop)) return 1;

    pick_mode(gop);
    GopInfo *info = gop->mode->info;
    if (info->format > 1) return 1;

    /* two frames: the live buffer and the cached wallpaper */
    u32 *back;
    if (bs->AllocatePool(2, (u64)info->hres * info->vres * 8, (void **)&back)) return 1;

    gfx_init((void *)gop->mode->fb_base, info->pitch, info->hres, info->vres, info->format == 1, back);

    u64 size, key, dsize;
    u32 dver;
    for (int tries = 0; tries < 2; tries++) {
        size = sizeof(mmap);
        if (bs->GetMemoryMap(&size, mmap, &key, &dsize, &dver)) return 1;
        if (!bs->ExitBootServices(image, key)) break;
        if (tries) return 1;
    }

    /* firmware's idt is dead from here on, any interrupt would triple fault */
    __asm__ volatile("cli");

    input_init(gfx_w, gfx_h);
    desktop_init();

    int dirty = 1;
    for (;;) {
        if (input_poll()) dirty = 1;
        if (desktop_step()) dirty = 1;
        if (desktop_tick()) dirty = 1;
        if (dirty) {
            desktop_draw();
            gfx_present();
            dirty = 0;
        } else {
            __asm__ volatile("pause");
        }
    }
}
