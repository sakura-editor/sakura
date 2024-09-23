@echo off
setlocal

if not defined CMD_GIT call "%~dp0find-tools.bat"
if not defined CMD_GIT (
    @echo NOTE: No git command
    exit /b 1
)

IF not exist "%~dp0vcpkg\bootstrap-vcpkg.bat" (
	git submodule update --init
)

IF not exist "%~dp0vcpkg\vcpkg.exe" (
    chcp 65001 >NUL 
	CALL "%~dp0vcpkg\bootstrap-vcpkg.bat"
)

: install command
"%~dp0vcpkg\vcpkg.exe" install "--x-manifest-root=%~dp0.." "--x-install-root=%~dp0..\build\vcpkg_installed"  --triplet x64-windows-static --x-wait-for-lock

mkdir "%~dp0..\build\TrayClickEmulator"
pushd "%~dp0..\build\TrayClickEmulator"
"%CMD_CMAKE%" -G "Visual Studio 17 2022" -A x64 -DCMAKE_CONFIGURATION_TYPES:STRING="Release" -DCMAKE_INSTALL_PREFIX:PATH="%~dp0..\build\vcpkg_installed\x64-windows-static" -DCMAKE_TOOLCHAIN_FILE:FILEPATH="%~dp0vcpkg\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET:STRING="x64-windows-static-md" "%~dp0TrayClickEmulator"
"%CMD_CMAKE%" --build . --config Release --target install
popd
