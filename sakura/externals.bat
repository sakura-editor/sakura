@echo on

: ---- arguments ---- :
: "Win32" or "x64"
set PLATFORM=%1
echo PLATFORM=%PLATFORM%

: "Debug" or "Release"
set CONFIGURATION=%2
echo CONFIGURATION=%CONFIGURATION%

if "%platform%" == "Win32" (
	@rem OK
) else if "%platform%" == "x64" (
	@rem OK
) else (
	call :showhelp %0
	exit /b 1
)

if "%configuration%" == "Release" (
	@rem OK
) else if "%configuration%" == "Debug" (
	@rem OK
) else (
	call :showhelp %0
	exit /b 1
)

set DEST_DIR=%~dp0..\%PLATFORM%\%CONFIGURATION%\
set EX_DIR=%~dp0..\installer\externals

set UNZIP_CMD=%~dp0..\tools\zip\unzip.bat
FOR /F "usebackq tokens=* delims=" %%A IN (`dir /b /s %EX_DIR%\*.zip`) DO (
    echo %%~fA
    echo %%~dpnA
    call %UNZIP_CMD% %%~fA %%~dpnA
)

: ---- Common ---- :
XCOPY /Y /I %EX_DIR%\bron412\*.txt %DEST_DIR%\License\bregonig\

: ---- Win32 ---- :
XCOPY /Y /I %EX_DIR%\bron412\bregonig.dll %DEST_DIR%\
XCOPY /Y /I %EX_DIR%\ctags-2018-09-16_e522743d-x86\ctags.exe %DEST_DIR%\
XCOPY /Y /I %EX_DIR%\ctags-2018-09-16_e522743d-x86\license\* %DEST_DIR%\license\ctags

: ---- x64 ---- :
XCOPY /Y /I %EX_DIR%\bron412\x64\bregonig.dll %DEST_DIR%\
XCOPY /Y /I %EX_DIR%\ctags-2018-09-16_e522743d-x64\ctags.exe %DEST_DIR%\
XCOPY /Y /I %EX_DIR%\ctags-2018-09-16_e522743d-x64\license\* %DEST_DIR%\license\ctags\
exit /b 0


@rem ------------------------------------------------------------------------------
@rem show help
@rem see http://orangeclover.hatenablog.com/entry/20101004/1286120668
@rem ------------------------------------------------------------------------------
:showhelp
@echo off
@echo usage
@echo    %~nx1 platform configuration
@echo.
@echo parameter
@echo    platform      : Win32   or x64
@echo    configuration : Release or Debug
@echo.
@echo example
@echo    %~nx1 Win32 Release
@echo    %~nx1 Win32 Debug
@echo    %~nx1 x64   Release
@echo    %~nx1 x64   Debug
exit /b 0

