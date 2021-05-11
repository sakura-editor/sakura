setlocal
set BUILD_DIR=%~dp1
set GOOGLETEST_INSTALL_PATH=%~dp2

set SOURCE_DIR=%~dp0googletest

:: find generic tools
if not defined CMD_NINJA call %~dp0..\tools\find-tools.bat

if not exist "%CMD_CMAKE%" (
  echo "no cmake found."
  exit /b 1
)

if not exist "%CMD_NINJA%" (
  echo "no ninja found."
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

mkdir %BUILD_DIR% > NUL 2>&1
pushd %BUILD_DIR%

call :run_cmake_install

endlocal && exit /b 0

:run_cmake_install
call :run_cmake_configure
"%CMD_CMAKE%" --build . --config %CONFIGURATION% --target install || endlocal && exit /b 1
goto :EOF

:run_cmake_configure
if "%PLATFORM%" == "Win32" (
  call :find_cl_compilers
)
if "%PLATFORM%" == "x64" (
  call :find_cl_compilers
)
if "%PLATFORM%" == "MinGW" (
  call :find_gcc_compilers
)

:: install lib64 for x64-platform.
if "%PLATFORM%" == "x64" (
  set GENERATOR_OPTS=-DCMAKE_INSTALL_LIBDIR=lib64 %GENERATOR_OPTS%
)

:: run cmake configuration.
"%CMD_CMAKE%" -G Ninja ^
  "-DCMAKE_MAKE_PROGRAM=%CMD_NINJA%" ^
  "-DCMAKE_C_COMPILER=%C_COMPILER%" ^
  "-DCMAKE_CXX_COMPILER=%CXX_COMPILER%" ^
  -DCMAKE_BUILD_TYPE=%CONFIGURATION% ^
  -DCMAKE_INSTALL_PREFIX=%GOOGLETEST_INSTALL_PATH%  ^
  %GENERATOR_OPTS%                                  ^
  -DBUILD_GMOCK=ON                                  ^
  -Dgtest_build_tests=OFF                           ^
  -Dgtest_build_samples=OFF                         ^
  %SOURCE_DIR%                                      ^
  || endlocal && exit /b 1
goto :EOF

:find_cl_compilers
call :find_cl
set C_COMPILER=%CMD_CL:\=/%
set CXX_COMPILER=%CMD_CL:\=/%
goto :EOF

:find_cl
for /f "usebackq delims=" %%a in (`where cl.exe`) do (
  set "CMD_CL=%%a"
  goto :EOF
)
goto :EOF

:find_gcc_compilers
set C_COMPILER=C:/msys64/mingw64/bin/gcc.exe
set CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe
goto :EOF
