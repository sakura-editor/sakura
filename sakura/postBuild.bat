@echo off

SETLOCAL

set UNZIP_CMD=%~dp0..\tools\zip\unzip.bat

@echo =======================
@echo postBuild
@echo =======================

: ---- arguments ---- :
: "Win32" or "x64"
set PLATFORM=%1
@echo PLATFORM=%PLATFORM%

: "Debug" or "Release"
set CONFIGURATION=%2
@echo CONFIGURATION=%CONFIGURATION%

set DEST_DIR=%PLATFORM%\%CONFIGURATION%
set OUT_DIR=%~dp0..\%DEST_DIR%

ENDLOCAL
