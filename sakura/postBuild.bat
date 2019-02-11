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
set CTAGS64=ctags-2018-09-16_e522743d-x64

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
 
ENDLOCAL
