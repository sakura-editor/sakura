@echo off

set EXEDIR=%1
set INDIR=%2
set OUTDIR=%3

if "%EXEDIR%" == "" (
	set EXEDIR=.
)
if "%INDIR%" == "" (
	set INDIR=.
)
if "%OUTDIR%" == "" (
	set OUTDIR=.
)

%EXEDIR%\HeaderMake -in=%INDIR%\Funccode_x.hsrc -out=%OUTDIR%\Funccode_define.h -mode=define
%EXEDIR%\HeaderMake -in=%INDIR%\Funccode_x.hsrc -out=%OUTDIR%\Funccode_enum.h -mode=enum -enum=EFunctionCode
