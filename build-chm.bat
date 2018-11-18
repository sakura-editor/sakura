call %~dp0tools\hhc\find-hhc.bat
if "%CMD_HHC%" == "" (
	echo hhc.exe was not found.
	exit /b 1
)

@rem hhc.exe returns 1 on success, and returns 0 on failure
"%CMD_HHC%" help\macro\macro.HHP
if not errorlevel 1 (
	echo error macro.HHP errorlevel %errorlevel%
	"%CMD_HHC%" help\macro\macro.HHP
)
if not errorlevel 1 (
	echo error macro.HHP errorlevel %errorlevel%
	exit /b 1
)

"%CMD_HHC%" help\plugin\plugin.hhp
if not errorlevel 1 (
	echo error plugin.hhp errorlevel %errorlevel%
	"%CMD_HHC%" help\macro\plugin.hhp
)
if not errorlevel 1 (
	echo error plugin.hhp errorlevel %errorlevel%
	exit /b 1
)

"%CMD_HHC%" help\sakura\sakura.hhp
if not errorlevel 1 (
	echo error sakura.hhp errorlevel %errorlevel%
	"%CMD_HHC%" help\macro\sakura.hhp
)
if not errorlevel 1 (
	echo error sakura.hhp errorlevel %errorlevel%
	exit /b 1
)
exit /b 0
