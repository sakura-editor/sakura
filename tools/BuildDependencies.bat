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
: "%~dp0vcpkg\vcpkg.exe" install --x-wait-for-lock --triplet "x64-windows-static" "--x-manifest-root=%~dp0.." "--x-install-root=%~dp0..\build\vcpkg_installed"
