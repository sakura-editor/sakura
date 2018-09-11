set platform=%1
set configuration=%2
set ERROR_RESULT=0

pushd %~dp0

set BUILDDIR=build\%platform%
cmake --build %BUILDDIR%  --config %configuration% || set ERROR_RESULT=1

popd

if "%ERROR_RESULT%" == "1" (
	@echo ERROR
	exit /b 1
)
