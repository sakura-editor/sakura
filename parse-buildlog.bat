@echo off

@rem -------------------------------------------------------
@rem install openpyxl module on appveyor
@rem -------------------------------------------------------
if "%APPVEYOR%" == "True" (
	call :openpyxl_install
) else (
	@echo skip 'pip install openpyxl --user'
)

where python --version 1>nul 2>&1
if not "%ERRORLEVEL%" == "0" (
	@echo NOTE: No python command
) else (
	python parse-buildlog.py %1
)
@echo on
exit /b %ERRORLEVEL%


:openpyxl_install

where pip 1>nul 2>&1
if not "%ERRORLEVEL%" == "0" (
	@echo NOTE: No pip command
) else (
	@echo NOTE: found pip command
	pip install openpyxl --user
)
exit /b
