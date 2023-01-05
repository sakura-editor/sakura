if not defined CMD_HHC call %~dp0tools\find-tools.bat
if not defined CMD_HHC (
	echo hhc.exe was not found.
	exit /b 1
)
set SRC_HELP=%~dp0help
set TMP_HELP=%~dp0temphelp

@rem create sakura.hh before copying because sakura.hh will be uploaded as an artifact.
set HH_INPUT=%~dp0sakura_core\sakura.hh
set HH_OUTPUT=%~dp0help\sakura\sakura.hh

if exist "%HH_OUTPUT%" del /F "%HH_OUTPUT%"
if exist "%TMP_HELP%" rmdir /s /q "%TMP_HELP%"

powershell.exe -ExecutionPolicy RemoteSigned -File "%~dp0help\build-chm.ps1" ^
-SRC_HELP "%SRC_HELP%" -TMP_HELP "%TMP_HELP%" -HH_INPUT "%HH_INPUT%" -HH_OUTPUT "%HH_OUTPUT%" -CMD_HHC "%CMD_HHC%" -CMD_LEPROC "%CMD_LEPROC%"

rmdir /s /q "%TMP_HELP%"
exit /b 0

