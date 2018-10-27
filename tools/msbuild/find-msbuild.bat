@rem see readme.md
@echo off
set CMD_MSBUILD=

call :pathTest Enterprise   && goto :done
call :pathTest Professional && goto :done
call :pathTest Community    && goto :done

echo MsBuild has not found.
exit /b 1


:pathTest
:: will take a parameter to specify vs2017 editions
if        "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
  call :invokeTest "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\%~1\MSBuild\15.0\Bin\MSBuild.exe"
) else if "%PROCESSOR_ARCHITECTURE%" == "x86"   (
  call :invokeTest "%ProgramFiles%\Microsoft Visual Studio\2017\%~1\MSBuild\15.0\Bin\MSBuild.exe"
) else (
  :: echo architecture '%PROCESSOR_ARCHITECTURE%' is currently not supported.
  exit /b 1
)
if %errorlevel% neq 0 exit /b %errorlevel%
exit /b 0


:invokeTest
:: will take a parameter to invoke
%1 /version /nologo > NUL 2>&1
if %errorlevel% neq 0 exit /b %errorlevel%
set CMD_MSBUILD=%~1
exit /b 0


:done
@echo CMD_MSBUILD "%CMD_MSBUILD%"
@echo.
exit /b 0
