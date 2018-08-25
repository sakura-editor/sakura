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

if not exist "%EXEDIR%" (
	@echo.
	@echo "%EXEDIR%" does not exist.
	@echo.
	call :showhelp %0
	exit /b 1
)
if not exist "%SRCDIR%" (
	@echo.
	@echo "%SRCDIR%" does not exist.
	@echo.
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
@rem see http://orangeclover.hatenablog.com/entry/20101004/1286120668
@rem ------------------------------------------------------------------------------
:showhelp
@echo off
@echo usage %~nx1 "HeaderMake.exe directory" "Funccode_x.hsrc directory"
exit /b 0
