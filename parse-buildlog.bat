@echo off
where python --version 1>nul 2>&1
if not "%ERRORLEVEL%" == "0" (
	@echo NOTE: No python command
) else (
	python parse-buildlog.py %1
)
@echo on
exit /b 0
