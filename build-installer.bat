@echo off
set platform=%1
set configuration=%2

if "%platform%" == "Win32" (
	@rem OK
) else if "%platform%" == "x64" (
	@rem OK
) else (
	call :showhelp %0
	exit /b 1
)

if "%configuration%" == "Release" (
	@rem OK
) else if "%configuration%" == "Debug" (
	@rem OK
) else (
	call :showhelp %0
	exit /b 1
)

rem Definitions & Dependencies

if not defined Build^
   set Build=%~dp0%Platform%\%Configuration%
set RECIPE_BAT=%~dp0tools\recipeProcessor.bat
set RECIPE=%~dp0installer\recipe.txt
set DST=%Build%\installer
set SRC=%~dp0
set SRC=%SRC:~0,-1%
call :Set_RELATIVE_To "%Build%" "%SRC%"
set BLD=%RELATIVE%
set ISS=%DST%\sakura-%platform%.iss
set ISS_LOG=%SRC%\iss-%platform%-%configuration%.log

rem Setup

mkdir 2>nul "%DST%"

call "%~dp0tools\ISCC\find-ISCC.bat"
if not defined CMD_ISCC (
	echo ISCC.exe was not found.
	exit /b 1
)

rem Build

call|"%RECIPE_BAT%" "%RECIPE%" "%SRC%" "%DST%"^
 || (rmdir 2>nul /S /Q "%DST" & exit /b 1)

@echo running "%CMD_ISCC%" "%ISS%"
"%CMD_ISCC%" "%ISS%" > "%ISS_LOG%"^
 || (echo error && exit /b 1)
exit /b 0

rem ---------------------------------------------------

:Set_RELATIVE_To
	setlocal

	set To=?%~dpnx1
	set From=?%~dpnx2
	if not "%From:~-1%" == "\" set From=%From%\
	set _Parents=%~3
	call set Relative=%%To:%From%=%%

	if not "%To%" == "%Relative%" (
		endlocal & set "RELATIVE=%_Parents%%Relative%"
		exit /b 0
	)
	if "%~dp2" == "%~dpnx2" (
		endlocal & set RELATIVE=
		exit /b 1
	)
	endlocal & call %0 "%~dpnx1" "%~dp2" "..\%_Parents%"
exit /b 0

@rem ------------------------------------------------------------------------------
@rem show help
@rem see http://orangeclover.hatenablog.com/entry/20101004/1286120668
@rem ------------------------------------------------------------------------------
:showhelp
@echo off
@echo usage
@echo    %~nx1 platform configuration
@echo.
@echo parameter
@echo    platform      : Win32   or x64
@echo    configuration : Release or Debug
@echo.
@echo example
@echo    %~nx1 Win32 Release
@echo    %~nx1 Win32 Debug
@echo    %~nx1 x64   Release
@echo    %~nx1 x64   Debug
exit /b 0
