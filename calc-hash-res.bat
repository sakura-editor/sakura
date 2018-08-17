@echo off
where python --version 1>nul 2>&1
if not "%ERRORLEVEL%" == "0" (
	@echo NOTE: No python command
) else (
	python calc-hash.py %1 %2 .res
)
@echo on
exit /b 0
