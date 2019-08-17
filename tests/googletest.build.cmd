@rem echo off
setlocal
set SOURCE_DIR=%1
set CONFIGURATION=%2
set VCVARSALL_PATH=%3
set VCVARS_ARCH=%4

if not defined CMD_GIT call %~dp0..\tools\find-tools.bat
if not defined CMD_GIT (
	echo git.exe was not found.
	endlocal && exit /b 1
)

if not exist "%~dp0googletest\CMakeLists.txt" (
	"%CMD_GIT%" submodule init   %~dp0googletest || endlocal && exit /b 1
	"%CMD_GIT%" submodule update %~dp0googletest || endlocal && exit /b 1
)

@rem call vcvasall.bat when we run in the Visual Studio IDE.
if defined VCVARSALL_PATH (
	call %VCVARSALL_PATH% %VCVARS_ARCH% || endlocal && exit /b 1
)

@rem find cl.exe in the PATH
for /f "usebackq delims=" %%a in (`where cl.exe`) do ( 
    set CMD_CL=%%a
)
if not defined CMD_CL (
	echo cl.exe was not found.
	endlocal && exit /b 1
)
set CMD_CL=%CMD_CL:\=/%

cmake -G "Ninja" -DCMAKE_BUILD_TYPE=%CONFIGURATION%  ^
  "-DCMAKE_C_COMPILER=%CMD_CL%"                      ^
  "-DCMAKE_CXX_COMPILER=%CMD_CL%"                    ^
  -DBUILD_GMOCK=OFF                                  ^
  -Dgtest_build_tests=OFF                            ^
  -Dgtest_build_samples=OFF                          ^
  %SOURCE_DIR%                                       ^
  || endlocal && exit /b 1

cmake --build . || endlocal && exit /b 1

endlocal && exit /b 0
