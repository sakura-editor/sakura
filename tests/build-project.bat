@echo off
setlocal ENABLEDELAYEDEXPANSION
set platform=%1
set configuration=%2

call :SetEnv_%platform% %platform% %configuration%
set BUILDDIR=%~dp0build\%platform%

pushd "%~dp0"
if not exist "googletest\CMakeLists.txt" (
    git submodule init
    git submodule update
)
popd

cmake --build "%BUILDDIR%" %CMAKE_BLD_OPT% ^
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

rem Remove path to sh.exe from PATH env.
rem see https://github.com/vim/vim/blob/4dbc2627641a6b950c30c31cbf7b7e6c36da1927/src/Make_cyg_ming.mak#L176-L202
rem via https://github.com/sakura-editor/sakura/pull/591#discussion_r230551589
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
