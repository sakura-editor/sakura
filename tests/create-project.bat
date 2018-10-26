set platform=%1
set configuration=%2
set ERROR_RESULT=0

pushd "%~dp0.."
rem   produces header files necessary in creating the project.
@echo ---- start build-sln.bat ----
call  build-sln.bat %platform% %configuration% || set ERROR_RESULT=1
@echo ---- end   build-sln.bat ----
popd
if "%ERROR_RESULT%" == "1" (
	@echo ERROR
	exit /b 1
)

pushd "%~dp0"

if not exist "googletest\CMakeLists.txt" (
    git submodule init
    git submodule update
)

set BUILDDIR=build\%platform%
if exist "%BUILDDIR%" (
	rmdir /s /q "%BUILDDIR%"
)
mkdir %BUILDDIR%
cmake -DCMAKE_GENERATOR_PLATFORM=%platform% -B%BUILDDIR% -H. || set ERROR_RESULT=1

popd

if "%ERROR_RESULT%" == "1" (
	@echo ERROR
	exit /b 1
)
