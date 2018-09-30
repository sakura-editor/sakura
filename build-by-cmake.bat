set BUILDDIR=build

cd /d %~dp0
@echo off
for /l %%n in (1,1,10) do (
	if exist "%BUILDDIR%" (
		rmdir /s /q "%BUILDDIR%"
	)
)
@echo on
mkdir "%BUILDDIR%" || exit /b 1
cd    "%BUILDDIR%"

set RETCODE=0
if %ERRORLEVEL% equ 0 (
	cmake ..
) else (
	set RETCODE=1
)

if %ERRORLEVEL% equ 0 (
	cmake --build . --config Debug
) else (
	set RETCODE=1
)

if %ERRORLEVEL% equ 0 (
	cmake --build . --config Release
) else (
	set RETCODE=1
)
cd ..

if %ERRORLEVEL% equ 0 (
	echo success
	set RETCODE=0
) else (
	set RETCODE=1
)

exit /b %RETCODE%
