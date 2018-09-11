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
set BRANCH_NAME=PR-%PR_NUMBER%/%timestamp%

if "%PR_NUMBER%" == "" (
	echo usage  : get-PR.bat PR-Number
	echo example: get-PR.bat 1
	exit /b 1
)
@echo on
@echo fetching pull request %PR_NUMBER%
git fetch origin pull/%PR_NUMBER%/head:%BRANCH_NAME%

@echo checkout PR %PR_NUMBER% to branch %BRANCH_NAME%
git checkout %BRANCH_NAME%
