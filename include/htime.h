#ifndef HTIME_H
#define HTIME_H

#include <efi.h>
#include <efilib.h>

STATIC
VOID
EFIAPI
TimerTickCallback(
  IN EFI_EVENT  Event,
  IN VOID      *Context
  );

EFI_STATUS
InitUefiTicker(
  IN UINTN  Freq
  );

STATIC
UINT64
GetUefiTicks(
  VOID
  );

STATIC
VOID
SleepTicks(
  IN UINT64  Timesleep
  );

#endif