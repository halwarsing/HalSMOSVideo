@echo on
setlocal EnableDelayedExpansion
for /F %%I in ('forfiles /p src /s /m *.c /c "cmd /c echo @relpath"') do (
	set "TMP_PATH=%%I"
	set "BUILD_PATH=!TMP_PATH:.\=build\!"
rem mingw64\bin\gcc -m64 -c -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinclude -c !TMP_PATH:.\=src\! -o !BUILD_PATH:.c=.o!
    mingw64\bin\gcc -m64 -c -ffreestanding -Ignu-efi-3.0.17/inc -Ignu-efi-3.0.17/inc/x86_64 -Ignu-efi-3.0.17/inc/protocol -O2 -Iinclude !TMP_PATH:.\=src\! -o !BUILD_PATH:.c=.o!
rem mingw64\bin\gcc -m64 -DEFI_FUNCTION_WRAPPER -fpic -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -Wextra -O2 -c -ffreestanding -Ignu-efi-3.0.17/inc -Ignu-efi-3.0.17/inc/x86_64 -Ignu-efi-3.0.17/inc/protocol -Iinclude !TMP_PATH:.\=src\! -o !BUILD_PATH:.c=.o!
)

mingw64\bin\gcc -m64 -c -ffreestanding -Ignu-efi-3.0.17/inc -Ignu-efi-3.0.17/inc/x86_64 -Ignu-efi-3.0.17/inc/protocol -O2 -Iinclude gnu-efi-3.0.17/lib/data.c -o build/data.o 

for /F %%I in ('forfiles /p src /s /m *.asm /c "cmd /c echo @relpath"') do (
	set "TMP_PATH=%%I"
	set "BUILD_PATH=!TMP_PATH:.\=build\!"
	mingw64\bin\nasm -fwin64 !TMP_PATH:.\=src\! -o !BUILD_PATH:.asm=.o!
)
::dir /S /A-D /B build\*.o > files.txt
:: set /p listFiles=%< files.txt:\n= %
:: del files.txt
for /f %%A in ('dir /S /A-D /B build\*.o') do set listFiles=!listFiles! %%A
mingw64\bin\gcc -Lgnu-efi-3.0.17\x86_64\gnuefi -Lgnu-efi-3.0.17\x86_64\lib -lefi -lgnuefi -m64 -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main !listFiles! -Iinclude -Ignu-efi-3.0.17/inc -Ignu-efi-3.0.17/inc/x86_64 -Ignu-efi-3.0.17/inc/protocol -o build/BOOTX64.efi

:: bash -c "dd if=/dev/zero of=HalOS.img bs=1k count=1440 && mformat -i HalOS.img -f 1440 :: && mmd -i HalOS.img ::/EFI && mmd -i HalOS.img ::/EFI/BOOT && mcopy -i HalOS.img build/BOOTX64.efi ::/EFI/BOOT && cp HalOS.img iso && xorriso -as mkisofs -R -f -e HalOS.img -no-emul-boot -o HalOS.iso iso"

@echo off
REM ------------------------------------------------
REM  build_iso.bat — Windows batch script (ASCII, English)
REM  
REM  - Uses MTools from ".\MTools\" (mformat.exe, mmd.exe, mcopy.exe)
REM  - Uses oscdimg from ".\oscdimg\oscdimg.exe"
REM  Creates a FAT12 image, copies files, and builds a bootable ISO.
REM ------------------------------------------------

REM 1) Name of the FAT image file
set "IMG_NAME=HalOS.img"

REM 2) Floppy size in kilobytes (1440 KB → standard 1.44 MB floppy)
set /a "IMG_SIZE_KB=1440"

REM 3) Directory where we will place HalOS.img before ISO creation
set "ISO_DIR=iso"

REM 4) Relative paths to external tools
set "MFORMAT_EXE=MTools\mformat.exe"
set "MMD_EXE=MTools\mmd.exe"
set "MCOPY_EXE=MTools\mcopy.exe"
set "OSCDIMG_EXE=oscdimg\oscdimg.exe"

REM Verify that MTools executables exist
if not exist "%MFORMAT_EXE%" (
    echo ERROR: Cannot find %MFORMAT_EXE%. Ensure MTools folder and mformat.exe are present.
    exit /b 1
)
if not exist "%MMD_EXE%" (
    echo ERROR: Cannot find %MMD_EXE%. Ensure MTools folder and mmd.exe are present.
    exit /b 1
)
if not exist "%MCOPY_EXE%" (
    echo ERROR: Cannot find %MCOPY_EXE%. Ensure MTools folder and mcopy.exe are present.
    exit /b 1
)

REM Verify that oscdimg exists
if not exist "%OSCDIMG_EXE%" (
    echo ERROR: Cannot find %OSCDIMG_EXE%. Ensure oscdimg folder and oscdimg.exe are present.
    exit /b 1
)

echo ==================================================
echo Step 1: Creating empty image file (%IMG_SIZE_KB% KiB)
echo ==================================================

if exist "%IMG_NAME%" (
    echo Deleting existing file %IMG_NAME%...
    del /Q "%IMG_NAME%"
)

REM Calculate image size in bytes
set /a "IMG_SIZE_BYTES=IMG_SIZE_KB * 1024"
fsutil file createnew "%IMG_NAME%" %IMG_SIZE_BYTES% 1>nul 2>&1
if errorlevel 1 (
    echo ERROR: Failed to create file %IMG_NAME% of size %IMG_SIZE_BYTES% bytes.
    exit /b 1
)

echo.

echo ==================================================
echo Step 2: Formatting %IMG_NAME% as FAT12 via MTools
echo ==================================================
"%MFORMAT_EXE%" -i "%IMG_NAME%" -f 1440 :: 1>nul 2>&1
if errorlevel 1 (
    echo ERROR: mformat failed to format %IMG_NAME% as FAT12.
    exit /b 1
)

echo.

echo ==================================================
echo Step 3: Creating EFI and EFI/BOOT directories inside the image
echo ==================================================
REM Use forward slashes for MTools directory paths
"%MMD_EXE%" -i "%IMG_NAME%" ::EFI 1>nul 2>&1
"%MMD_EXE%" -i "%IMG_NAME%" ::EFI/BOOT 1>nul 2>&1
if errorlevel 1 (
    echo ERROR: mmd failed to create directories EFI or EFI/BOOT inside %IMG_NAME%.
    exit /b 1
)

echo.

echo ==================================================
echo Step 4: Copying BOOTX64.EFI into EFI/BOOT/BOOTX64.EFI
echo ==================================================
"%MCOPY_EXE%" -i "%IMG_NAME%" "build\BOOTX64.EFI" ::EFI/BOOT/BOOTX64.EFI 1>nul 2>&1
if errorlevel 1 (
    echo ERROR: mcopy failed to copy BOOTX64.EFI into EFI/BOOT.
    exit /b 1
)

echo.

echo ==================================================
echo Step 5: Copying user files into the root of the image
echo ==================================================
"%MCOPY_EXE%" -i "%IMG_NAME%" "build\main.hsm"   ::main.hsm    1>nul 2>&1

if errorlevel 1 (
    echo ERROR: mcopy failed to copy one or more root files.
    exit /b 1
)

echo.

echo ==================================================
echo Step 6: Creating subfolders and copying additional files
echo ==================================================
:: "%MMD_EXE%" -i "%IMG_NAME%" ::scripts         1>nul 2>&1
:: "%MCOPY_EXE%" -i "%IMG_NAME%" "build\script.hsm" ::scripts/script.hsm  1>nul 2>&1

:: "%MMD_EXE%" -i "%IMG_NAME%" ::images          1>nul 2>&1
:: "%MCOPY_EXE%" -i "%IMG_NAME%" "build\logo.bmp" ::images/logo.bmp      1>nul 2>&1

echo.

echo ==================================================
echo Step 7: Preparing %ISO_DIR% folder
echo ==================================================
if exist "%ISO_DIR%" (
    echo Deleting existing folder %ISO_DIR%...
    rmdir /S /Q "%ISO_DIR%"
)
mkdir "%ISO_DIR%"

echo.

echo ==================================================
echo Step 8: Copying %IMG_NAME% into %ISO_DIR%
echo ==================================================
copy /Y "%IMG_NAME%" "%ISO_DIR%\" 1>nul 2>&1
if errorlevel 1 (
    echo ERROR: Failed to copy %IMG_NAME% into %ISO_DIR%.
    exit /b 1
)

echo.

echo ==================================================
echo Step 9: Creating bootable HalOS.iso with oscdimg
echo ==================================================
"%OSCDIMG_EXE%" -n -m -b"%ISO_DIR%\%IMG_NAME%" "%ISO_DIR%" "HalOS.iso" 1>nul 2>&1
if errorlevel 1 (
    echo ERROR: oscdimg failed to create HalOS.iso. ErrorLevel=%ERRORLEVEL%
    exit /b 1
)

echo.
echo ==================================================
echo Done! You now have: HalOS.iso
echo ==================================================
echo.

pause
exit /b 0
