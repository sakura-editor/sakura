@echo off
set platform=%1
set configuration=%2

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

@rem find a NuGet CLI.
if not defined CMD_NUGET (
	where nuget > NUL
	if errorlevel 1 (
		set CMD_NUGET=%~dp0externals\nuget\nuget.exe
	) else (
		set CMD_NUGET=nuget.exe
	)
)

@rem restore NuGet packages.
@echo "%CMD_NUGET%" restore -PackagesDirectory packages %~dp0tests\unittests\packages.config
      "%CMD_NUGET%" restore -PackagesDirectory packages %~dp0tests\unittests\packages.config

@rem check NuGet's restore results.
if errorlevel 1 (
   @echo NuGet.exe^(CMD_NUGET^) makes some error^(%errorlevel%^).
   exit /b 1
)

if not defined CMD_MSBUILD call %~dp0tools\find-tools.bat
if not defined CMD_MSBUILD (
	echo msbuild.exe was not found.
	exit /b 1
)

set SLN_FILE=sakura.sln

@rem https://www.appveyor.com/docs/environment-variables/

if "%APPVEYOR%"=="True" (
    set EXTRA_CMD=/verbosity:minimal /logger:"%ProgramFiles%\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
) else (
    set EXTRA_CMD=
)

set LOG_FILE=msbuild-%platform%-%configuration%.log
@rem https://msdn.microsoft.com/ja-jp/library/ms171470.aspx
set LOG_OPTION=/flp:logfile=%LOG_FILE%

@echo "%CMD_MSBUILD%" %SLN_FILE% /p:Platform=%platform% /p:Configuration=%configuration%  /t:"Build" %EXTRA_CMD% %LOG_OPTION%
      "%CMD_MSBUILD%" %SLN_FILE% /p:Platform=%platform% /p:Configuration=%configuration%  /t:"Build" %EXTRA_CMD% %LOG_OPTION%
if errorlevel 1 (
	echo ERROR in msbuild.exe errorlevel %errorlevel%
	exit /b 1
)

@echo call parse-buildlog.bat %LOG_FILE%
      call parse-buildlog.bat %LOG_FILE%
if errorlevel 1 (
	echo ERROR in parse-buildlog.bat errorlevel %errorlevel%
	exit /b 1
)
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
