setlocal
set BUILD_BASE_DIR=%~dp1
set GOOGLETEST_INSTALL_PATH=%~dp2

set SOURCE_DIR=%~dp0googletest

:: find generic tools
if not defined CMD_VSWHERE call %~dp0..\tools\find-tools.bat

set /a NUM_VSVERSION_NEXT=NUM_VSVERSION + 1

if not exist "%CMD_CMAKE%" (
  echo "no cmake found."
  exit /b 1
)

pushd "%SOURCE_DIR%" || exit /b 1
if not exist CMakeLists.txt (
  if not exist "%CMD_GIT%" (
    echo "no git found."
    exit /b 1
  )
  "%CMD_GIT%" submodule update --init || exit /b 1
)
popd

if not exist "%CMD_NINJA%" (
  set GENERATOR="%CMAKE_G_PARAM%"
  set GENERATOR_OPTS=-A %PLATFORM% "-DCMAKE_CONFIGURATION_TYPES=Debug;Release"
  set "MAKE_PROGRAM=%CMD_MSBUILD%"
  set "BUILD_DIR=%BUILD_BASE_DIR%googletest\%platform%"
) else (
  set GENERATOR=Ninja
  set GENERATOR_OPTS=-DCMAKE_BUILD_TYPE=%CONFIGURATION%
  set "MAKE_PROGRAM=%CMD_NINJA%"
  set "BUILD_DIR=%BUILD_BASE_DIR%googletest\%platform%\%configuration%"
)

mkdir %BUILD_DIR% > NUL 2>&1
pushd %BUILD_DIR%

call :run_cmake_install

endlocal && exit /b 0

:run_cmake_install
call :run_cmake_configure
"%CMD_CMAKE%" --build . --config %CONFIGURATION% --target install || endlocal && exit /b 1
goto :EOF

:run_cmake_configure
call :find_cl_compiler

:: replace back-slash to slash in the path.
set CL_COMPILER=%CMD_CL:\=/%

:: install lib64 for x64-platform.
if "%PLATFORM%" == "x64" (
  set GENERATOR_OPTS=-DCMAKE_INSTALL_LIBDIR=lib64 %GENERATOR_OPTS%
)

:: run cmake configuration.
"%CMD_CMAKE%" -G %GENERATOR%                        ^
  "-DCMAKE_MAKE_PROGRAM=%MAKE_PROGRAM%"             ^
  "-DCMAKE_C_COMPILER=%CL_COMPILER%"                ^
  "-DCMAKE_CXX_COMPILER=%CL_COMPILER%"              ^
  -DCMAKE_INSTALL_PREFIX=%GOOGLETEST_INSTALL_PATH%  ^
  %GENERATOR_OPTS%                                  ^
  -DBUILD_GMOCK=OFF                                 ^
  -Dgtest_build_tests=OFF                           ^
  -Dgtest_build_samples=OFF                         ^
  %SOURCE_DIR%                                      ^
  || endlocal && exit /b 1
goto :EOF

:find_cl_compiler
for /f "usebackq delims=" %%a in (`where cl.exe`) do (
  set "CMD_CL=%%a"
  goto :EOF
)
goto :EOF
