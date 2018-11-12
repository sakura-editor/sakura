set platform=%1
set configuration=%2
set ERROR_RESULT=0

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

pushd "%~dp0"

if not exist "googletest\CMakeLists.txt" (
    git submodule init
    git submodule update
)

set BUILDDIR=build\%platform%
if exist "%BUILDDIR%" (
	rmdir /s /q "%BUILDDIR%"
)
mkdir "%BUILDDIR%"

call :setenv_%platform% %platform% %configuration%
cmake %CMAKE_GEN_OPT% -H. -B"%BUILDDIR%" || set ERROR_RESULT=1

popd

if "%ERROR_RESULT%" == "1" (
	@echo ERROR
	exit /b 1
)

exit /b

@rem ----------------------------------------------
@rem  sub-routines
@rem ----------------------------------------------

:setenv_Win32
:setenv_x64
	set CMAKE_GEN_OPT=-G "Visual Studio 15 2017" -A "%~1"
exit /b

:setenv_MinGW
	set CMAKE_GEN_OPT=-G "MinGW Makefiles" -D CMAKE_BUILD_TYPE="%~2"
	set PATH=C:\msys64\mingw64\bin;%PATH:C:\Program Files\Git\usr\bin;=%
exit /b
