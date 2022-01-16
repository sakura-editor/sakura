call sakura\githash.bat "%~dp0sakura_core"
if not defined CMD_DOXYGEN call :doxygen

if exist html rmdir /s /q html

set PROJECT_NUMBER=%GIT_SHORT_COMMIT_HASH%
set HHC_LOCATION=%CMD_HHC%
set DOXYGEN_LOG=doxygen-%platform%-%configuration%.log

if not exist "%CMD_DOXYGEN%" (
	echo doxygen was not found
	exit /b
)
"%CMD_DOXYGEN%" doxygen.conf > %DOXYGEN_LOG%
exit /b

:doxygen
set APPDIR=doxygen\bin
set PATH2=%PATH%;%ProgramFiles%\%APPDIR%\;%ProgramFiles(x86)%\%APPDIR%\;%ProgramW6432%\%APPDIR%\;
for /f "usebackq delims=" %%a in (`where $PATH2:doxygen.exe`) do ( 
    set "CMD_DOXYGEN=%%a"
    exit /b
)
exit /b
