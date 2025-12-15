@echo off
set platform=%1
set configuration=%2

if "%platform%" == "MinGW" (
	@rem OK
) else (
	call :showhelp %0
	exit /b 1
)

if not defined MSYSTEM call :setmsys64path

set BUILD_DIR=build/%platform%

@rem run cmake configure.
cmake -S . -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=%configuration% -DBUILD_PLATFORM=%platform%
if errorlevel 1 (
	echo cmake configure failed. errorlevel %errorlevel%
	exit /b 1
)

@rem build "sakura_core".
cmake --build %BUILD_DIR% --config %configuration% --target sakura
if errorlevel 1 (
	echo cmake build sakura failed. errorlevel %errorlevel%
	exit /b 1
)

@rem build "sakura_lang_en_US".
cmake --build %BUILD_DIR% --config %configuration% --target sakura_lang_en_US
if errorlevel 1 (
	echo cmake build sakura_lang_en_US failed. errorlevel %errorlevel%
	exit /b 1
)

@rem build "tests1".
cmake --build %BUILD_DIR% --config %configuration% --target tests1
if errorlevel 1 (
	echo cmake build tests1 failed. errorlevel %errorlevel%
	exit /b 1
)

@rem run test.
ctest --test-dir %BUILD_DIR% --build-config %configuration% --output-on-failure
if errorlevel 1 (
	echo ctest failed. errorlevel %errorlevel%
	exit /b 1
)

exit /b 0

:setmsys64path
path=C:\msys64\usr\bin;%path:C:\msys64\usr\bin;=%
path=C:\msys64\mingw64\bin;%path:C:\msys64\mingw64\bin;=%
exit /b 0

@rem ------------------------------------------------------------------------------
@rem show help
@rem see http://orangeclover.hatenablog.com/entry/20101004/1286120668
@rem ------------------------------------------------------------------------------
:showhelp
@echo off
@echo usage
@echo    %~nx1 platform configuration
@echo.
@echo parameter
@echo    platform      : MinGW
@echo    configuration : Release or Debug
@echo.
@echo example
@echo    %~nx1 MinGW Release
@echo    %~nx1 MinGW Debug
exit /b 0
