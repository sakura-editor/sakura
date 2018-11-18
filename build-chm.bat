call %~dp0tools\hhc\find-hhc.bat
if "%CMD_HHC%" == "" (
	echo hhc.exe was not found.
	exit /b 1
)

set HHP_MACRO=help\macro\macro.HHP
set HHP_PLUGIN=help\plugin\plugin.hhp
set HHP_SAKURA=help\sakura\sakura.hhp

@rem hhc.exe returns 1 on success, and returns 0 on failure
"%CMD_HHC%" %HHP_MACRO%
if not errorlevel 1 (
	echo error %HHP_MACRO% errorlevel %errorlevel%
	"%CMD_HHC%" %HHP_MACRO%
)
if not errorlevel 1 (
	echo error %HHP_MACRO% errorlevel %errorlevel%
	exit /b 1
)

"%CMD_HHC%" %HHP_PLUGIN%
if not errorlevel 1 (
	echo error %HHP_PLUGIN% errorlevel %errorlevel%
	"%CMD_HHC%" %HHP_PLUGIN%
)
if not errorlevel 1 (
	echo error %HHP_PLUGIN% errorlevel %errorlevel%
	exit /b 1
)

"%CMD_HHC%" %HHP_SAKURA%
if not errorlevel 1 (
	echo error %HHP_SAKURA% errorlevel %errorlevel%
	"%CMD_HHC%" %HHP_SAKURA%
)
if not errorlevel 1 (
	echo error %HHP_SAKURA% errorlevel %errorlevel%
	exit /b 1
)
exit /b 0
