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
set SonarScanner_MSBUILD=C:\ProgramData\chocolatey\bin\SonarScanner.MSBuild.exe
exit /b 0
