set HHC=C:\Program Files (x86)\HTML Help Workshop\hhc.exe

@rem hhc.exe returns 1 on success, and returns 0 on failure
"%HHC%" help\macro\macro.HHP
if %errorlevel% equ 0 (echo error && exit /b 1)

"%HHC%" help\plugin\plugin.hhp
if %errorlevel% equ 0 (echo error && exit /b 1)

"%HHC%" help\sakura\sakura.hhp
if %errorlevel% equ 0 (echo error && exit /b 1)

exit /b 0
