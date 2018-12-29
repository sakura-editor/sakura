@echo off
set platform=%1
set configuration=%2
set TEST_LAUNCHED=0
set ERROR_RESULT=0

set UPLOAD_BAT=%~dp0upload.bat

pushd %~dp0
set BUILDDIR=build\%platform%
set BINARY_DIR=%BUILDDIR%\unittests\%platform%\%configuration%

pushd %BINARY_DIR%
for /r %%i in (tests*.exe) do (
	set TEST_LAUNCHED=1

	@echo %%i --gtest_list_tests
	%%i --gtest_list_tests || set ERROR_RESULT=1

	@echo %%i --gtest_output=xml:%%i.xml 
	%%i --gtest_output=xml:%%i.xml || set ERROR_RESULT=1
	call "%UPLOAD_BAT%" %%i.xml    || set ERROR_RESULT=1
)
popd
popd

if "%TEST_LAUNCHED%" == "0" (
	@echo ERROR: no tests are available.
	exit /b 1
)

if "%ERROR_RESULT%" == "1" (
	@echo ERROR
	exit /b 1
)
