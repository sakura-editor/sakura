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

set DEST_DIR=%~dp0..\%PLATFORM%\%CONFIGURATION%
set TEMP_DIR=%~dp0..\installer\temp

: expands every zip
for %%i in (%~dp0\..\installer\externals\*.zip) do (
	REM call %UNZIP_CMD% %%i %TEMP_DIR%
	call %UNZIP_CMD% %%i %TEMP_DIR%\%%~ni\
)

set BRON=bron412
set CTAGS32=ctags-2018-09-16_e522743d-x86
set CTAGS32=ctags-2018-09-16_e522743d-x64

if "%PLATFORM%" == "Win32" (
	: for Win32
	xcopy /Y /I %TEMP_DIR%\%BRON%\bregonig.dll %DEST_DIR%\
	xcopy /Y /I %TEMP_DIR%\%BRON%\*.txt %DEST_DIR%\license\bregonig\

	xcopy /Y /I %TEMP_DIR%\%CTAGS32%\ctags.exe %DEST_DIR%\
	xcopy /Y /I %TEMP_DIR%\%CTAGS32%\license %DEST_DIR%\license\ctags\
) else (
	: for x64
	xcopy /Y /I %TEMP_DIR%\%BRON%\bregonig.dll %DEST_DIR%\
	xcopy /Y /I %TEMP_DIR%\%BRON%\*.txt %DEST_DIR%\license\bregonig\

	xcopy /Y /I %TEMP_DIR%\%CTAGS64%\ctags.exe %DEST_DIR%\
	xcopy /Y /I %TEMP_DIR%\%CTAGS64%\license %DEST_DIR%\license\ctags\
)
 

REM forfiles /P %~dp0..\installer\externals\ /M *.zip /c "cmd /c %UNZIP_CMD% @file %TEMP_DIR%"

REM : ---- bron412 ---- :
REM set INSTALLER_RESOURCES_BRON=%~dp0..\installer\temp\bron
REM set BRON_ZIP=%~dp0..\installer\externals\bregonig\bron412.zip
REM set DLL_BREGONIG_NAME=bregonig.dll
REM if "%platform%" == "x64" (
REM 	set INSTALLER_RESOURCES_BRON_DLL=%INSTALLER_RESOURCES_BRON%\x64
REM ) else (
REM 	set INSTALLER_RESOURCES_BRON_DLL=%INSTALLER_RESOURCES_BRON%
REM )

REM if not exist "%INSTALLER_RESOURCES_BRON_DLL%\%DLL_BREGONIG_NAME%" (
REM 	@echo extract %BRON_ZIP%
REM 	call %UNZIP_CMD% %BRON_ZIP% %INSTALLER_RESOURCES_BRON% || (echo error && exit /b 1)
REM )
REM if not exist "%DEST_DIR%\%DLL_BREGONIG_NAME%" (
REM 	@echo %DLL_BREGONIG_NAME% to destination directory.
REM 	copy /Y /B %INSTALLER_RESOURCES_BRON_DLL%\%DLL_BREGONIG_NAME%  %DEST_DIR%\
REM )

REM : ---- ctags.exe ---- :
REM set INSTALLER_RESOURCES_CTAGS=%~dp0..\installer\temp\ctags
REM set INSTALLER_RESOURCES_CTAGS_EXE=%INSTALLER_RESOURCES_CTAGS%
REM if "%PLATFORM%" == "Win32" (
REM 	set CTAGS_PREFIX=x86
REM ) else if "%PLATFORM%" == "x64" (
REM 	set CTAGS_PREFIX=x64
REM ) else (
REM 	@echo unknown PLATFORM %PLATFORM%
REM 	exit /b 1
REM )
REM set CTAGS_ZIP=%~dp0..\installer\externals\universal-ctags\ctags-2018-09-16_e522743d-%CTAGS_PREFIX%.zip
REM set EXE_CTAGS_NAME=ctags.exe

REM if not exist "%INSTALLER_RESOURCES_CTAGS_DLL%\%EXE_CTAGS_NAME%" (
REM 	@echo extract %CTAGS_ZIP%
REM 	call %UNZIP_CMD% %CTAGS_ZIP% %INSTALLER_RESOURCES_CTAGS% || (echo error && exit /b 1)
REM )
REM if not exist "%DEST_DIR%\%EXE_CTAGS_NAME%" (
REM 	@echo %EXE_CTAGS_NAME% to destination directory.
REM 	copy /Y /B %INSTALLER_RESOURCES_CTAGS_EXE%\%EXE_CTAGS_NAME%  %DEST_DIR%\
REM )

ENDLOCAL
