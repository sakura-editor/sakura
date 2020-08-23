set platform=%1
set configuration=%2
set ERROR_RESULT=0

if not defined CMD_CMAKE call "%~dp0..\tools\find-tools.bat"
if not defined CMD_CMAKE (
	echo cmake.exe was not found.
	exit /b 1
)

@rem produces header files necessary in creating the project.
if "%platform%" == "MinGW" (
	set BUILD_EDITOR_BAT=build-gnu.bat
) else (
	set BUILD_EDITOR_BAT=build-sln.bat
)
pushd "%~dp0.."
@echo ---- start %BUILD_EDITOR_BAT% ----
call  "%BUILD_EDITOR_BAT%" %platform% %configuration% || set ERROR_RESULT=1
@echo ---- end   %BUILD_EDITOR_BAT% ----
popd
if "%ERROR_RESULT%" == "1" (
	@echo ERROR
	exit /b 1
)

set BINARY_DIR=%~dp0..\%platform%\%configuration%
set BUILDDIR=%~dp0..\build\%platform%\%configuration%\tests
if exist %BUILDDIR%\CMakeCache.txt (
	exit /b 0
)

if not exist %BUILDDIR% (
	mkdir %BUILDDIR%
)

if not exist %~dp0googletest\CMakeLists.txt (
	"%CMD_GIT%" submodule update --init
)

if "%platform%" == "MinGW" (
	call :setenv_MinGW %platform% %configuration%
) else (
	exit /b 0
)

pushd %BUILDDIR%
"%CMD_CMAKE%" %CMAKE_GEN_OPT% %~dp0 || set ERROR_RESULT=1
popd

if "%ERROR_RESULT%" == "1" (
	@echo ERROR
	exit /b 1
)

exit /b

@rem ----------------------------------------------
@rem  sub-routines
@rem ----------------------------------------------

:setenv_MinGW
	set CMAKE_GEN_OPT=-G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%configuration% -DBUILD_GTEST=OFF -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%BINARY_DIR% -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=%BINARY_DIR%
	set PATH=C:\msys64\mingw64\bin;%PATH:C:\Program Files\Git\usr\bin;=%
exit /b
