call %~dp0tools\hhc\find-hhc.bat
if "%CMD_HHC%" == "" (
	echo hhc.exe was not found.
	exit /b 1
)

@rem hhc.exe returns 1 on success, and returns 0 on failure
"%CMD_HHC%" help\macro\macro.HHP
if %errorlevel% equ 0 (echo error && exit /b 1)

"%CMD_HHC%" help\plugin\plugin.hhp
if %errorlevel% equ 0 (echo error && exit /b 1)

"%CMD_HHC%" help\sakura\sakura.hhp
if %errorlevel% equ 0 (echo error && exit /b 1)

exit /b 0
