@echo off
setlocal
set platform=%1
set configuration=%2

set BUILDDIR=%~dp0build\%platform%
set FILTER_BAT=%~dp0test_result_filter_tell_AppVeyor.bat

set TEST_LAUNCHED=0
set TEST_FAILED=0
for /r "%BUILDDIR%" %%T in (tests*.exe) do (
	@echo "%%T" --gtest_list_tests
	"%%T" --gtest_list_tests

	@echo "%%T" ^| "%FILTER_BAT%"
	"%%T" | "%FILTER_BAT%"

	if errorlevel 1 (
		set /A TEST_FAILED=%TEST_FAILED% + 1
	) else (
		set /A TEST_LAUNCHED=%TEST_LAUNCHED% + 1
	)
)
if "%TEST_LAUNCHED%" == "0" (
	@echo ERROR: no tests are available.
	exit /b 1
)
if not "%TEST_FAILED%" == "0" (
	@echo ERROR
	exit /b 1
)
exit /b 0
