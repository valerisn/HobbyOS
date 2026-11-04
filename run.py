#!/usr/bin/env python3
"""Builds, then boots the ESP folder in QEMU. Extra args go straight to qemu."""
import os
import shutil
import subprocess
import sys

root = os.path.dirname(os.path.abspath(__file__))
if subprocess.call([sys.executable, os.path.join(root, "build.py")]):
    sys.exit(1)

qemu = shutil.which("qemu-system-x86_64") or r"C:\Program Files\qemu\qemu-system-x86_64.exe"
fw = os.path.join(os.path.dirname(qemu), "share", "edk2-x86_64-code.fd")
if not os.path.exists(fw):
    fw = "/usr/share/OVMF/OVMF_CODE.fd"

vars_src = os.path.join(os.path.dirname(fw), "edk2-i386-vars.fd")
vars_fd = os.path.join(root, "build", "vars.fd")
if os.path.exists(vars_src) and not os.path.exists(vars_fd):
    shutil.copy(vars_src, vars_fd)

# ovmf wants code and nvram as separate pflash units, -bios choked on the 3.6MB image
fw_args = ["-drive", "if=pflash,format=raw,unit=0,readonly=on,file=" + fw]
if os.path.exists(vars_fd):
    fw_args += ["-drive", "if=pflash,format=raw,unit=1,file=" + vars_fd]

cmd = [qemu, "-machine", "q35", "-m", "256M", *fw_args,
       "-drive", "format=raw,file=fat:rw:" + os.path.join(root, "build", "esp"),
       "-device", "VGA", "-name", "HobbyOS", *sys.argv[1:]]
sys.exit(subprocess.call(cmd))
