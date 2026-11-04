# HobbyOS

A small x86_64 hobby OS written from scratch in C. It boots through UEFI, takes over the framebuffer and runs a windowed desktop. No libc, no third-party kernel code.

## What works

- UEFI boot, GOP framebuffer, double buffered drawing
- Desktop with icons, taskbar, clock, draggable and stackable windows
- PS/2 keyboard and mouse (polled)
- A tiny terminal app (`help`, `clear`, `echo`, `time`, `res`, `ver`, `reboot`)

## What does not exist yet

Own GDT/IDT and interrupts, memory manager, scheduler, filesystem, USB input, real apps. Right now it still runs on the firmware's page tables and descriptor tables with interrupts disabled.

## Build and run (Windows)

Needs a mingw-w64 `gcc` on `PATH`, Python 3, and QEMU installed in the default location.

```
python run.py
```

`build.py` produces `build/esp/EFI/BOOT/BOOTX64.EFI`. `run.py` builds, then boots it in QEMU with the OVMF firmware that ships with QEMU. On Linux the same scripts work if `qemu-system-x86_64` and an OVMF package are installed.

To run on real hardware or a VM, copy `BOOTX64.EFI` to `EFI/BOOT/` on a FAT32 USB stick and boot it with Secure Boot off.

## Layout

```
src/main.c      UEFI entry, framebuffer setup, main loop
src/gfx.c       drawing primitives and text
src/input.c     i8042 keyboard and mouse
src/desktop.c   window manager, terminal, about window
src/lib.c       memcpy and friends
tools/genfont.py  regenerates src/font_data.h
```

The bundled font is Pillow's Aileron (CC0), rasterized by `tools/genfont.py`.

## License

MIT
