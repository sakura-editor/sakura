@echo off

SETLOCAL

set EXEDIR=%1
set SRCDIR=%2
if "%EXEDIR%" == "" (
	call :showhelp %0
	exit /b 1
)

if "%SRCDIR%" == "" (
	call :showhelp %0
	exit /b 1
)

@rem replace '/' with '\'
set EXEDIR=%EXEDIR:/=\%
set SRCDIR=%SRCDIR:/=\%

set DSTDIR=%SRCDIR%

set EXE_HEADERMAKE=HeaderMake
set HEADERMAKE=%EXEDIR%\%EXE_HEADERMAKE%

@echo.
@echo ---- HeaderMake ----
%HEADERMAKE% -in=%SRCDIR%\Funccode_x.hsrc -out=%DSTDIR%\Funccode_define.h -mode=define
%HEADERMAKE% -in=%SRCDIR%\Funccode_x.hsrc -out=%DSTDIR%\Funccode_enum.h -mode=enum -enum=EFunctionCode
exit /b 0

@rem ------------------------------------------------------------------------------
@rem show help
@rem ------------------------------------------------------------------------------
:showhelp
@echo off
@echo usage %1 "HeaderMake.exe directory" "Funccode_x.hsrc directory"
exit /b
