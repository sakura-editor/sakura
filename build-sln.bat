@echo off
set platform=%1
set configuration=%2
set SonarScanner_MSBUILD=C:\ProgramData\chocolatey\bin\SonarScanner.MSBuild.exe

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

@echo off
if "%SONAR_QUBE%" == "Yes" (
	call :BuildSonarQube
) else (
	call :BuildNormal
)
if errorlevel 1 (
	echo ERROR build %errorlevel%
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

@rem ------------------------------------------------------------------------------
@rem build normally
@rem ------------------------------------------------------------------------------
:BuildNormal
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
@rem build with SonarQube
@rem ------------------------------------------------------------------------------
:BuildSonarQube

@rem to ensure hide variable SONAR_QUBE_TOKEN
@echo off
if "%SONAR_QUBE_TOKEN%" == "" (
	@echo SONAR_QUBE_TOKEN is not defined. Abort without building.
	exit /b 0
)

if "%SONAR_QUBE_ORG%" == "" (
	@echo SONAR_QUBE_ORG is not defined. Abort without building.
	exit /b 0
)

if "%SONAR_QUBE_PROJECT%" == "" (
	@echo SONAR_QUBE_PROJECT is not defined. Abort without building.
	exit /b 0
)

set URL_BUILDWRAPPER_ZIP=https://sonarcloud.io/static/cpp/build-wrapper-win-x86.zip
set BUILDWRAPPER_ZIP=.\build-wrapper-win-x86.zip
set BUILDWRAPPER_DIR=build-wrapper-win-x86
set BUILDWRAPPER_EXE=%BUILDWRAPPER_DIR%\build-wrapper-win-x86\build-wrapper-win-x86-64.exe

curl %URL_BUILDWRAPPER_ZIP% -o %BUILDWRAPPER_ZIP%
if errorlevel 1 (
	echo ERROR in download from %URL_BUILDWRAPPER_ZIP%
	exit /b 1
)
call tools\zip\unzip.bat %BUILDWRAPPER_ZIP% %BUILDWRAPPER_DIR%

if exist .sonarqube rmdir /s /q .sonarqube

@rem to ensure hide variable SONAR_QUBE_TOKEN
@echo off
"%SonarScanner_MSBUILD%" begin                      ^
	/k:"%SONAR_QUBE_PROJECT%"                       ^
	/d:sonar.organization="%SONAR_QUBE_ORG%"        ^
	/d:sonar.cfamily.build-wrapper-output=bw-output ^
	/d:sonar.sourceEncoding=UTF-8                   ^
	/d:sonar.host.url="https://sonarcloud.io"       ^
	/d:sonar.login="%SONAR_QUBE_TOKEN%"
if errorlevel 1 (
	echo ERROR in %SonarScanner_MSBUILD% begin errorlevel %errorlevel%
	exit /b 1
)

echo "%BUILDWRAPPER_EXE%" --out-dir bw-output "%CMD_MSBUILD%"  %SLN_FILE% /p:Platform=%platform% /p:Configuration=%configuration%  /t:"Rebuild"
     "%BUILDWRAPPER_EXE%" --out-dir bw-output "%CMD_MSBUILD%"  %SLN_FILE% /p:Platform=%platform% /p:Configuration=%configuration%  /t:"Rebuild"
if errorlevel 1 (
	exit /b 1
)

@rem to ensure hide variable SONAR_QUBE_TOKEN
@echo off
"%SonarScanner_MSBUILD%" end /d:sonar.login="%SONAR_QUBE_TOKEN%"
if errorlevel 1 (
	echo ERROR in %SonarScanner_MSBUILD% end errorlevel %errorlevel%
	exit /b 1
)
exit /b 0
