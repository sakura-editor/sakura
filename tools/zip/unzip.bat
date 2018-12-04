@echo off
set SRCZIP=%~1
set OUTDIR=%~2

if exist "%OUTDIR%" rmdir /s /q "%OUTDIR%"

call "%~dp0find-7z.bat"

@echo extracting %SRCZIP%.
@rem CMD_7Z contains double quotes.
if "%CMD_7Z%" == "" (
	@echo -------------------------------------------------------
	@echo ---- you can make this faster by installing 7-zip. ----
	@echo -------------------------------------------------------
	powershell -ExecutionPolicy RemoteSigned -File "%~dp0unzip.ps1" "%SRCZIP%" "%OUTDIR%"
) else (
	"%CMD_7Z%" x "%SRCZIP%"  -O"%OUTDIR%"
)
@echo on
