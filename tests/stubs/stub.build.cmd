setlocal
set SOURCE_DIR=%~dp1
set BUILD_DIR=%~dp2
set OUT_DIR=%~dp3

:: find generic tools
if not defined CMD_VSWHERE call %~dp0..\tools\find-tools.bat

if not exist "%CMD_CMAKE%" (
  echo "no cmake found."
  exit /b 1
)

if not exist "%CMD_NINJA%" (
  set GENERATOR="%CMAKE_G_PARAM%"
  set GENERATOR_OPTS=-A %PLATFORM% "-DCMAKE_CONFIGURATION_TYPES=Debug;Release"
  set "MAKE_PROGRAM=%CMD_MSBUILD%"
) else (
  set GENERATOR=Ninja
  set GENERATOR_OPTS=-DCMAKE_BUILD_TYPE=%CONFIGURATION%
  set "MAKE_PROGRAM=%CMD_NINJA%"
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%" > NUL 2>&1
pushd "%BUILD_DIR%" || endlocal && exit /b 1

if not exist CMakeCache.txt call :run_cmake_configure || endlocal && exit /b 1
:: "%CMD_CMAKE%" --build . --config %CONFIGURATION% || endlocal && exit /b 1

endlocal && exit /b 0

:run_cmake_configure
call :find_cl_compiler

:: replace back-slash to slash in the path.
set CL_COMPILER=%CMD_CL:\=/%

echo %CD%

:: run cmake configuration.
"%CMD_CMAKE%" -G %GENERATOR%^
  "-DCMAKE_MAKE_PROGRAM=%MAKE_PROGRAM%"^
  "-DCMAKE_C_COMPILER=%CL_COMPILER%"^
  "-DCMAKE_CXX_COMPILER=%CL_COMPILER%"^
  "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%OUT_DIR%"^
  %GENERATOR_OPTS%^
  "%SOURCE_DIR%"^
  || endlocal && exit /b 1
goto :EOF

:find_cl_compiler
for /f "usebackq delims=" %%a in (`where cl.exe`) do (
  set "CMD_CL=%%a"
  goto :EOF
)
goto :EOF
