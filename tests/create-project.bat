@echo off
setlocal ENABLEDELAYEDEXPANSION
set platform=%1
set configuration=%2

rem produces header files necessary in creating the project.

if "%platform%" == "MinGW" (
	call :CMAKE_PATH_FIX
	set BUILD_EDITOR_BAT=build-gnu.bat
) else (
	set BUILD_EDITOR_BAT=build-sln.bat
)
@echo ---- start %BUILD_EDITOR_BAT% ----
cmd /C "pushd "%~dp0.." & call "%BUILD_EDITOR_BAT%" %platform% %configuration%" ^
 || (echo ERROR & exit /b 1)
@echo ---- end   %BUILD_EDITOR_BAT% ----


pushd "%~dp0"
if not exist "googletest\CMakeLists.txt" (
    git submodule init
    git submodule update
)
popd

set BUILDDIR=%~dp0build\%platform%\%configuration%
if exist "%BUILDDIR%" (
	rmdir /s /q "%BUILDDIR%"
)
mkdir "%BUILDDIR%"

call :SetEnv_%platform% %platform% %configuration%
cmake %CMAKE_GEN_OPT% -H"%~dp0." -B"%BUILDDIR%" ^
 || (echo ERROR & exit /b 1)

exit /b 0

rem ----------------------------------------------
rem  sub-routines
rem ----------------------------------------------

:SetEnv_Win32
:SetEnv_x64
	set CMAKE_GEN_OPT=-G "Visual Studio 15 2017" -A "%~1"
	set CMAKE_BLD_OPT=--config "%~2"
exit /b 0

:SetEnv_MinGW
	set CMAKE_GEN_OPT=-G "MinGW Makefiles" -D CMAKE_BUILD_TYPE="%~2"
	set CMAKE_BLD_OPT=
	set PATH=C:\msys64\mingw64\bin;%PATH%
	call :CMAKE_PATH_FIX
exit /b 0

:CMAKE_PATH_FIX
	if not "%~1" == "sh.exe" (
		call %0 "sh.exe" & exit /b
	)
	set SHDIR=%~dp$PATH:1
	if "%SHDIR%" == "" (
		exit /b 0
	)
	set SHDIR=%SHDIR:~0,-1%
	set ORG=;%PATH%;
	set MOD=!ORG:;%SHDIR%\;=;%!
	set MOD=!MOD:;%SHDIR%;=;%!
	set PATH=%MOD:~1,-1%
	if not "%ORG%" == "%MOD%" (
		call %0 "sh.exe" & exit /b
	)
exit /b 0
