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
set BRON_ARCHIVE=%~dp0..\installer\externals\bregonig\bron412.zip
set BRON_EXTRACTED_DEST=%~dp0..\installer\temp\bron
set BRON_BINRY=bregonig.dll
if "%platform%" == "x64" (
    set BRON_EXTRACTED_BINARY=%BRON_EXTRACTED_DEST%\x64%BRON_BINARY%
) else (
    set BRON_EXTRACTED_BINARY=%BRON_EXTRACTED_DEST%\%BRON_BINARY%
)

if not exist "%BRON_EXTRACTED_BINARY%" (
    @echo extract %BRON_ARCHIVE%
    call %UNZIP_CMD% %BRON_ARCHIVE% %BRON_EXTRACTED_DEST% || (echo error && exit /b 1) 
)
copy /Y /B %BRON_EXTRACTED_BINARY% %DEST_DIR%\

: ---- ctags.exe ---- :
if "%platform%" == "x64" (
    set CTAGS_ARCHIVE=%~dp0..\installer\externals\universal-ctags\ctags-2018-09-16_e522743d-x64.zip
) else (
    set CTAGS_ARCHIVE=%~dp0..\installer\externals\universal-ctags\ctags-2018-09-16_e522743d-x86.zip
)
set CTAGS_EXTRACTED_DEST=%~dp0..\installer\temp\ctags
set CTAGS_BINRY=ctags.exe
set CTAGS_EXTRACTED_BINARY=%CTAGS_EXTRACTED_DEST%\%CTAGS_BINARY%

if not exist "%CTAGS_EXTRACTED_BINARY%" (
    @echo extract %CTAGS_ARCHIVE%
    call %UNZIP_CMD% %CTAGS_ARCHIVE% %CTAGS_EXTRACTED_DEST% || (echo error && exit /b 1) 
)
copy /Y /B %CTAGS_EXTRACTED_BINARY% %DEST_DIR%\

: ---- diff.exe ---- :
set DIFF_ARCHIVE=%~dp0..\installer\externals\patch-diff-w32\patch-diff-w32.zip
set DIFF_EXTRACTED_DEST=%~dp0..\installer\temp\diff
set DIFF_BINARY=diff.exe
set DIFF_EXTRACTED_BINARY=%DIFF_EXTRACTED_DEST%\%DIFF_BINARY%

if not exist "%DIFF_EXTRACTED_BINARY%" (
@echo extract %DIFF_ARCHIVE%
call %UNZIP_CMD% %DIFF_ARCHIVE% %DIFF_EXTRACTED_DEST% || (echo error && exit /b 1)
)
copy /Y /B %DIFF_EXTRACTED_BINARY% %DEST_DIR%\

: ---- migemo ---- :
@echo on
set MIGEMO_BINARY=migemo.dll
set MIGEMO_EXTRACTED_DEST=%~dp0..\installer\temp\migemo
if "%platform%" == "x64" (
set MIGEMO_ARCHIVE=%~dp0..\installer\externals\migemo\cmigemo-default-win64-20110227.zip
set MIGEMO_EXTRACTED_FILES_ROOT=%MIGEMO_EXTRACTED_DEST%_temp\cmigemo-default-win64
) else (
set MIGEMO_ARCHIVE=%~dp0..\installer\externals\migemo\cmigemo-default-win32-20110227.zip
set MIGEMO_EXTRACTED_FILES_ROOT=%MIGEMO_EXTRACTED_DEST%_temp\cmigemo-default-win32
)
set MIGEMO_EXTRACTED_BINARY=%MIGEMO_EXTRACTED_DEST%\%MIGEMO_BINARY%

if not exist "%MIGEMO_EXTRACTED_BINARY%" (
@echo extract %MIGEMO_ARCHIVE%
call %UNZIP_CMD% %MIGEMO_ARCHIVE% %MIGEMO_EXTRACTED_DEST%_temp || (echo error && exit /b 1)
MOVE %MIGEMO_EXTRACTED_FILES_ROOT% %MIGEMO_EXTRACTED_DEST%
)
echo %MIGEMO_EXTRACTED_BINARY% %MIGEMO_EXTRACTED_DEST%
copy /Y /B %MIGEMO_EXTRACTED_BINARY% %DEST_DIR%\
xcopy %MIGEMO_EXTRACTED_DEST%\dict %DEST_DIR%\dict /S/E/I/Y
ENDLOCAL
