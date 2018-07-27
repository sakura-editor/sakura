@echo off

SETLOCAL

set EXEDIR=%1
set SRCDIR=%2
set DSTDIR=%SRCDIR%

set EXE_HEADERMAKE=HeaderMake
set HEADERMAKE=%EXEDIR%\%EXE_HEADERMAKE%

@echo.
@echo ---- HeaderMake ----
%HEADERMAKE% -in=%SRCDIR%\Funccode_x.hsrc -out=%DSTDIR%\Funccode_define.h -mode=define
%HEADERMAKE% -in=%SRCDIR%\Funccode_x.hsrc -out=%DSTDIR%\Funccode_enum.h -mode=enum -enum=EFunctionCode
