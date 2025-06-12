#include <efi.h>
#include <efilib.h>
#include <kernel.h>
#include <HalSM.h>
#include "fs.h"
#include <efiprot.h>
#include <PrintModule.h>
#include <SnakeGame.h>
#include <HalOSModule.h>
#include <htime.h>

EFI_HANDLE HLibImageHandle;
EFI_RUNTIME_SERVICES    *RT;

// Minimal EFI function to read a file into memory without relying on external Protocol headers.
//
// This code only depends on <efi.h> and <efilib.h> (from GNU‐EFI or EDK II). It defines the necessary
// GUIDs and protocol structures inline so you can compile it even if <Protocol/...> headers are missing.

//=========================
// GUID definitions
//=========================


/**
  ReadFileToBuffer
  ----------------
  Opens a file (given by a Unicode path, e.g. L"\\script.hsm" or L"\\EFI\\BOOT\\config.json")
  on the same FAT volume from which the current EFI application was launched. Reads its entire contents
  into a buffer allocated with AllocatePool, and returns that buffer and its size.

  Caller must FreePool the returned buffer when completed.

  @param  FilePath       IN   Path to the file in Unicode (CHAR16*), e.g. L"\\script.hsm"
  @param  OutBufferPtr   OUT  Receives the address of the allocated buffer
  @param  OutFileSize    OUT  Receives the number of bytes read into the buffer

  @retval EFI_SUCCESS           File was read successfully
  @retval EFI_NOT_FOUND         File not found on the volume
  @retval EFI_OUT_OF_RESOURCES  Memory allocation failed
  @retval EFI_DEVICE_ERROR      I/O error while reading file
  @retval any other EFI error   Protocol or API error
**/
EFI_STATUS
ReadFileToBuffer(
    IN  CHAR16 *FilePath,
    OUT VOID   **OutBufferPtr,
    OUT UINTN   *OutFileSize
    )
{
    EFI_STATUS                       Status;
    EFI_LOADED_IMAGE_PROTOCOL       *LoadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFs;
    EFI_FILE_PROTOCOL               *RootDir;
    EFI_FILE_PROTOCOL               *FileHandle;
    EFI_FILE_INFO                   *FileInfo;
    UINTN                            FileInfoSize;
    VOID                            *Buffer;
    UINTN                            BufferSize;
    UINTN                            ReadSize;

    if (FilePath == NULL || OutBufferPtr == NULL || OutFileSize == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    // 1) Locate the LoadedImage protocol to get DeviceHandle
    Status = gBS->HandleProtocol(
        HLibImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (VOID**)&LoadedImage
    );
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // 2) Locate the SimpleFileSystem protocol on that DeviceHandle
    Status = gBS->HandleProtocol(
        LoadedImage->DeviceHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID**)&SimpleFs
    );
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // 3) Open the volume (root directory)
    Status = SimpleFs->OpenVolume(SimpleFs, &RootDir);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // 4) Open the file for reading
    Status = RootDir->Open(
        RootDir,
        &FileHandle,
        FilePath,
        EFI_FILE_MODE_READ,
        0
    );
    if (EFI_ERROR(Status)) {
        RootDir->Close(RootDir);
        return EFI_NOT_FOUND;
    }

    // 5) Query file size via GetInfo(EFI_FILE_INFO)
    FileInfoSize = sizeof(EFI_FILE_INFO) + 256;
    Status = gBS->AllocatePool(
        EfiLoaderData,
        FileInfoSize,
        (VOID**)&FileInfo
    );
    if (EFI_ERROR(Status)) {
        FileHandle->Close(FileHandle);
        RootDir->Close(RootDir);
        return EFI_OUT_OF_RESOURCES;
    }

    Status = FileHandle->GetInfo(
        FileHandle,
        &gEfiFileInfoGuid,
        &FileInfoSize,
        FileInfo
    );
    if (EFI_ERROR(Status)) {
        FileHandle->Close(FileHandle);
        RootDir->Close(RootDir);
        gBS->FreePool(FileInfo);
        return Status;
    }

    BufferSize = (UINTN)FileInfo->FileSize;
    gBS->FreePool(FileInfo);

    // 6) Allocate buffer for entire file contents
    Status = gBS->AllocatePool(
        EfiLoaderData,
        BufferSize,
        &Buffer
    );
    if (EFI_ERROR(Status)) {
        FileHandle->Close(FileHandle);
        RootDir->Close(RootDir);
        return EFI_OUT_OF_RESOURCES;
    }

    // 7) Read file into Buffer
    ReadSize = BufferSize;
    Status = FileHandle->Read(FileHandle, &ReadSize, Buffer);
    if (EFI_ERROR(Status) || ReadSize != BufferSize) {
        FileHandle->Close(FileHandle);
        RootDir->Close(RootDir);
        gBS->FreePool(Buffer);
        return EFI_DEVICE_ERROR;
    }

    // 8) Close file and root directory
    FileHandle->Close(FileHandle);
    RootDir->Close(RootDir);

    *OutBufferPtr = Buffer;
    *OutFileSize  = BufferSize;
    return EFI_SUCCESS;
}


VOID
RUNTIMEFUNCTION
HalCopyMem (
    IN VOID        *Dest,
    IN CONST VOID  *Src,
    IN UINTN       len
    )
{
    CHAR8 *d = (CHAR8*)Dest;
    CHAR8 *s = (CHAR8*)Src;

    if (d == NULL || s == NULL || s == d)
        return;

    // If the beginning of the destination range overlaps with the end of
    // the source range, make sure to start the copy from the end so that
    // we don't end up overwriting source data that we need for the copy.
    if ((d > s) && (d < s + len)) {
        for (d += len, s += len; len--; )
            *--d = *--s;
    } else {
        while (len--)
            *d++ = *s++;
    }
}

VOID
RUNTIMEFUNCTION
HalZeroMem (
    IN VOID     *Buffer,
    IN UINTN     Size
    )
{
    INT8        *pt;

    pt = Buffer;
    while (Size--) {
        *(pt++) = 0;
    }
}

//Memory

void * malloc(UINTN poolSize)
{
    EFI_STATUS status;
    void * handle;
    status=BS->AllocatePool(EfiLoaderData,poolSize+sizeof(UINTN),&handle);

    if(status == EFI_OUT_OF_RESOURCES)
    {
        ST->ConOut->OutputString(ST->ConOut,L"out of resources for pool\n");
        return 0;
    }
    else if(status == EFI_INVALID_PARAMETER)
    {
        ST->ConOut->OutputString(ST->ConOut,L"invalid pool type\n");
        return 0;
    }
    
    *(UINTN*)handle=poolSize;
    return (void*)(((UINTN*)handle)+1);
}

void free(void * pool)
{
    EFI_STATUS status;
    status = BS->FreePool((void*)(((UINTN*)pool)-1));

    if(status == EFI_INVALID_PARAMETER)
    {
        ST->ConOut->OutputString(ST->ConOut,L"invalid pool pointer\n");
    }
}

void* realloc(void* ptr,ui64 size) {
    if (ptr == NULL) {
        // Если ptr == NULL, просто malloc, как в стандарте C
        return malloc(size);
    }

    UINTN* dp=((UINTN*)ptr)-1;

    // Размер пользовательских данных, которые были аллоцированы (исключая заголовок):
    unsigned long long int OldDataSize = *dp;
    unsigned long long int NewBlockSize = size + sizeof(UINTN);

    if (size == 0) {
        // Стандарт C: realloc(ptr, 0) = free(ptr) + возвращаем NULL
        free(ptr);
        return NULL;
    }

    // Выделяем новый блок под новые данные (+заголовок):
    void* newData = malloc(NewBlockSize);
    if (newData == NULL) {
        // не удалось аллоцировать больше — возвращаем NULL (старый ptr остаётся живым)
        return NULL;
    }

    // Сколько байт копировать из старой области: минимум между тем,
    // что было выделено пользователю, и тем, что сейчас просит он.
    unsigned long long int CopyBytes = (OldDataSize < size) ? OldDataSize : size;
    HalCopyMem(newData, ptr, CopyBytes);

    // Освобождаем старый блок
    free(ptr);

    return newData;
}

void* calloc(ui64 num,ui64 size) {
    if(num==0||size==0)return 0;
    void* ptr=malloc(num*size);

    HalZeroMem(ptr,num*size);

    return ptr;
}

VOID
HalInitializeLib (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
/*++

Routine Description:

    Initializes EFI library for use
    
Arguments:

    Firmware's EFI system table
    
Returns:

    None

--*/ 
{
    EFI_LOADED_IMAGE        *LoadedImage;
    EFI_STATUS              Status;
    //CHAR8                   *LangCode;

    HLibImageHandle = ImageHandle;

    //
    // Set up global pointer to the system table, boot services table,
    // and runtime services table
    //

    ST = SystemTable;
    BS = SystemTable->BootServices;
    RT = SystemTable->RuntimeServices;
    // ASSERT (CheckCrc(0, &ST->Hdr));
    // ASSERT (CheckCrc(0, &BS->Hdr));
    // ASSERT (CheckCrc(0, &RT->Hdr));

    //
    // Initialize pool allocation type
    //

    if (ImageHandle) {
        Status=BS->HandleProtocol(ImageHandle,&LoadedImageProtocol,(VOID*)&LoadedImage);

        if (!EFI_ERROR(Status)) {
            //PoolAllocationType = LoadedImage->ImageDataType;
        }
    }

    //
    // Initialize Guid table
    //

    //InitializeLibPlatform(ImageHandle,SystemTable);

    /*if (ImageHandle && UnicodeInterface == &LibStubUnicodeInterface) {
        LangCode = LibGetVariable (VarLanguage, &EfiGlobalVariable);
        InitializeUnicodeSupport (LangCode);
        if (LangCode) {
            FreePool (LangCode);
        }
    }*/
}

#define MAX_INPUT_LEN  256   // максимальная длина строки (в символах)

/**
  ReadLine — считывает строку с консоли до нажатия Enter.
  - Символы эхоятся на экран.
  - Поддерживается Backspace.
  - Возвращает введённую строку в динамически выделенном буфере (CHAR16*).
  - Caller обязан освободить возвращённый буфер через FreePool.

  @param[out] OutBufferPtr   Указатель, в который будет записан адрес выделенного буфера с результатом.
                           Этот буфер содержит строку длиной до MAX_INPUT_LEN символов (без учёта завершающего L'\0').
  @retval EFI_SUCCESS        Успешно прочитана строка.
  @retval EFI_BUFFER_TOO_SMALL
                           Ввод превысил MAX_INPUT_LEN−1 символов.
  @retval EFI_OUT_OF_RESOURCES
                           Не удалось выделить память через AllocatePool.
  @retval любой другой код  Ошибка чтения/ожидания клавиши.
**/
EFI_STATUS
ReadLine(
    OUT CHAR16 **OutBufferPtr
    )
{
    EFI_STATUS     Status;
    EFI_INPUT_KEY  Key;
    CHAR16        *Buffer;
    UINTN          Index;
    UINTN          MaxLen = MAX_INPUT_LEN;
    UINTN          Pos = 0;

    if (OutBufferPtr == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    Buffer=malloc(sizeof(CHAR16)*MaxLen);

    // 2) Сразу же инициализируем весь буфер нулями (не обязательно, но удобно).
    HalZeroMem(Buffer, sizeof(CHAR16) * MaxLen);

    // 3) Цикл чтения символов до CR (Enter).
    while (TRUE) {
        // Ждём события «нажатие любой клавиши»
        Status = gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Index);
        if (EFI_ERROR(Status)) {
            // освободим буфер перед возвратом ошибки
            gBS->FreePool(Buffer);
            return Status;
        }

        // Считываем сам код клавиши
        Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
        if (EFI_ERROR(Status)) {
            gBS->FreePool(Buffer);
            return Status;
        }

        // 3.1) Если нажали Enter (Carriage Return = L'\r'), завершаем ввод
        if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            // Заканчиваем строку нулём
            Buffer[Pos] = L'\0';
            // Эхо «перенос строки» (чтобы курсор перешёл на новую строку)
            gST->ConOut->OutputString(gST->ConOut, L"\r\n");
            break;
        }

        // 3.2) Если Backspace
        else if (Key.UnicodeChar == CHAR_BACKSPACE) {
            if (Pos > 0) {
                // Удаляем последний символ из буфера
                Pos--;

                // Эхо «двигаться назад, пробел, двигаться назад»:
                //  – L"\x8" = переместить курсор влево
                //  – L" "   = стереть символ (заменить пробелом)
                //  – L"\x8" = снова переместить курсор влево
                gST->ConOut->OutputString(gST->ConOut, L"\x8 \x8");
            }
            // Если Pos==0, просто игнорируем дополнительные Backspace
        }

        // 3.3) Если обычный печатный символ (от U+0020 до U+FFFE), и места хватает
        else if (Key.UnicodeChar >= 0x0020 && Pos + 1 < MaxLen) {
            // Запоминаем символ в буфер
            Buffer[Pos++] = Key.UnicodeChar;

            // Эхо самого символа
            CHAR16 EchoChar[2] = { Key.UnicodeChar, 0 };
            gST->ConOut->OutputString(gST->ConOut, EchoChar);
        }

        // 3.4) В остальных случаях (Escape, функции F1–F12 и т. д.) — игнорируем.
        //       Если же пользователь ввёл слишком много символов (Pos + 1 >= MaxLen),
        //       можно сообщить об этом сообщением или просто отбрасывать дальше.
        else if (Pos + 1 >= MaxLen) {
            // Перебор буфера — можем вывести предупреждение и завершить ввод:
            gST->ConOut->OutputString(
                gST->ConOut,
                L"\r\n[ERROR] Input too long. Buffer overflow.\r\n"
            );
            gBS->FreePool(Buffer);
            return EFI_BUFFER_TOO_SMALL;
        }
    }

    // 4) Возвращаем введённую строку
    *OutBufferPtr = Buffer;
    return EFI_SUCCESS;
}

//HALSM

HalSMNull hnulld;
HalSMVariable nulld={&hnulld,HalSMVariableType_HalSMNull};
HalSMMemory* memory;

static int hsl_strlen(const char* s) {
    int i = 0;
    while (s[i]) ++i;
    return i;
}

//HalSM Memory Library

void* HML_malloc(unsigned long long int size) {
    return malloc(size);
}

void* HML_calloc(unsigned long long int n,unsigned long long int size) {
    return calloc(n,size);
}

void* HML_realloc(void* p,unsigned long long int nsize) {
    return realloc(p,nsize);
}

void HML_free(void* p) {free(p);}

//HalSM Memory Library

//HalSM System Library

void HSYSL_exit(int code){gBS->Exit(HLibImageHandle,0,0,NOTHALSMNULLPOINTER);}

//HalSM System Library

void HSM_print(HalStringFormatChar* str){ST->ConOut->OutputString(ST->ConOut,HU16(str));
ST->ConOut->OutputString(ST->ConOut,L"\r\n");}

void HSM_printError(HalStringFormatChar* str){ST->ConOut->OutputString(ST->ConOut,HU16(str));
ST->ConOut->OutputString(ST->ConOut,L"\r\n");}


HalStringFormatChar* HSM_input(HalStringFormatChar* str){
    CHAR16* input;
    ST->ConOut->OutputString(ST->ConOut,HU16(str));
    ReadLine(&input);
    return U16H((unsigned short*)input);
}

HalSMVariable* HSM_getVariable(HalStringFormatChar* name){return &nulld;}

//HalSM File System Library

void* HFSL_openFile(HalStringFormatChar* path,HalStringFormatChar* mode){return NULL;}

void HFSL_closeFile(void* p){}

unsigned long long int HFSL_read(void* out,unsigned long long int size,unsigned long long int count,void* p){return 0;}

unsigned long long int HFSL_write(void* in,unsigned long long int size,unsigned long long int count,void* p) {
    return 0;
}

void HFSL_puts(void* p,HalStringFormatChar* str){}

void HFSL_seek(void* p,unsigned long long int offset,unsigned long long int origin){}

unsigned long long int HFSL_tell(void* p){return 0;}

void HFSL_rewind(void* p){}

void HFSL_flush(void* p){}

int HFSL_eof(void* p){return -1;}

unsigned char HFSL_isExistsDir(HalStringFormatChar* path){
    return 0;
}

unsigned char HFSL_isExistsFile(HalStringFormatChar* path){
    return 0;
}

HalSMArray* HFSL_getListFiles(HalSMCompiler* hsmc,HalStringFormatChar* path){return HalSMArray_init(hsmc->memory);}

HalSMVariable* HSMLoadModule(HalSMCompiler* hsmc,HalStringFormatChar* name){
    if(HSFCompare(name,U8H("PrintModule"))) {
        return HalSMVariable_init(hsmc->memory,PrintModule_init(hsmc),HalSMVariableType_HalSMCModule);
    } else if(HSFCompare(name,U8H("SnakeGame"))) {
        return HalSMVariable_init(hsmc->memory,SnakeGame_init(hsmc),HalSMVariableType_HalSMCModule);
    } else if(HSFCompare(name,U8H("HalOS"))) {
        return HalSMVariable_init(hsmc->memory,HalOSModule_init(hsmc),HalSMVariableType_HalSMCModule);
    }
    return &nulld;
}

void* hstrf_malloc(ui64 s) {
    return HalSMMemory_malloc(memory,s);
}

//HalSM

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle,EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    HalInitializeLib(ImageHandle,SystemTable);

    Status = InitUefiTicker(1000ULL);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut,L"InitUefiTicker failed\n");
        return Status;
    }

    ST->ConOut->EnableCursor(ST->ConOut,TRUE);

    HalSMMemoryManagmentLibrary hmml={HML_malloc,HML_calloc,HML_realloc,HML_free};
    memory=(HalSMMemory*)malloc(sizeof(HalSMMemory));
    HalSMMemory_init(memory,&hmml);
    HalStringFormat_init(hstrf_malloc);
    HalSMSystemLibrary hsysl={HSYSL_exit};
    HalSMArray* modules=HalSMArray_init(memory);
    HalSMFileSystemLibrary hfsl={HFSL_isExistsDir,HFSL_isExistsFile,HFSL_getListFiles,HFSL_openFile,HFSL_closeFile,HFSL_read,HFSL_write,HFSL_puts,HFSL_seek,HFSL_tell,HFSL_rewind,HFSL_flush,HFSL_eof};
    HalSMCompiler* hsmc=HalSMCompiler_init(U8H(""),HSM_print,HSM_printError,HSM_input,HSM_getVariable,memory,&hsysl,&hfsl,HSMLoadModule);

    VOID      *FileData;
    UINTN      FileSize;
    Status = ReadFileToBuffer(L"\\main.hsm", &FileData, &FileSize);
    if (EFI_ERROR(Status)) {
        if (Status == EFI_NOT_FOUND) {
            gST->ConOut->OutputString(gST->ConOut, L"File not found: \\main.hsm\r\n");
        } else {
            gST->ConOut->OutputString(gST->ConOut, L"Error reading \\main.hsm\r\n");
        }
    } else {
        gST->ConOut->OutputString(gST->ConOut, L"Loaded main.hsm\r\n");

        HalSMCompiler_compile(hsmc,U8H((char*)FileData),0,&nulld);

        gBS->FreePool(FileData);
    }
    
    return EFI_SUCCESS;
}