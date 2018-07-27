@echo off

SETLOCAL

set EXEDIR=%1
set SRCDIR=..\sakura_core
set DSTDIR=%SRCDIR%
set MAKEFILEDIR=%SRCDIR%
set TOPDIR=%SRCDIR%

set EXE_HEADERMAKE=HeaderMake
set EXE_MAKEFILEMAKE=MakefileMake

if not "%EXEDIR%" == "" (
	set HEADERMAKE=%EXEDIR%\%EXE_HEADERMAKE%
	set MAKEFILEMAKE=%EXEDIR%\%EXE_MAKEFILEMAKE%
) else (
	set HEADERMAKE=%EXE_HEADERMAKE%
	set MAKEFILEMAKE=%EXE_MAKEFILEMAKE%
)

@echo =======================
@echo preBuild
@echo =======================

@echo.
@echo ---- HeaderMake ----
%HEADERMAKE% -in=%SRCDIR%\Funccode_x.hsrc -out=%DSTDIR%\Funccode_define.h -mode=define
%HEADERMAKE% -in=%SRCDIR%\Funccode_x.hsrc -out=%DSTDIR%\Funccode_enum.h -mode=enum -enum=EFunctionCode

@echo.
@echo ---- MakefileMake ----
%MAKEFILEMAKE% -file=%MAKEFILEDIR%\Makefile -dir=%TOPDIR%

call githash.bat %SRCDIR%
