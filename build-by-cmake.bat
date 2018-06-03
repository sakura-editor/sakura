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

if %ERRORLEVEL% equ 0 (
	cmake ..
)
if %ERRORLEVEL% equ 0 (
	cmake --build .
)
cd ..
