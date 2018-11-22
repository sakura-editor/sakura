call sakura\githash.bat
call tools\hhc\find-hhc.bat
call tools\doxygen\find-doxygen.bat

if exist html rmdir /s /q html

set PROJECT_NUMBER=%GIT_SHORT_COMMIT_HASH%
set HHC_LOCATION=%CMD_HHC%

if "%CMD_DOXYGEN%" == "" (
	echo doxygen was not found
) else if exist "%CMD_DOXYGEN%" (
	"%CMD_DOXYGEN%" doxygen.conf
) else (
	echo doxygen was not found
)
