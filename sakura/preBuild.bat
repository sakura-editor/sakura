@echo off

SETLOCAL

set EXEDIR=%1
set SRCDIR=..\sakura_core

@echo =======================
@echo preBuild
@echo =======================

call gen-makefile.bat %EXEDIR% %SRCDIR%
