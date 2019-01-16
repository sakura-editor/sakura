@echo off
set DSTZIP=%1
set SRCDIR=%2

if exist "%DSTZIP%" del "%DSTZIP%"
@echo archiving %SRCZIP%.

if not defined CMD_7Z call %~dp0..\find-tools.bat
if not defined CMD_7Z (
	@echo -------------------------------------------------------
	@echo ---- you can make this faster by installing 7-zip. ----
	@echo -------------------------------------------------------
	powershell -ExecutionPolicy RemoteSigned -File %~dp0zip.ps1 %DSTZIP% %SRCDIR%
) else (
	"%CMD_7Z%" a "%DSTZIP%"  -r "%SRCDIR%"
)
@echo on
