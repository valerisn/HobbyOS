#ifndef EFI_H
#define EFI_H

#include "lib/types.h"

/* mingw already targets the MS x64 ABI, which is what UEFI wants. no attributes needed */

typedef struct { u32 a; u16 b; u16 c; u8 d[8]; } EfiGuid;
typedef struct { u64 sig; u32 rev, size, crc, rsvd; } EfiHeader;

typedef struct {
    EfiHeader hdr;
    void *RaiseTPL, *RestoreTPL;
    u64 (*AllocatePages)(int type, int memtype, u64 pages, u64 *addr);
    void *FreePages;
    u64 (*GetMemoryMap)(u64 *size, void *map, u64 *key, u64 *dsize, u32 *dver);
    u64 (*AllocatePool)(int memtype, u64 size, void **buf);
    void *FreePool;
    void *CreateEvent, *SetTimer, *WaitForEvent, *SignalEvent, *CloseEvent, *CheckEvent;
    void *InstallProtocolInterface, *ReinstallProtocolInterface, *UninstallProtocolInterface;
    void *HandleProtocol, *Reserved, *RegisterProtocolNotify, *LocateHandle;
    void *LocateDevicePath, *InstallConfigurationTable;
    void *LoadImage, *StartImage, *Exit, *UnloadImage;
    u64 (*ExitBootServices)(void *image, u64 key);
    void *GetNextMonotonicCount;
    u64 (*Stall)(u64 us);
    u64 (*SetWatchdogTimer)(u64 timeout, u64 code, u64 dlen, u16 *data);
    void *ConnectController, *DisconnectController, *OpenProtocol, *CloseProtocol;
    void *OpenProtocolInformation, *ProtocolsPerHandle, *LocateHandleBuffer;
    u64 (*LocateProtocol)(const EfiGuid *proto, void *reg, void **iface);
} EfiBootServices;

typedef struct {
    EfiHeader hdr;
    void *vendor;
    u32 fwrev, pad;
    void *conin_h, *conin, *conout_h, *conout, *stderr_h, *stderr_;
    void *runtime;
    EfiBootServices *boot;
} EfiSystemTable;

typedef struct {
    u32 version, hres, vres, format;
    u32 rmask, gmask, bmask, resv;
    u32 pitch;
} GopInfo;

typedef struct {
    u32 max_mode, mode;
    GopInfo *info;
    u64 info_size;
    u64 fb_base;
    u64 fb_size;
} GopMode;

typedef struct Gop {
    u64 (*QueryMode)(struct Gop *, u32 n, u64 *size, GopInfo **info);
    u64 (*SetMode)(struct Gop *, u32 n);
    void *Blt;
    GopMode *mode;
} Gop;

#define EFI_GOP_GUID { 0x9042a9de, 0x23dc, 0x4a38, { 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a } }

#endif
