@echo off
set DSTZIP=%1
set SRCDIR=%2
set LISTFILE=%3

if exist "%DSTZIP%" del "%DSTZIP%"
@echo archiving %SRCZIP%.

if not defined CMD_7Z call %~dp0..\find-tools.bat
if not defined CMD_7Z (
	if defined LISTFILE (
		@echo Archving was skiped. Please install 7-zip.
		exit /b
	)
	@echo -------------------------------------------------------
	@echo ---- you can make this faster by installing 7-zip. ----
	@echo -------------------------------------------------------
	powershell -ExecutionPolicy RemoteSigned -File %~dp0zip.ps1 %DSTZIP% %SRCDIR%
) else (
	if not defined LISTFILE (
		"%CMD_7Z%" a "%DSTZIP%"  -r "%SRCDIR%"
	) else (
		setlocal
		cd /d "%SRCDIR%"
		"%CMD_7Z%" a "%DSTZIP%" @%LISTFILE%
		endlocal
	)
)
@echo on
