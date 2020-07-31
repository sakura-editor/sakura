setlocal
set platform=%1
set configuration=%2

set SOURCE_DIR=%~dp0compiletests

:: find generic tools
if not defined CMD_CMAKE call %~dp0..\tools\find-tools.bat
if not exist "%CMD_CMAKE%" (
  @echo no cmake found. 1>&2
  exit /b 1
)

set /a NUM_VSVERSION_NEXT=NUM_VSVERSION + 1

if exist "%CMD_NINJA%" (
  set GENERATOR=Ninja
  set GENERATOR_OPTS=-DCMAKE_BUILD_TYPE=%CONFIGURATION%
  set "MAKE_PROGRAM=%CMD_NINJA%"
) else (
  set GENERATOR="%CMAKE_G_PARAM%"
  set GENERATOR_OPTS=-A %PLATFORM% "-DCMAKE_CONFIGURATION_TYPES=Debug;Release"
  set "MAKE_PROGRAM=%CMD_MSBUILD%"
  pushd ..\
)

if not exist "./CMakeCache.txt" (
  call :run_cmake_configure || endlocal && exit /b 1
)

endlocal && exit /b 0

:run_cmake_configure
if not defined CMAKE_CXX_COMPILER (
  call :find_cxx_compiler
)

:: run cmake configuration.
"%CMD_CMAKE%" -G %GENERATOR%                        ^
  "-DCMAKE_MAKE_PROGRAM=%MAKE_PROGRAM%"             ^
  "-DCMAKE_C_COMPILER=%CMAKE_C_COMPILER%"           ^
  "-DCMAKE_CXX_COMPILER=%CMAKE_CXX_COMPILER%"       ^
  %GENERATOR_OPTS%                                  ^
  %SOURCE_DIR%                                      ^
  || endlocal && exit /b 1

goto :EOF

:find_cxx_compiler
if "%platform%" == "MinGW" (
  call :find_gcc_compiler
) else (
  call :find_cl_compiler
)

:: replace back-slash to slash in the path.
set CMAKE_C_COMPILER=%CMAKE_C_COMPILER:\=/%
set CMAKE_CXX_COMPILER=%CMAKE_CXX_COMPILER:\=/%

goto :EOF

:find_cl_compiler
for /f "usebackq delims=" %%a in (`where cl.exe`) do (
  set "CMAKE_C_COMPILER=%%a"
  set "CMAKE_CXX_COMPILER=%%a"
  goto :EOF
)
goto :EOF

:find_gcc_compiler
for /f "usebackq delims=" %%a in (`where g++.exe`) do (
  set "CMAKE_C_COMPILER=%%~dpagcc.exe"
  set "CMAKE_CXX_COMPILER=%%a"
  goto :EOF
)
goto :EOF
