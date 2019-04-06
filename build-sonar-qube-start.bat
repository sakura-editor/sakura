@rem to ensure hide variable SONAR_QUBE_TOKEN
call %~dp0build-sonar-qube-env.bat
if "%SonarScanner_MSBUILD%" == "" (
	echo do nothing
	exit /b 0
)

curl %URL_BUILDWRAPPER_ZIP% -o %BUILDWRAPPER_ZIP%
if errorlevel 1 (
	echo ERROR in download from %URL_BUILDWRAPPER_ZIP%
	exit /b 1
)
call %~dp0tools\zip\unzip.bat %BUILDWRAPPER_ZIP% %BUILDWRAPPER_DIR%

if exist .sonarqube rmdir /s /q .sonarqube

@rem to ensure hide variable SONAR_QUBE_TOKEN
@echo off
"%SonarScanner_MSBUILD%" begin                      ^
	/k:"%SONAR_QUBE_PROJECT%"                       ^
	/o:"%SONAR_QUBE_ORG%"                           ^
	/d:sonar.cfamily.build-wrapper-output=bw-output ^
	/d:sonar.sourceEncoding=UTF-8                   ^
	/d:sonar.host.url="https://sonarcloud.io"       ^
	/d:sonar.login="%SONAR_QUBE_TOKEN%"
if errorlevel 1 (
	echo ERROR in %SonarScanner_MSBUILD% begin errorlevel %errorlevel%
	exit /b 1
)
exit /b 0
