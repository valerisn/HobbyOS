# HobbyOS

A small x86_64 hobby OS written from scratch in C. It boots through UEFI, takes over the framebuffer and runs a dark, GNOME-ish desktop. No libc, no third-party kernel code.

## What works

- UEFI boot, GOP framebuffer, double buffered software rendering
- Anti-aliased text, rounded windows, soft shadows, translucency
- Top panel with clock and power menu, floating dock, draggable stacking windows
- Own GDT, TSS and IDT, PIC and 100Hz PIT timer, interrupt-driven PS/2 keyboard and mouse, `hlt` when idle
- Crash screen with registers for any CPU exception, RTC clock
- Terminal app (`help`, `clear`, `echo`, `time`, `uptime`, `res`, `ver`, `neofetch`, `crash`, `reboot`, `poweroff`) and an About app

## What does not exist yet

Memory manager, scheduler, filesystem, USB input, APIC, real apps. It still runs on the firmware's page tables and the legacy 8259 PIC. Power off only works under QEMU.

## Build and run

Needs a mingw-w64 `gcc` on `PATH`, Python 3 and QEMU (default Windows install path, or on `PATH`).

```
python run.py
```

`build.py` produces `build/esp/EFI/BOOT/BOOTX64.EFI`. `run.py` builds, then boots it in QEMU with the OVMF firmware that ships with QEMU. On Linux the same scripts work if `qemu-system-x86_64` and an OVMF package are installed.

To try it on hardware or another VM, copy `BOOTX64.EFI` to `EFI/BOOT/` on a FAT32 USB stick and boot it with Secure Boot off.

## Layout

```
src/
  boot/      UEFI types and entry point, main loop, crash screen
  cpu/       GDT, IDT, interrupt stubs
  lib/       freestanding libc bits, shared types, version
  drivers/   PIC, PIT timer, PS/2 input, RTC, power
  gfx/       framebuffer, alpha blending, rounded rects, shadows, text
  ui/        window manager, panel, dock, desktop compositor, theme
  apps/      terminal, about, and the App interface they plug into
tools/
  genfont.py regenerates src/gfx/font_data.h
```

New apps define an `App` (see `src/apps/app.h`) and register it in `desktop_init`. The dock and window manager pick it up from there.

The bundled font is Pillow's Aileron (CC0), rasterized by `tools/genfont.py`.

## License

MIT
