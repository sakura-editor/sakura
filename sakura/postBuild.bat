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

: ---- bron412 ---- :
set BREGONIG_DLL=bregonig.dll
set BRON_ZIP=..\installer\externals\bregonig\bron412.zip
set BRON_TMP=..\installer\temp\bron
if "%platform%" == "x64" (
	set BRON_DIR=%~dp0%BRON_TMP%\x64
) else (
	set BRON_DIR=%~dp0%BRON_TMP%
)
if not exist "%OUT_DIR%\%BREGONIG_DLL%" (
	@echo %BRON_ZIP% -^> %DEST_DIR%\%BREGONIG_DLL%
	if not exist "%BRON_DIR%\%BREGONIG_DLL%" (
		call "%UNZIP_CMD%" "%~dp0%BRON_ZIP%" "%~dp0%BRON_TMP%" > NUL || (echo error && exit /b 1)
	)
	copy /Y /B "%BRON_DIR%\%BREGONIG_DLL%" "%OUT_DIR%\" > NUL
)

: ---- ctags.exe ---- :
set CTAGS_EXE=ctags.exe
if "%PLATFORM%" == "Win32" (
	set CTAGS_PREFIX=x86
) else if "%PLATFORM%" == "x64" (
	set CTAGS_PREFIX=x64
) else (
	echo unknown PLATFORM %PLATFORM%
	exit /b 1
)
set CTAGS_ZIP=..\installer\externals\universal-ctags\ctags-2018-09-16_e522743d-%CTAGS_PREFIX%.zip
set CTAGS_TMP=..\installer\temp\ctags
set CTAGS_DIR=%~dp0%CTAGS_TMP%
if not exist "%OUT_DIR%\%CTAGS_EXE%" (
	@echo %CTAGS_ZIP% -^> %DEST_DIR%\%CTAGS_EXE%
	if not exist "%CTAGS_DIR%\%CTAGS_EXE%" (
		call "%UNZIP_CMD%" "%~dp0%CTAGS_ZIP%" "%CTAGS_DIR%" > NUL || (echo error && exit /b 1)
	)
	copy /Y /B "%CTAGS_DIR%\%CTAGS_EXE%" "%OUT_DIR%\" > NUL
)

ENDLOCAL
