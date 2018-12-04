@echo off
set OUTHASHFILE=%~1
set SRCDIR=%~2

if "%OUTHASHFILE%" == "" (
	call :showhelp %0
	exit /b 1
)
if "%SRCDIR%" == "" (
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

where python --version 1>nul 2>&1
if errorlevel 1 (
	@echo NOTE: No python command
) else (
	python "%~dp0calc-hash.py" "%OUTHASHFILE%" "%SRCDIR%"
)

exit /b 0

@rem ------------------------------------------------------------------------------
@rem show help
@rem see http://orangeclover.hatenablog.com/entry/20101004/1286120668
@rem ------------------------------------------------------------------------------
:showhelp
@echo off
@echo usage
@echo    %~nx1 "output hash file" "source directory"
@echo.
@echo parameter
@echo    output hash file : filename of hash file
@echo    source directory : source directory where target files are in
@echo.
@echo example
@echo    %~nx1 sha256.txt sakura-buildLocal-Win32-Release
exit /b 0
