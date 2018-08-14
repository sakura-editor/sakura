@echo off

SETLOCAL

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

: ---- copy bregonig.dll ---- :
@echo Copy bregonig.dll to destination directory.
copy /Y /B ..\installer\externals\bregonig\%PLATFORM%\bregonig.dll ..\%PLATFORM%\%CONFIGURATION%\bregonig.dll


ENDLOCAL
