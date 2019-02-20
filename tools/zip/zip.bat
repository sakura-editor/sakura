set DSTZIP=%1
set SRCDIR=%2

if exist "%DSTZIP%" del "%DSTZIP%"
@echo archiving %SRCZIP%.

if not defined CMD_7Z call "%~dp0..\find-tools.bat" > NUL
if not defined CMD_7Z (
	@echo ------------------------------------------------------- >&2
	@echo ---- you can make this faster by installing 7-zip. ---- >&2
	@echo ------------------------------------------------------- >&2
	powershell -ExecutionPolicy RemoteSigned -File %~dp0zip.ps1 %DSTZIP% %SRCDIR%
) else (
	"%CMD_7Z%" a "%DSTZIP%" -r "%SRCDIR%"
)
