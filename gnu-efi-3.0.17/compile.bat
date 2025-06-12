@echo ofn
setlocal EnableDelayedExpansion
for /F %%I in ('forfiles /p lib /m *.c /c "cmd /c echo @relpath"') do (
	set "TMP_PATH=%%I"
	set "BUILD_PATH=!TMP_PATH:.\=x86_64\lib\!"
	..\mingw64\bin\gcc -m64 -c -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc !TMP_PATH:.\=lib\! -o !BUILD_PATH:.c=.o!
)

for /F %%I in ('forfiles /p lib\x86_64 /m *.c /c "cmd /c echo @relpath"') do (
	set "TMP_PATH=%%I"
	set "BUILD_PATH=!TMP_PATH:.\=x86_64\lib\x86_64\!"
	..\mingw64\bin\gcc -m64 -c -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc !TMP_PATH:.\=lib\x86_64\! -o !BUILD_PATH:.c=.o!
)

for /F %%I in ('forfiles /p lib /m *.S /c "cmd /c echo @relpath"') do (
	set "TMP_PATH=%%I"
	set "BUILD_PATH=!TMP_PATH:.\=x86_64\lib\!"
	..\mingw64\bin\gcc -m64 -c -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc !TMP_PATH:.\=lib\! -o !BUILD_PATH:.S=.o!
)