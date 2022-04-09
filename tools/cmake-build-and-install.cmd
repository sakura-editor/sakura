setlocal
set SOURCE_DIR=%~1
set BUILD_DIR=%~2
set INSTALL_DIR=%~3
set CMAKE_OPTS=%~4

:: using NUM_VSVERSION
:: using PLATFORM
:: using CONFIGURATION

:: find generic tools
if not defined CMD_NINJA call %~dp0find-tools.bat %NUM_VSVERSION%

if not exist "%CMD_NINJA%" (
  echo "no ninja found."
  exit /b 1
)

if not exist "%SOURCE_DIR%\CMakeLists.txt" (
  echo "CMakeLists.txt was not found."
  exit /b 1
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%" > NUL 2>&1
pushd "%BUILD_DIR%" || endlocal && exit /b 1
call :run_cmake_install || endlocal && exit /b 1
endlocal && exit /b 0

:run_cmake_install
if not exist "CMakeCache.txt" call :run_cmake_configure
"%CMD_CMAKE%" --build . --config %CONFIGURATION% --target install || exit /b 1
goto :EOF

:: run cmake configuration.
:run_cmake_configure
if "%PLATFORM%" == "x64"   call :find_msvc
if "%PLATFORM%" == "Win32" call :find_msvc
if "%PLATFORM%" == "MinGW" call :find_gcc

"%CMD_CMAKE%" -G Ninja^
  "-DCMAKE_MAKE_PROGRAM=%CMD_NINJA%"^
  "-DCMAKE_C_COMPILER=%C_COMPILER%"^
  "-DCMAKE_CXX_COMPILER=%CXX_COMPILER%"^
  -DCMAKE_BUILD_TYPE=%CONFIGURATION%^
  -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR%^
  %CMAKE_OPTS%^
  "%SOURCE_DIR%"
goto :EOF

:find_msvc
if not defined CMD_CL call :find_cl
set C_COMPILER=%CMD_CL:\=/%
set CXX_COMPILER=%CMD_CL:\=/%
goto :EOF

:find_cl
for /f "usebackq delims=" %%a in (`where cl.exe`) do (
  set "CMD_CL=%%a"
  goto :EOF
)
goto :EOF

:find_gcc
if not defined CMD_CC call :find_cc
if not defined CMD_CXX call :find_cxx
set C_COMPILER=%CMD_CC:\=/%
set CXX_COMPILER=%CMD_CXX:\=/%
goto :EOF

:find_cc
for /f "usebackq delims=" %%a in (`where gcc.exe`) do (
  set "CMD_CC=%%a"
  goto :EOF
)
goto :EOF

:find_cxx
for /f "usebackq delims=" %%a in (`where g++.exe`) do (
  set "CMD_CXX=%%a"
  goto :EOF
)
goto :EOF
