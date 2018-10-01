@echo off
set platform=%1
set configuration=%2

if "%platform%" == "MinGW" (
	@rem OK
) else (
	call :showhelp %0
	exit /b 1
)

if "%configuration%" == "Release" (
	@rem OK
    set MYDEFINES=-DNDEBUG
    set MYCFLAGS=-O2
    set MYLIBS=-s
) else if "%configuration%" == "Debug" (
	@rem OK
    set MYDEFINES=-D_DEBUG
    set MYCFLAGS=-g -O0
    set MYLIBS=
) else (
	call :showhelp %0
	exit /b 1
)

@rem https://www.appveyor.com/docs/environment-variables/
@rem path=C:\mingw-w64\x86_64-7.2.0-posix-seh-rt_v5-rev1\mingw64\bin;%path%
path=C:\msys64\mingw64\bin;%path%

@echo mingw32-make -C sakura_core MYDEFINES="%MYDEFINES%" MYCFLAGS="%MYCFLAGS%" MYLIBS="%MYLIBS%"
mingw32-make -C sakura_core MYDEFINES="%MYDEFINES%" MYCFLAGS="%MYCFLAGS%" MYLIBS="%MYLIBS%" githash stdafx Funccode_enum.h Funccode_define.h
if %errorlevel% neq 0 (popd && exit /b 1)

mingw32-make -C sakura_core MYDEFINES="%MYDEFINES%" MYCFLAGS="%MYCFLAGS%" MYLIBS="%MYLIBS%" -j4
if %errorlevel% neq 0 (popd && exit /b 1)

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
@echo    platform      : MinGW
@echo    configuration : Release or Debug
@echo.
@echo example
@echo    %~nx1 MinGW Release
@echo    %~nx1 MinGW Debug
exit /b 0
