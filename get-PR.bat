@echo off

set yyyy=%date:~0,4%
set mm=%date:~5,2%
set dd=%date:~8,2%
set time2=%time: =0%
set hh=%time2:~0,2%
set mn=%time2:~3,2%
set ss=%time2:~6,2%
set timestamp=%yyyy%%mm%%dd%-%hh%%mn%%ss%
set PR_NUMBER=%1
set REMOTE_NAME=%2
set BRANCH_NAME=PR-%PR_NUMBER%/%timestamp%

if "%PR_NUMBER%" == "" (
	echo usage   : get-PR.bat PR-Number [remote name]
	echo example1: get-PR.bat 1
	echo example2: get-PR.bat 1 upstream
	echo example3: get-PR.bat 1 origin
	echo.
	echo [remote name]
	echo 1. If "remote name" is valid, it is used as the origin of PR.
	echo 2. If "remote name" is invalid and env SAKURA_EDITOR_REMOTE_NAME is valid, 
	echo    env SAKURA_EDITOR_REMOTE_NAME is used as the origin of PR.
	echo 3. Otherwise, "origin" is used as the origin of PR.
	exit /b 1
)

if not "%REMOTE_NAME%" == "" (
	@rem nothing to do
) else if not "%SAKURA_EDITOR_REMOTE_NAME%" == "" (
	set REMOTE_NAME=%SAKURA_EDITOR_REMOTE_NAME%
) else (
	set REMOTE_NAME=origin
)

@echo fetching pull request %PR_NUMBER% from %REMOTE_NAME%
echo git fetch %REMOTE_NAME% pull/%PR_NUMBER%/head:%BRANCH_NAME%
     git fetch %REMOTE_NAME% pull/%PR_NUMBER%/head:%BRANCH_NAME% || exit /b 1

@echo checkout PR %PR_NUMBER% to branch %BRANCH_NAME%
@echo git checkout %BRANCH_NAME%
      git checkout %BRANCH_NAME% || exit /b 1

exit /b 0
