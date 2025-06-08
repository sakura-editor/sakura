call sakura\githash.bat "%~dp0sakura_core"
call tools\find-tools.bat

if exist html rmdir /s /q html

set PROJECT_NUMBER=%GIT_SHORT_COMMIT_HASH%
set HHC_LOCATION=%CMD_HHC%
set DOXYGEN_LOG=doxygen-%platform%-%configuration%.log

if "%CMD_DOXYGEN%" == "" (
	echo doxygen was not found
) else if exist "%CMD_DOXYGEN%" (
	"%CMD_DOXYGEN%" doxygen.conf > %DOXYGEN_LOG%
) else (
	echo doxygen was not found
)
