set platform=%1
set configuration=%2
SET BUILD_BASE_DIR=%3

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

if not defined BUILD_BASE_DIR (
	set SAKURA_CORE_BUILD_DIR=%~dp0sakura_core
	set SAKURA_CORE_BUILD_MAKEFILE=.\Makefile
	set SAKURA_LANG_EN_US_BUILD_DIR=%~dp0sakura_lang_en_US
	set SAKURA_LANG_EN_US_MAKEFILE=.\Makefile
) else (
	set SAKURA_CORE_BUILD_DIR=%BUILD_BASE_DIR%\sakura_core
	set SAKURA_CORE_BUILD_MAKEFILE=%~dp0sakura_core\Makefile
	set SAKURA_LANG_EN_US_BUILD_DIR=%BUILD_BASE_DIR%\sakura_lang_en_US
	set SAKURA_LANG_EN_US_MAKEFILE=%~dp0sakura_lang_en_US\Makefile
)

@rem https://www.appveyor.com/docs/environment-variables/
@rem path=C:\mingw-w64\x86_64-7.2.0-posix-seh-rt_v5-rev1\mingw64\bin;%path%
path=C:\msys64\mingw64\bin;%path:C:\msys64\mingw64\bin;=%

@rem create output directory, all executables will be placed here.
set OUTDIR=%~dp0MinGW\%configuration%
if not exist "%OUTDIR%" (
	mkdir %OUTDIR% > NUL 2>&1
)

@rem create a directory for build "sakura_core".
if not exist "%SAKURA_CORE_BUILD_DIR%" (
	mkdir %SAKURA_CORE_BUILD_DIR% > NUL 2>&1
)
pushd %SAKURA_CORE_BUILD_DIR%

@rem build "sakura_core".
mingw32-make -f %SAKURA_CORE_BUILD_MAKEFILE% MYDEFINES="%MYDEFINES%" MYCFLAGS="%MYCFLAGS%" MYLIBS="%MYLIBS%" OUTDIR=%OUTDIR% -j%NUMBER_OF_PROCESSORS%
if errorlevel 1 (
	echo error 2 errorlevel %errorlevel%
	popd
	exit /b 1
)
popd

@rem create a directory for build "sakura_lang_en_US".
if not exist "%SAKURA_LANG_EN_US_BUILD_DIR%" (
	mkdir %SAKURA_LANG_EN_US_BUILD_DIR% > NUL 2>&1
)
pushd %SAKURA_LANG_EN_US_BUILD_DIR%

@rem build "sakura_lang_en_US".
mingw32-make -f %SAKURA_LANG_EN_US_MAKEFILE% MYDEFINES="%MYDEFINES%" OUTDIR=%OUTDIR% -j%NUMBER_OF_PROCESSORS%
if errorlevel 1 (
	echo error 2 errorlevel %errorlevel%
	popd
	exit /b 1
)
popd
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
@echo    BUILD_BASE_DIR : (optional) Path to build directory.
@echo.
@echo example
@echo    %~nx1 MinGW Release
@echo    %~nx1 MinGW Debug
exit /b 0
