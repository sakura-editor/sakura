set platform=%1
set configuration=%2
set ERROR_RESULT=0

if not defined CMD_CMAKE call "%~dp0..\tools\find-tools.bat"
if not defined CMD_CMAKE (
	echo cmake.exe was not found.
	exit /b 1
)

if "%platform%" neq "MinGW" (
	echo nothing to do for Non-MinGW Platforms.
	exit /b 0
)

set BUILDDIR=%~dp0..\build\%platform%\%configuration%\tests
"%CMD_CMAKE%" --build %BUILDDIR% --config %configuration% || set ERROR_RESULT=1

if "%ERROR_RESULT%" == "1" (
	@echo ERROR
	exit /b 1
)
