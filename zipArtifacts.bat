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
set SRC=%~dp0
set SRC=%SRC:~0,-1%
set DST=%~dp0%BASENAME%
set TAB=	
set HASH_BAT=!SRC!\calc-hash.bat
set  ZIP_BAT=!SRC!\tools\zip\zip.bat

rem Setup

rmdir /s /q "%DST%" 2>nul
mkdir       "%DST%"

rem Main

set WORKING_ZIP=
set WORKING_PATH=.\
set WORKING_FILE=
for /F "usebackq tokens=* eol=# delims=" %%L in ("%~dpn0.txt" 'FINISHED') do (
	rem TODO: Forbid ".."
	rem Prevent the next 'for' command from merging empty columns.
	set L=%%L%TAB%%TAB%%TAB%
	set L=!L:%TAB%= %TAB%!
for /F "usebackq tokens=1,2,3 delims=%TAB%" %%A in ('!L!') do (
	rem First column: Zip name (relative to %DST% for working & relative to %SRC% for output)
	set rpnxA=%%~dpnxA
	set rpnxA=!rpnxA:%CD:)=^)%\=!
	if not "!rpnxA: =!" == "" (
		rem Make a zip before switching WORKING_ZIP.
		if defined WORKING_ZIP for /F "delims=" %%P in ("!DST!\!WORKING_ZIP!") do (
			cmd /V:ON /C "pushd "%%~P" &("!HASH_BAT!" sha256.txt . >nul)& popd"^
			|| del "%%~P\sha256.txt" 2>nul
			cmd /V:ON /C ""!ZIP_BAT!" "!SRC!\%%~nxP" "%%~P\*""^
			|| del "!SRC!\%%~nxP" 2>nul
			rmdir /S /Q "%%~P"

			set WORKING_ZIP=
			set WORKING_PATH=.\
			set WORKING_FILE=
		)

		@echo ZIP  !rpnxA!

		rem Prepare working directory for a zip.
		mkdir 2>nul  "!DST!\!rpnxA!"
		if not exist "!DST!\!rpnxA!" (
			goto :CleanExit 1
		)

		set WORKING_ZIP=!rpnxA!
	)
	rem Second column: Path (destination, relative to Zip)
	set rpB=%%~dpB
	set rpB=!rpB:%CD:)=^)%\=!
	if not "!rpB: =!" == "" (
		@echo PATH !rpB!

		rem Unfinished preparation.
		if not defined WORKING_ZIP (
			@echo>&2 ERROR: Give zip name before path.
			goto :CleanExit 1
		)
		if not exist "!DST!\!WORKING_ZIP!" (
			@echo>&2 ERROR: Missing directory: !DST!\!WORKING_ZIP!
			goto :CleanExit 1
		)
		rem Prepare working directory for a path.
		mkdir 2>nul  "!DST!\!WORKING_ZIP!\!rpB!"
		if not exist "!DST!\!WORKING_ZIP!\!rpB!" (
			goto :CleanExit 1
		)

		set WORKING_PATH=!rpB!
		set WORKING_FILE=!%%~nxB!
	)
	rem Third column: File (source, relative to %SRC%)
	if not "%%~C" == " " (
		@echo FILE %%~C

		rem Unfinished preparation.
		if not exist "!DST!\!WORKING_ZIP!\!WORKING_PATH!" (
			@echo>&2 ERROR: Missing directory: !DST!\!WORKING_ZIP!\!WORKING_PATH!
		)
		rem Prepare working file.
		copy /Y /B "!SRC!\%%~C" "!DST!\!WORKING_ZIP!\!WORKING_PATH!!WORKING_FILE!"
	)
))

goto :CleanExit 0

:CleanExit
	if defined WORKING_ZIP rmdir /S /Q "%DST%\%WORKING_ZIP%" 2>nul
	rmdir /Q "%DST%"
exit /b %1


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
