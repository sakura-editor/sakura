@echo off
set platform=%1
@rem doesn't use
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

call %~dp0tools\find-tools.bat
if not defined CMD_CPPCHECK (
	echo cppcheck.exe was not found. so skip to run it.
	exit /b 0
)

set CPPCHECK_OUT=cppcheck-%platform%-%configuration%.xml
set CPPCHECK_LOG=cppcheck-%platform%-%configuration%.log

set CPPCHECK_PLATFORM=
if "%PLATFORM%" == "Win32" (
	set CPPCHECK_PLATFORM=win32W
) else if "%PLATFORM%" == "x64" (
	set CPPCHECK_PLATFORM=win64
) else (
	@echo not supported platform
	exit /b 1
)

if exist "%CPPCHECK_OUT%" (
	del %CPPCHECK_OUT%
)

if exist "%CPPCHECK_LOG%" (
	del %CPPCHECK_LOG%
)

set CPPCHECK_PARAMS=
set CPPCHECK_PARAMS=%CPPCHECK_PARAMS% --force
set CPPCHECK_PARAMS=%CPPCHECK_PARAMS% --enable=all
set CPPCHECK_PARAMS=%CPPCHECK_PARAMS% --xml
set CPPCHECK_PARAMS=%CPPCHECK_PARAMS% --platform=%CPPCHECK_PLATFORM%
set CPPCHECK_PARAMS=%CPPCHECK_PARAMS% --output-file=%CPPCHECK_OUT%
set CPPCHECK_PARAMS=%CPPCHECK_PARAMS% -j %NUMBER_OF_PROCESSORS%
set CPPCHECK_PARAMS=%CPPCHECK_PARAMS% %~dp0sakura_core

set ERROR_RESULT=0
if exist "%CMD_CPPCHECK%" (
	@echo "%CMD_CPPCHECK%" %CPPCHECK_PARAMS%
	"%CMD_CPPCHECK%" %CPPCHECK_PARAMS% > %CPPCHECK_LOG% || set ERROR_RESULT=1
	@echo.
	@echo The log files are %CPPCHECK_LOG% and %CPPCHECK_OUT%
	@echo cppcheck success
)
exit /b %ERROR_RESULT%


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
