#!/usr/bin/env python3
"""Builds build/esp/EFI/BOOT/BOOTX64.EFI with the mingw gcc already on PATH."""
import glob
import os
import subprocess
import sys

root = os.path.dirname(os.path.abspath(__file__))
out = os.path.join(root, "build", "esp", "EFI", "BOOT")
os.makedirs(out, exist_ok=True)

cflags = ["-O2", "-std=gnu11", "-Wall", "-Wextra", "-ffreestanding", "-fno-stack-protector",
          "-fno-builtin", "-mno-red-zone", "-fshort-wchar", "-fno-asynchronous-unwind-tables",
          "-I" + os.path.join(root, "src")]
ldflags = ["-nostdlib", "-shared", "-Wl,--subsystem,10", "-Wl,-e,efi_main", "-s"]

srcs = sorted(glob.glob(os.path.join(root, "src", "**", "*.c"), recursive=True))
cmd = ["gcc", *cflags, *ldflags, "-o", os.path.join(out, "BOOTX64.EFI"), *srcs]
sys.exit(subprocess.call(cmd))
