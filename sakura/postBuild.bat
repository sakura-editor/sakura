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

set DEST_DIR=..\%PLATFORM%\%CONFIGURATION%

: ---- bron412 ---- :
set INSTALLER_RESOURCES_BRON=%~dp0..\installer\temp\bron
set BRON_ZIP=%~dp0..\installer\externals\bregonig\bron412.zip
set DLL_BREGONIG_NAME=bregonig.dll
if "%platform%" == "x64" (
	set INSTALLER_RESOURCES_BRON_DLL=%INSTALLER_RESOURCES_BRON%\x64
) else (
	set INSTALLER_RESOURCES_BRON_DLL=%INSTALLER_RESOURCES_BRON%
)

if not exist "%INSTALLER_RESOURCES_BRON_DLL%\%DLL_BREGONIG_NAME%" (
	@echo extract %BRON_ZIP%
	call %UNZIP_CMD% %BRON_ZIP% %INSTALLER_RESOURCES_BRON% || (echo error && exit /b 1)
)
if not exist "%DEST_DIR%\%DLL_BREGONIG_NAME%" (
	@echo %DLL_BREGONIG_NAME% to destination directory.
	copy /Y /B %INSTALLER_RESOURCES_BRON_DLL%\%DLL_BREGONIG_NAME%  %DEST_DIR%\
)

: ---- ctags.exe ---- :
set INSTALLER_RESOURCES_CTAGS=%~dp0..\installer\temp\ctags
set INSTALLER_RESOURCES_CTAGS_EXE=%INSTALLER_RESOURCES_CTAGS%
if "%PLATFORM%" == "Win32" (
	set CTAGS_PREFIX=x86
) else if "%PLATFORM%" == "x64" (
	set CTAGS_PREFIX=x64
) else (
	@echo unknown PLATFORM %PLATFORM%
	exit /b 1
)
set CTAGS_ZIP=%~dp0..\installer\externals\universal-ctags\ctags-2018-09-16_e522743d-%CTAGS_PREFIX%.zip
set EXE_CTAGS_NAME=ctags.exe

if not exist "%INSTALLER_RESOURCES_CTAGS_DLL%\%EXE_CTAGS_NAME%" (
	@echo extract %CTAGS_ZIP%
	call %UNZIP_CMD% %CTAGS_ZIP% %INSTALLER_RESOURCES_CTAGS% || (echo error && exit /b 1)
)
if not exist "%DEST_DIR%\%EXE_CTAGS_NAME%" (
	@echo %EXE_CTAGS_NAME% to destination directory.
	copy /Y /B %INSTALLER_RESOURCES_CTAGS_EXE%\%EXE_CTAGS_NAME%  %DEST_DIR%\
)

ENDLOCAL
