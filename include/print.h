#ifndef PRINT_H
#define PRINT_H

#include <efi.h>
#include <efilib.h>
#include <stdarg.h>
#include <stddef.h>
#include <kernel.h>

// Вспомогательный макрос для вычисления размера массива
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif

VOID
print(
    IN CHAR16 *fmt,
    ...
    );

#endif