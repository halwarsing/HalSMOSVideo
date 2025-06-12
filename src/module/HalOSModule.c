#include <HalOSModule.h>


// Выполняет инструкцию CPUID с заданным EAX и ECX, возвращает результаты в регистры.
static
VOID
DoCpuid (
    IN  UINT32  EaxIn,
    IN  UINT32  EcxIn,
    OUT UINT32* EaxOut,
    OUT UINT32* EbxOut,
    OUT UINT32* EcxOut,
    OUT UINT32* EdxOut
    )
{
    // gcc-style inline assembly для x86_64 UEFI
    __asm__ __volatile__ (
        "cpuid"
        : "=a" (*EaxOut),
          "=b" (*EbxOut),
          "=c" (*EcxOut),
          "=d" (*EdxOut)
        : "a" (EaxIn), "c" (EcxIn)
    );
}

// Преобразует четыре колонок EBX, EDX, ECX (по 4 байта каждая) в 12-символьную строку vendor ID.
// В UEFI строки – UTF-16 (CHAR16), поэтому мы сначала собираем ASCII, а потом приводим в CHAR16.
static
VOID
GetCpuVendor (
    OUT CHAR16 VendorId[13]  // 12 символов + завершающий '\0'
    )
{
    UINT32 eax, ebx, ecx, edx;
    CHAR8  tmp[13];

    DoCpuid(0, 0, &eax, &ebx, &ecx, &edx);
    // EBX | EDX | ECX содержат vendor ID по стандарту CPUID
    *(UINT32*)&tmp[0]  = ebx;
    *(UINT32*)&tmp[4]  = edx;
    *(UINT32*)&tmp[8]  = ecx;
    tmp[12] = '\0';

    // Преобразуем в CHAR16
    for (INTN i = 0; i < 12; i++) {
        VendorId[i] = (CHAR16)tmp[i];
    }
    VendorId[12] = L'\0';
}

// Получает семейство и модель процессора через CPUID(EAX=1).
static
VOID
GetCpuFamilyModel (
    OUT UINT32* Family,
    OUT UINT32* Model
    )
{
    UINT32 eax, ebx, ecx, edx;
    DoCpuid(1, 0, &eax, &ebx, &ecx, &edx);

    // Биты 8-11 – базовое семейство, биты 20-27 – расширенное семейство
    UINT32 baseFamily = (eax >> 8) & 0xF;
    UINT32 extFamily  = (eax >> 20) & 0xFF;
    *Family = baseFamily;
    if (baseFamily == 0xF) {
        *Family += extFamily;
    }

    // Биты 4-7 – базовая модель, биты 16-19 – расширенная модель
    UINT32 baseModel = (eax >> 4) & 0xF;
    UINT32 extModel  = (eax >> 16) & 0xF;
    *Model = baseModel + (extModel << 4);
}

// Вычисляет суммарный объём всех областей памяти типа EfiConventionalMemory из таблицы памяти UEFI.
static
EFI_STATUS
ComputeTotalRam (
    IN  EFI_BOOT_SERVICES    *BootServices,
    OUT UINT64               *TotalRamBytes
    )
{
    EFI_STATUS                  Status;
    UINTN                       MapSize = 0;
    EFI_MEMORY_DESCRIPTOR       *MemMap = NULL;
    UINTN                       DescriptorSize = 0;
    UINT32                      DescriptorVersion = 0;
    UINTN                       MapKey = 0;
    UINTN                       MapEntries;
    UINTN                       Index;
    UINT64                      Total = 0;

    // Сначала узнать необходимый размер буфера
    Status = BootServices->GetMemoryMap(
        &MapSize,
        MemMap,
        &MapKey,
        &DescriptorSize,
        &DescriptorVersion
    );
    if (Status != EFI_BUFFER_TOO_SMALL) {
        return Status;
    }

    // Выделяем немного больше памяти на всякий случай
    MapSize += 2 * DescriptorSize;
    Status = BootServices->AllocatePool(
        EfiLoaderData,
        MapSize,
        (VOID**)&MemMap
    );
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Получаем саму таблицу памяти
    Status = BootServices->GetMemoryMap(
        &MapSize,
        MemMap,
        &MapKey,
        &DescriptorSize,
        &DescriptorVersion
    );
    if (EFI_ERROR(Status)) {
        BootServices->FreePool(MemMap);
        return Status;
    }

    // Количество записей = общий размер / размер одного дескриптора
    MapEntries = MapSize / DescriptorSize;

    for (Index = 0; Index < MapEntries; Index++) {
        EFI_MEMORY_DESCRIPTOR *Desc = (EFI_MEMORY_DESCRIPTOR *)
            ((UINT8*)MemMap + (Index * DescriptorSize));

        // Смотрим, если тип – EfiConventionalMemory (то есть обычная RAM)
        if (Desc->Type == EfiConventionalMemory) {
            // Размер в байтах = NumberOfPages * 4096 (страница = 4KiB)
            Total += (UINT64)Desc->NumberOfPages * 4096ULL;
        }
    }

    // Освобождаем буфер
    BootServices->FreePool(MemMap);

    *TotalRamBytes = Total;
    return EFI_SUCCESS;
}

HalSMCModule* HalOSModule_init(HalSMCompiler* hsmc)
{
    HalSMCModule* module=HalSMCModule_init(hsmc->memory,U8H("HalOS"));

    //Functions
    PutDictElementToDict(hsmc,module->lfuncs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("printSystemInfo")),
        HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,HalOSModule_printSystemInfo),HalSMVariableType_HalSMFunctionC)
    ));

    return module;
}



HalSMVariable* HalOSModule_printSystemInfo(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars)
{
    // --- Распознаём CPU ---
    CHAR16 Vendor[13];
    GetCpuVendor(Vendor);

    UINT32 Family, Model;
    GetCpuFamilyModel(&Family, &Model);

    print(L"Processor:\r\n");
    print(L"  Vendor ID: %s\r\n", Vendor);
    print(L"  Family: %u, Model: %u\r\n", Family, Model);

    // --- Собираем информацию по RAM ---
    UINT64 TotalRam = 0;
    EFI_STATUS Status = ComputeTotalRam(gST->BootServices, &TotalRam);
    if (!EFI_ERROR(Status)) {
        // Переведём в мегабайты (округляем вниз)
        UINT64 RamMiB = TotalRam / (1024 * 1024);
        print(L"RAM:\r\n");
        print(L"  Available (EfiConventionalMemory): %u MiB\r\n", (UINT32)RamMiB);
    } else {
        print(L"[Ошибка] Failed to get memory map: %r\r\n", Status);
    }
    
    return &HALSMNULLVAR;
}