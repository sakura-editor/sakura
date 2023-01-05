if not defined CMD_HHC call %~dp0tools\find-tools.bat
if not defined CMD_HHC (
	echo hhc.exe was not found.
	exit /b 1
)

set SRC_HELP=%~dp0help

@rem create sakura.hh before copying because sakura.hh will be uploaded as an artifact.
set HH_INPUT=%~dp0sakura_core\sakura.hh
set HH_OUTPUT=%~dp0help\sakura\sakura.hh

if exist "%HH_OUTPUT%" del /F "%HH_OUTPUT%"
powershell "(Get-Content -li %HH_INPUT% -Encoding UTF8) -replace '//.*' | Set-Content -li %HH_OUTPUT% -Encoding UTF8"

set HHP_MACRO=%SRC_HELP%\macro\macro.HHP
set HHP_PLUGIN=%SRC_HELP%\plugin\plugin.hhp
set HHP_SAKURA=%SRC_HELP%\sakura\sakura.hhp

call :BuildChm %HHP_MACRO%   || (echo error && exit /b 1)
call :BuildChm %HHP_PLUGIN%  || (echo error && exit /b 1)
call :BuildChm %HHP_SAKURA%  || (echo error && exit /b 1)

exit /b 0

@rem ------------------------------------------------------------------------------
@rem BuildChm
@rem ------------------------------------------------------------------------------
:BuildChm
powershell.exe -ExecutionPolicy RemoteSigned -File %~dp0help\CompileChm.ps1 %1 -CMD_HHC "%CMD_HHC%"
exit /b 0
