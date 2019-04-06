call %~dp0build-sonar-qube-env.bat
if "%SonarScanner_MSBUILD%" == "" (
	echo do nothing
	exit /b 0
)

@rem to ensure hide variable SONAR_QUBE_TOKEN
@echo off
"%SonarScanner_MSBUILD%" end /d:sonar.login="%SONAR_QUBE_TOKEN%"
if errorlevel 1 (
	echo ERROR in %SonarScanner_MSBUILD% end errorlevel %errorlevel%
	exit /b 1
)
exit /b 0
