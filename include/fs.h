#ifndef FS_H
#define FS_H

#include <efi.h>
#include <efilib.h>

EFI_STATUS
ReadFileToBuffer(
    IN  CHAR16  *FilePath,
    OUT VOID   **OutBufferPtr,
    OUT UINTN   *OutFileSize
    );

#endif