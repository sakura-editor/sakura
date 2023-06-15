@echo off
setlocal

pushd "%~dp0vcpkg"

IF not EXIST vcpkg.exe (
	CALL bootstrap-vcpkg.bat
)

: For tests1
vcpkg install ^
    gtest:x64-windows-static ^
    gtest:x86-windows-static

vcpkg export ^
    gtest:x64-windows-static ^
    gtest:x86-windows-static ^
	--nuget --nuget-id=tests1.dependencies --nuget-version=1.0.0 --output-dir="%~dp0..\.nuget"

if not defined CMD_NUGET call :nuget
if defined CMD_NUGET (
    "%CMD_NUGET%" install tests1.dependencies -Source "%~dp0..\.nuget" -OutputDirectory "%~dp0..\packages"
)

: For sakura
mkdir ports\header-make
copy /Y ..\HeaderMake\portfile.cmake ports\header-make\
copy /Y ..\HeaderMake\vcpkg.json     ports\header-make\

vcpkg install ^
    header-make:x64-windows ^
    header-make:x86-windows

vcpkg export ^
    header-make:x64-windows ^
    header-make:x86-windows ^
	--nuget --nuget-id=sakura.dependencies --nuget-version=1.0.0 --output-dir="%~dp0..\.nuget"

"%CMD_NUGET%" install sakura.dependencies -Source "%~dp0..\.nuget" -OutputDirectory "%~dp0..\packages"

popd

exit /b

:nuget
for /f "usebackq delims=" %%a in (`dir nuget.exe /b /s`) do ( 
    set "CMD_NUGET=%%a"
    exit /b
)
for /f "usebackq delims=" %%a in (`where nuget.exe`) do ( 
    set "CMD_NUGET=%%a"
    exit /b
)
exit /b
