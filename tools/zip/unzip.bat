@echo off
set SRCZIP=%1
set OUTDIR=%2

if exist "%OUTDIR%" rmdir /s /q "%OUTDIR%"

@echo extracting %SRCZIP%.
if not defined CMD_7Z call %~dp0..\find-tools.bat
if not defined CMD_7Z (
	@echo -------------------------------------------------------
	@echo ---- you can make this faster by installing 7-zip. ----
	@echo -------------------------------------------------------
	powershell -ExecutionPolicy RemoteSigned -File %~dp0unzip.ps1 %SRCZIP% %OUTDIR%
) else (
	"%CMD_7Z%" x "%SRCZIP%"  -O"%OUTDIR%"
)
@echo on
