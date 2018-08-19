@echo off
set DSTZIP=%1
set SRCDIR=%2

if exist "%DSTZIP%" del "%DSTZIP%"

call %~dp0find-7z.bat

if "%CMD_7Z%" == "" (
	powershell -ExecutionPolicy RemoteSigned -File %~dp0zip.ps1 %DSTZIP% %SRCDIR%
) else (
	"%CMD_7Z%" a "%DSTZIP%"  -r "%SRCDIR%"
)
@echo on
