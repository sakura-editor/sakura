set PLATFORM=%1
set CONFIGURATION=%2

if "%PLATFORM%" == "MinGW" (
	@rem OK
) else (
	call :showhelp %0
	exit /b 1
)

if "%CONFIGURATION%" == "Release" (
    set MYDEFINES=-DNDEBUG
    set MYCFLAGS=-O2
    set MYLIBS=-s
) else if "%CONFIGURATION%" == "Debug" (
    set MYDEFINES=-D_DEBUG
    set MYCFLAGS=-g -O0
    set MYLIBS=
) else (
	call :showhelp %0
	exit /b 1
)

@rem https://www.appveyor.com/docs/environment-variables/
@rem path=C:\mingw-w64\x86_64-7.2.0-posix-seh-rt_v5-rev1\mingw64\bin;%path%
path=C:\msys64\mingw64\bin;%path:C:\msys64\mingw64\bin;=%

@rem create output directory, all executables will be placed here.
set OUTDIR=../../../../%PLATFORM%/%CONFIGURATION%
mkdir "%~dp0%PLATFORM%\%CONFIGURATION%" > NUL 2>&1

@rem build "sakura_core".
set SAKURA_CORE_MAKEFILE=%~dp0sakura_core\Makefile
set SAKURA_CORE_BUILD_DIR=%~dp0build\%PLATFORM%\%CONFIGURATION%\sakura_core
mkdir "%SAKURA_CORE_BUILD_DIR%" > NUL 2>&1
pushd "%SAKURA_CORE_BUILD_DIR%"
mingw32-make -f "%SAKURA_CORE_MAKEFILE%" MYDEFINES="%MYDEFINES%" MYCFLAGS="%MYCFLAGS%" MYLIBS="%MYLIBS%" OUTDIR=%OUTDIR% StdAfx.h.gch sakura_rc.o && mingw32-make -f "%SAKURA_CORE_MAKEFILE%" MYDEFINES="%MYDEFINES%" MYCFLAGS="%MYCFLAGS%" MYLIBS="%MYLIBS%" OUTDIR=%OUTDIR% -j4
if errorlevel 1 (
	echo error 2 errorlevel %errorlevel%
	popd
	exit /b 1
)
popd

@rem build "sakura_lang_en_US".
set SAKURA_LANG_EN_US_MAKEFILE=%~dp0sakura_lang_en_US\Makefile
set SAKURA_LANG_EN_US_BUILD_DIR=%~dp0build\%PLATFORM%\%CONFIGURATION%\sakura_lang_en_US
mkdir "%SAKURA_LANG_EN_US_BUILD_DIR%" > NUL 2>&1
pushd "%SAKURA_LANG_EN_US_BUILD_DIR%"
mingw32-make -f "%SAKURA_LANG_EN_US_MAKEFILE%" MYDEFINES="%MYDEFINES%" SAKURA_CORE=../sakura_core OUTDIR=%OUTDIR%
if errorlevel 1 (
	echo error 2 errorlevel %errorlevel%
	popd
	exit /b 1
)
popd

@rem build "tests1".
set TESTS1_MAKEFILE=%~dp0tests\unittests\Makefile
set TESTS1_BUILD_DIR=%~dp0build\%PLATFORM%\%CONFIGURATION%\tests1
mkdir "%TESTS1_BUILD_DIR%" > NUL 2>&1
pushd "%TESTS1_BUILD_DIR%"
mingw32-make -f "%TESTS1_MAKEFILE%" MYDEFINES="%MYDEFINES%" SAKURA_CORE=../sakura_core OUTDIR=%OUTDIR% -j4
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
@echo    %~nx1 PLATFORM CONFIGURATION
@echo.
@echo parameter
@echo    PLATFORM      : MinGW
@echo    CONFIGURATION : Release or Debug
@echo    BUILD_BASE    : (optional) Path to build directory.
@echo.
@echo example
@echo    %~nx1 MinGW Release
@echo    %~nx1 MinGW Debug
exit /b 0
