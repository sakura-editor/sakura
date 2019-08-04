set platform=%1
set configuration=%2
set ERROR_RESULT=0

if not defined CMD_CMAKE call "%~dp0..\tools\find-tools.bat"
if not defined CMD_CMAKE (
	echo cmake.exe was not found.
	exit /b 1
)

pushd %~dp0
if not exist googletest (
    git submodule init
    git submodule update
)

set BUILDDIR=build\%platform%
"%CMD_CMAKE%" --build %BUILDDIR%  --config %configuration% || set ERROR_RESULT=1

popd

if "%ERROR_RESULT%" == "1" (
	@echo ERROR
	exit /b 1
)
