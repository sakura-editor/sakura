@echo off
set SRCZIP=%1

call %~dp0find-7z.bat

if "%CMD_7Z%" == "" (
	powershell -ExecutionPolicy RemoteSigned -File %~dp0listzip.ps1 %SRCZIP%
) else (
	"%CMD_7Z%" l "%SRCZIP%"
)
@echo on
