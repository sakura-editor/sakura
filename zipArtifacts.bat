@echo off
@setlocal enabledelayedexpansion
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

if "%platform%" == "x64" (
	set ALPHA=alpha
) else (
	set ALPHA=
)

rem Definitions and Dependencies

call :Set_BASENAME
set RECIPE_BAT=%~dp0tools\recipeProcessor.bat
set RECIPE=%~dpn0.txt
set SRC=%~dp0
set DST=%~dp0%BASENAME%
set HASH_BAT=%~dp0calc-hash.bat
set  ZIP_BAT=%~dp0tools\zip\zip.bat

rem Setup

mkdir 2>nul "%DST%"

rem Zip

call|"%RECIPE_BAT%" "%RECIPE%" "%SRC%" "%DST%"^
 || (rmdir 2>nul /S /Q "%DST" & exit /b 1)

for /D %%Z in ("%DST%\*.zip") do (
	call :MakeZip "%%~Z"
)

exit /b 0

rem -----------------------------------------------------

:MakeZip
	setlocal

	set ZipSrc=%~1
	set ZipName=%~nx1
	if not defined ZipSrc exit /b 0

	cmd /C "pushd "%ZipSrc%" &"%HASH_BAT%" sha256.txt . >nul"^
	|| del 2>nul "%ZipSrc%\sha256.txt"
	cmd /C ""%ZIP_BAT%" "%SRC%\%ZipName%" "%ZipSrc%\*""^
	|| del 2>nul "%SRC%\%ZipName%"
	rmdir /S /Q "%ZipSrc%"
exit /b 0

rem ---------------------- BASENAME ---------------------------------
rem "sakura"
rem BUILD_ACCOUNT (option)
rem TAG_NAME      (option) "tag-" is prefixed.
rem PR_NUMBER     (option) "PR" is prefixed.
rem {BUILD_NUMBER|"Local"} "build" is prefixed.
rem SHORTHASH     (option) SHORTHASH is leading 8 charactors
rem PLATFORM
rem CONFIGURATION
rem ALPHA         (x64 build only)
rem ----------------------------------------------------------------

:Set_BASENAME
	setlocal ENABLEDELAYEDEXPANSION

	set BUILD_ACCOUNT=%APPVEYOR_ACCOUNT_NAME%
	if "%BUILD_ACCOUNT%" == "sakuraeditor" set BUILD_ACCOUNT=

	set TAG_NAME=%APPVEYOR_REPO_TAG_NAME%
	if defined TAG_NAME set TAG_NAME=tag-%TAG_NAME%
	call :ReplaceForbiddenPathChars TAG_NAME

	set PR_NUMBER=%APPVEYOR_PULL_REQUEST_NUMBER%
	if defined PR_NUMBER set PR_NUMBER=PR%PR_NUMBER%

	set BUILD_NUMBER=%APPVEYOR_BUILD_NUMBER%
	if not defined BUILD_NUMBER set BUILD_NUMBER=Local
	set BUILD_NUMBER=build%BUILD_NUMBER%

	set SHORTHASH=%APPVEYOR_REPO_COMMIT%
	if defined SHORTHASH set SHORTHASH=%SHORTHASH:~0,8%

	rem PLATFORM

	rem CONFIGURATION

	rem ALPHA

	set BASENAME=sakura
	for %%V in (
		BUILD_ACCOUNT TAG_NAME PR_NUMBER BUILD_NUMBER
		SHORTHASH PLATFORM CONFIGURATION ALPHA
	) do (
		@echo %%V=!%%V!
		if defined %%V set BASENAME=!BASENAME!-!%%V!
	)
	@echo BASENAME=%BASENAME%

	endlocal & set BASENAME=%BASENAME%
exit /b 0

rem '/' -> '_'
rem ' ' -> '_'
:ReplaceForbiddenPathChars
	setlocal ENABLEDELAYEDEXPANSION

	set VAR=%~1
	if not defined VAR exit /b 1
	set VAL=!%VAR%!
	if not defined VAL exit /b 1

	set VAL=%VAL:/=_%
	set VAL=%VAL: =_%

	endlocal & set %VAR%=%VAL%
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
