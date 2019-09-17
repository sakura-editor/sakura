setlocal
set SOURCE_DIR=%1
set GENERATOR=%~2
set CONFIGURATION=%~3
set VCVARSALL_PATH=%4
set VCVARS_ARCH=%~5

@rem call vcvasall.bat when we run in the Visual Studio IDE.
if defined VCVARSALL_PATH (
	call %VCVARSALL_PATH% %VCVARS_ARCH% || endlocal && exit /b 1
)

if not exist CMakeCache.txt (
	call :run_cmake_configure
)

cmake --build . --config %CONFIGURATION% || endlocal && exit /b 1

endlocal && exit /b 0

:run_cmake_configure
where ninja.exe > NUL 2>&1
if not errorlevel 1 (
	set GENERATOR=Ninja
	set GENERATOR_OPTS=-DCMAKE_BUILD_TYPE=%CONFIGURATION%
)

@rem find cl.exe in the PATH
call :find_cl_exe

if not defined CMD_CL (
	echo cl.exe was not found.
	endlocal && exit /b 1
)
set CMD_CL=%CMD_CL:\=/%

cmake -G "%GENERATOR%" %GENERATOR_OPTS%                   ^
  "-DCMAKE_C_COMPILER=%CMD_CL%"                           ^
  "-DCMAKE_CXX_COMPILER=%CMD_CL%"                         ^
  -DBUILD_GMOCK=OFF                                       ^
  -Dgtest_build_tests=OFF                                 ^
  -Dgtest_build_samples=OFF                               ^
  %SOURCE_DIR%                                            ^
  || endlocal && exit /b 1

goto :EOF


:find_cl_exe
for /f "usebackq delims=" %%a in (`where cl.exe`) do (
    set CMD_CL=%%a
    goto :EOF
)
