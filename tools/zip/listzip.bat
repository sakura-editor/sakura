@echo off
set SRCZIP=%1

call %~dp0find-7z.bat

@echo examining %SRCZIP%.
@rem CMD_7Z contains double quotes.
if "%CMD_7Z%" == "" (
	@echo -------------------------------------------------------
	@echo ---- you can make this faster by installing 7-zip. ----
	@echo -------------------------------------------------------
	powershell -ExecutionPolicy RemoteSigned -File %~dp0listzip.ps1 %SRCZIP%
) else (
	"%CMD_7Z%" l "%SRCZIP%"
)
@echo on
