@echo off
set platform=%1
set configuration=%2
set ERROR_RESULT=0

set ROOTDIR=%~dp0build\bin\%configuration%

pushd %ROOTDIR%
for /r %%i in (tests*.exe) do (
	@echo %%i --gtest_list_tests
	%%i --gtest_list_tests || set ERROR_RESULT=1

	@echo %%i
	%%i || set ERROR_RESULT=1
)
popd

if "%ERROR_RESULT%" == "1" (
	@echo ERROR
	exit /b 1
)
