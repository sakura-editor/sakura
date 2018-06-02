@echo off
set PR_NUMBER=%1

if "%PR_NUMBER%" == "" (
	echo usage  : get-PR.bat PR-Number
	echo example: get-PR.bat 1
	exit /b 1
)
@echo on
@echo fetching pull request %PR_NUMBER%
git fetch origin pull/%PR_NUMBER%/head:pull-request-%PR_NUMBER%

@echo checkout the local branch for PR %PR_NUMBER%
git checkout pull-request-%PR_NUMBER%
