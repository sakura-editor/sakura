@echo off
set CMD_7Z=
if "%FORCE_POWERSHELL_ZIP%" == "1" (
	exit /b 0
)

where 7z 1>nul 2>&1
if "%ERRORLEVEL%" == "0" (
	set CMD_7Z=7z
) else if exist "C:\Program Files\7-Zip\7z.exe" (
	set CMD_7Z=C:\Program Files\7-Zip\7z.exe
)
