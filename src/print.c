// Минимальная реализация функции Print для UEFI на C без
// интеграции GNU-EFI-библиотек. Форматирует:
//   %s – строка CHAR16*
//   %d – знаковое целое (INTN)
//   %u – беззнаковое целое (UINTN)
//   %x – беззнаковое целое в шестнадцатеричной (0x...)
//   %c – одиночный символ (CHAR16/UINTN)
//   %% – символ '%'
//
// Предполагается, что существует глобальная переменная:
//   extern EFI_SYSTEM_TABLE *SystemTable;
//
// А также что реализованы функции:
//   void *malloc(size_t), void free(void *).
//

#include <print.h>

// Объявляем, что загрузчик UEFI передаёт нам этот указатель глобально.
extern EFI_SYSTEM_TABLE *SystemTable;

// Максимальный размер буфера для одной строки вывода (в символах CHAR16).
#define MAX_PRINT_BUFFER 1024

/**
  Вывод отформатированного текста в консоль UEFI.

  @param[in]  fmt   Форматная строка (CHAR16*), поддерживает:
                    %s, %d, %u, %x, %c, %%.
  @param[in]  ...   Аргументы согласно спецификаторам.
**/
VOID
print(
    IN CHAR16 *fmt,
    ...
    )
{
    va_list args;
    va_start(args, fmt);

    // Выделяем буфер в CHAR16. Если не удалось — просто выходим.
    CHAR16 *buffer = (CHAR16 *)malloc(MAX_PRINT_BUFFER * sizeof(CHAR16));
    if (buffer == NULL) {
        va_end(args);
        return;
    }

    CHAR16 *out = buffer;      // указатель, куда пишем текущий символ
    CHAR16 *end = buffer + (MAX_PRINT_BUFFER - 1); // оставляем место для '\0'

    for (CHAR16 *p = fmt; *p != L'\0' && out < end; p++) {
        if (*p != L'%') {
            // Обычный символ – копируем “как есть”.
            *out++ = *p;
        } else {
            // Столкнулись с '%', переходим к следующему символу-спецификатору
            p++;
            switch (*p) {
                case L'%':
                    // "%%" => один символ '%'
                    *out++ = L'%';
                    break;

                case L'c': {
                    // Символ. В UEFI CHAR16 передаётся в va_arg как UINTN (или INTN),
                    // поэтому извлекаем именно UINTN и приводим к CHAR16.
                    UINTN ch = va_arg(args, UINTN);
                    *out++ = (CHAR16)ch;
                    break;
                }

                case L's': {
                    // Строка CHAR16*. Переписываем до '\0' или пока не упремся в конец.
                    CHAR16 *s = va_arg(args, CHAR16*);
                    if (s == NULL) {
                        // Защита от NULL – выводим L"(null)"
                        const CHAR16 *null_msg = L"(null)";
                        for (; *null_msg != L'\0' && out < end; null_msg++) {
                            *out++ = *null_msg;
                        }
                    } else {
                        for (; *s != L'\0' && out < end; s++) {
                            *out++ = *s;
                        }
                    }
                    break;
                }

                case L'd': {
                    // Знаковое десятичное. va_arg как INTN.
                    INTN val_signed = va_arg(args, INTN);
                    UINTN val;
                    if (val_signed < 0) {
                        *out++ = L'-';
                        // В UEFI обычно INTN — 64-битный, берём отрицательное в беззнаковое
                        val = (UINTN)(-val_signed);
                    } else {
                        val = (UINTN)val_signed;
                    }
                    // Формируем цифры “в обратном порядке” в tmp, затем выведем в нужном порядке.
                    CHAR16 tmp[32];
                    INTN idx = 0;
                    if (val == 0) {
                        tmp[idx++] = L'0';
                    } else {
                        while (val > 0 && idx < (INTN)ARRAY_SIZE(tmp)) {
                            tmp[idx++] = (CHAR16)(L'0' + (val % 10));
                            val /= 10;
                        }
                    }
                    // Копируем из tmp в буфер в нормальном порядке
                    while (idx > 0 && out < end) {
                        *out++ = tmp[--idx];
                    }
                    break;
                }

                case L'u': {
                    // Беззнаковое десятичное. va_arg как UINTN.
                    UINTN val = va_arg(args, UINTN);
                    CHAR16 tmp[32];
                    INTN idx = 0;
                    if (val == 0) {
                        tmp[idx++] = L'0';
                    } else {
                        while (val > 0 && idx < (INTN)ARRAY_SIZE(tmp)) {
                            tmp[idx++] = (CHAR16)(L'0' + (val % 10));
                            val /= 10;
                        }
                    }
                    while (idx > 0 && out < end) {
                        *out++ = tmp[--idx];
                    }
                    break;
                }

                case L'x': {
                    // Шестнадцатеричное беззнаковое. va_arg как UINTN.
                    UINTN val = va_arg(args, UINTN);
                    CHAR16 tmp[32];
                    INTN idx = 0;
                    // Если нужно выводить “0x...” — раскомментируйте две строки ниже.
                    // *out++ = L'0';
                    // *out++ = L'x';

                    if (val == 0) {
                        tmp[idx++] = L'0';
                    } else {
                        const CHAR16 *hex_digits = L"0123456789ABCDEF";
                        while (val > 0 && idx < (INTN)ARRAY_SIZE(tmp)) {
                            tmp[idx++] = hex_digits[val & 0xF];
                            val >>= 4;
                        }
                    }
                    // Копируем цифры в буфер в нормальном порядке
                    while (idx > 0 && out < end) {
                        *out++ = tmp[--idx];
                    }
                    break;
                }

                default:
                    // Нераспознанный спецификатор – просто выводим его “как следующий символ fmt”
                    *out++ = *p;
                    break;
            }
        }
    }

    // Завершающий нуль
    *out = L'\0';

    // Собственно, вызов UEFI-функции для вывода на экран
    gST->ConOut->OutputString(gST->ConOut, buffer);

    // Освобождаем буфер
    free(buffer);
    va_end(args);
}