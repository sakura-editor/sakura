set SRCZIP=%1
set OUTDIR=%2

if exist "%OUTDIR%" rmdir /s /q "%OUTDIR%" > NUL

@echo extracting %SRCZIP%.
if not defined CMD_7Z call "%~dp0..\find-tools.bat" > NUL
if not defined CMD_7Z (
	@echo ------------------------------------------------------- >&2
	@echo ---- you can make this faster by installing 7-zip. ---- >&2
	@echo ------------------------------------------------------- >&2
	powershell -ExecutionPolicy RemoteSigned -File %~dp0unzip.ps1 %SRCZIP% %OUTDIR%
) else (
	"%CMD_7Z%" x "%SRCZIP%" "-o%OUTDIR%"
)
