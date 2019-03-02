set SRCZIP=%1

@echo examining %SRCZIP%.
@rem CMD_7Z contains double quotes.
if not defined CMD_7Z call "%~dp0..\find-tools.bat" > NUL
if not defined CMD_7Z (
	@echo ------------------------------------------------------- >&2
	@echo ---- you can make this faster by installing 7-zip. ---- >&2
	@echo ------------------------------------------------------- >&2
	powershell -ExecutionPolicy RemoteSigned -File %~dp0listzip.ps1 %SRCZIP%
) else (
	"%CMD_7Z%" l "%SRCZIP%"
)
