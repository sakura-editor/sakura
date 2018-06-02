@echo off
set PR_NUMBER=%1
set BRANCH_NAME=pull-request/%PR_NUMBER%

if "%PR_NUMBER%" == "" (
	echo usage  : get-PR.bat PR-Number
	echo example: get-PR.bat 1
	exit /b 1
)
@echo on
@echo fetching pull request %PR_NUMBER%
git fetch origin pull/%PR_NUMBER%/head:%BRANCH_NAME%

git checkout %BRANCH_NAME%
