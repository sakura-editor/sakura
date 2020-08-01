@echo off
set platform=%1
set configuration=%2

if "%platform%" == "MinGW" (
	@rem OK
) else (
	call :showhelp %0
	exit /b 1
)

if not defined CMD_CMAKE call "%~dp0tools\find-tools.bat"
if not defined CMD_CMAKE (
	@echo cmake.exe was not found. 1>&2
	exit /b 1
)

if not defined CMD_NINJA (
	@echo ninja.exe was not found. 1>&2
	exit /b 1
)

set SOURCE_DIR=%~dp0
set BUILD_BASE_DIR=%~dp0build\
set BUILD_DIR=%BUILD_BASE_DIR%%platform%\%configuration%\
set OUTPUT_BASE_DIR=%~dp0
set OUTPUT_DIR=%OUTPUT_BASE_DIR%%platform%\%configuration%\
set GOOGLETEST_INSTALL_DIR=%OUTPUT_BASE_DIR%tools\googletest\

call :run_cmake_build

goto :EOF

:run_cmake_configure
@rem create build directory, if not exist.
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

@rem add msys2 path to the PATH.
set path=%path:C:\msys64\mingw64\bin;=%
path=C:\msys64\mingw64\bin;%path%

pushd %BUILD_DIR%

:: run cmake configuration.
"%CMD_CMAKE%" -G Ninja ^
  "-DCMAKE_MAKE_PROGRAM=%CMD_NINJA%" ^
  -DCMAKE_BUILD_TYPE=%configuration% ^
  -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe ^
  -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe ^
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=%OUTPUT_DIR% ^
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%OUTPUT_DIR% ^
  -DPLATFORM=MinGW ^
  -DBUILD_BASE_DIR=%BUILD_BASE_DIR% ^
  -DGOOGLETEST_INSTALL_DIR=%GOOGLETEST_INSTALL_DIR% ^
  %SOURCE_DIR% ^
  || endlocal && exit /b 1

popd 

goto :EOF

:run_cmake_build
if not exist "%BUILD_DIR%CMakeCache.txt" (
  call :run_cmake_configure || endlocal && exit /b 1
)

:: run cmake build, and install artifacts.
"%CMD_CMAKE%" --build %BUILD_DIR% --config %configuration% || endlocal && exit /b 1

goto :EOF

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
