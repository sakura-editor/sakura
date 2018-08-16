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

set INSTALLER_RESOURCES_BRON=%~dp0..\installer\temp\bron
set BRON_ZIP=%~dp0..\installer\externals\bregonig\bron412.zip

if exist "%INSTALLER_RESOURCES_BRON%" rmdir /s /q "%INSTALLER_RESOURCES_BRON%"
7z x %BRON_ZIP%  -O"%INSTALLER_RESOURCES_BRON%" || (echo error && exit /b 1)

set DLL_BREGONIG_NAME=bregonig.dll
if "%platform%" == "x64" (
	set INSTALLER_RESOURCES_BRON_DLL=%INSTALLER_RESOURCES_BRON%\x64
) else (
	set INSTALLER_RESOURCES_BRON_DLL=%INSTALLER_RESOURCES_BRON%
)

: ---- copy bregonig.dll ---- :
@echo Copy bregonig.dll to destination directory.
copy /Y /B %INSTALLER_RESOURCES_BRON_DLL%\%DLL_BREGONIG_NAME%  ..\%PLATFORM%\%CONFIGURATION%\

ENDLOCAL
