@echo off

SETLOCAL

set EXEDIR=%1
set MAKEFILEDIR=%2
set TOPDIR=%MAKEFILEDIR%

set EXE_MAKEFILEMAKE=MakefileMake
set MAKEFILEMAKE=%EXEDIR%\%EXE_MAKEFILEMAKE%

@echo.
@echo ---- MakefileMake ----
%MAKEFILEMAKE% -file=%MAKEFILEDIR%\Makefile -dir=%TOPDIR%
