if not defined CMD_PYTHON call %~dp0tools\find-tools.bat
if not defined CMD_PYTHON (
	@echo ERROR: py.exe was not found.
	exit /b 1
)

"%CMD_PYTHON%" %~dp0addDoxygenFileComment.py %~dp0sakura_core
