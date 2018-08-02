@echo off

SETLOCAL

set OUT_DIR=%1

@echo.
@echo ---- Make githash.h ----
: Git enabled checking
set GIT_ENABLED=1
where git 1>nul 2>&1
if not "%ERRORLEVEL%" == "0" (
	set GIT_ENABLED=0
	@echo NOTE: No git command
)
if not exist ..\.git (
	set GIT_ENABLED=0
	@echo NOTE: No .git directory
)

: Get git hash if git is enabled
if "%GIT_ENABLED%" == "1" (
	for /f "usebackq" %%s in (`git show -s --format^=%%H`) do (
		set COMMITID=%%s
	)
	for /f "usebackq" %%s in (`git show -s --format^=%%h`) do (
		set SHORT_COMMITID=%%s
	)
	for /f "usebackq" %%s in (`git config --get remote.origin.url`) do (
		set GIT_URL=%%s
	)
) else (
	set SHORT_COMMITID=
	set COMMITID=
	set GIT_URL=
)

set PREFIX_GITHUB=https://github.com
if "%APPVEYOR_REPO_PROVIDER%" == "gitHub" (
	set GITHUB_COMMIT_URL=%PREFIX_GITHUB%/%APPVEYOR_REPO_NAME%/commit/%APPVEYOR_REPO_COMMIT%
	@rem Not Pull Request
	if "%APPVEYOR_PULL_REQUEST_NUMBER%" == "" (
		@rem No PR
	) else (
		@rem PR URL
		set GITHUB_COMMIT_URL_PR_HEAD=%PREFIX_GITHUB%/%APPVEYOR_REPO_NAME%/pull/%APPVEYOR_PULL_REQUEST_NUMBER%/commits/%APPVEYOR_PULL_REQUEST_HEAD_COMMIT%
	)
)

if not "%APPVEYOR_REPO_COMMIT%" == "" (
	set APPVEYOR_SHORTHASH=%APPVEYOR_REPO_COMMIT:~0,8%
) else (
	set APPVEYOR_SHORTHASH=
)
if not "%APPVEYOR_PULL_REQUEST_HEAD_COMMIT%" == "" (
	set APPVEYOR_SHORTHASH_PR_HEAD=%APPVEYOR_PULL_REQUEST_HEAD_COMMIT:~0,8%
) else (
	set APPVEYOR_SHORTHASH_PR_HEAD=
)

@echo SHORT_COMMITID: %SHORT_COMMITID%
@echo COMMITID: %COMMITID%
@echo GIT_URL: %GIT_URL%
@echo APPVEYOR_URL          : %APPVEYOR_URL%
@echo APPVEYOR_REPO_NAME    : %APPVEYOR_REPO_NAME%
@echo APPVEYOR_ACCOUNT_NAME : %APPVEYOR_ACCOUNT_NAME%
@echo APPVEYOR_PROJECT_SLUG : %APPVEYOR_PROJECT_SLUG%
@echo APPVEYOR_BUILD_VERSION: %APPVEYOR_BUILD_VERSION%
@echo APPVEYOR_BUILD_NUMBER : %APPVEYOR_BUILD_NUMBER%
@echo GITHUB_COMMIT_URL           : %GITHUB_COMMIT_URL%
@echo GITHUB_COMMIT_URL_PR_HEAD   : %GITHUB_COMMIT_URL_PR_HEAD%
@echo APPVEYOR_SHORTHASH          : %APPVEYOR_SHORTHASH%
@echo APPVEYOR_SHORTHASH_PR_HEAD  : %APPVEYOR_SHORTHASH_PR_HEAD%

@rem -- build APPVEYOR_BUILD_URL variable start ----
set APPVEYOR_BUILD_URL_VALID=1
if "%APPVEYOR_URL%"           == ""  set APPVEYOR_BUILD_URL_VALID=0
if "%APPVEYOR_ACCOUNT_NAME%"  == ""  set APPVEYOR_BUILD_URL_VALID=0
if "%APPVEYOR_PROJECT_SLUG%"  == ""  set APPVEYOR_BUILD_URL_VALID=0
if "%APPVEYOR_BUILD_VERSION%" == ""  set APPVEYOR_BUILD_URL_VALID=0
if "%APPVEYOR_BUILD_URL_VALID%" == "1" (
	set APPVEYOR_BUILD_URL=%APPVEYOR_URL%/project/%APPVEYOR_ACCOUNT_NAME%/%APPVEYOR_PROJECT_SLUG%/build/%APPVEYOR_BUILD_VERSION%
)
@echo APPVEYOR_BUILD_URL          : %APPVEYOR_BUILD_URL%
@rem -- build APPVEYOR_BUILD_URL variable end   ----

: Output githash.h
set GITHASH_H=%OUT_DIR%\githash.h
set GITHASH_H_TMP=%GITHASH_H%.tmp

@rem check if skip creation of %GITHASH_H%
set VALID_CREATE_GITHASH=1
if "%SKIP_CREATE_GITHASH%" == "1" (
	set VALID_CREATE_GITHASH=0
)
if not exist "%GITHASH_H%" (
	set VALID_CREATE_GITHASH=1
)

if "%VALID_CREATE_GITHASH%" == "0" (
	@echo skip creation of %GITHASH_H%
	exit /b 0
)

call :output_githash > %GITHASH_H_TMP%

fc %GITHASH_H% %GITHASH_H_TMP% 1>nul 2>&1
if "%ERRORLEVEL%" == "0" (
	del %GITHASH_H_TMP%
	@echo %GITHASH_H% was not updated.
) else (
	if exist %GITHASH_H% del %GITHASH_H%
	move /y %GITHASH_H_TMP% %GITHASH_H%
	@echo %GITHASH_H% was updated.
)

ENDLOCAL
exit /b

:output_githash
echo #pragma once
if "%COMMITID%" == "" (
	echo // GIT_COMMIT_HASH is not defined
) else (
	echo #define GIT_COMMIT_HASH "%COMMITID%"
)
if "%SHORT_COMMITID%" == "" (
	echo // GIT_SHORT_COMMIT_HASH is not defined
) else (
	echo #define GIT_SHORT_COMMIT_HASH "%SHORT_COMMITID%"
)
if "%GIT_URL%" == "" (
	echo // GIT_URL is not defined
) else (
	echo #define GIT_URL "%GIT_URL%"
)

if "%APPVEYOR_URL%" == "" (
	echo // APPVEYOR_URL is not defined
) else (
	echo #define APPVEYOR_URL "%APPVEYOR_URL%"
)

if "%APPVEYOR_REPO_NAME%" == "" (
	echo // APPVEYOR_REPO_NAME is not defined
) else (
	echo #define APPVEYOR_REPO_NAME "%APPVEYOR_REPO_NAME%"
)

if "%APPVEYOR_ACCOUNT_NAME%" == "" (
	echo // APPVEYOR_ACCOUNT_NAME is not defined
) else (
	echo #define APPVEYOR_ACCOUNT_NAME "%APPVEYOR_ACCOUNT_NAME%"
)

if "%APPVEYOR_PROJECT_SLUG%" == "" (
	echo // APPVEYOR_PROJECT_SLUG is not defined
) else (
	echo #define APPVEYOR_PROJECT_SLUG "%APPVEYOR_PROJECT_SLUG%"
)

if "%APPVEYOR_BUILD_VERSION%" == "" (
	echo // APPVEYOR_BUILD_VERSION is not defined
) else (
	echo #define APPVEYOR_BUILD_VERSION "%APPVEYOR_BUILD_VERSION%"
)

if "%APPVEYOR_BUILD_NUMBER%" == "" (
	echo // APPVEYOR_BUILD_NUMBER     is not defined
	echo // APPVEYOR_BUILD_NUMBER_INT is not defined
) else (
	echo #define APPVEYOR_BUILD_NUMBER     "%APPVEYOR_BUILD_NUMBER%"
	echo #define APPVEYOR_BUILD_NUMBER_INT  %APPVEYOR_BUILD_NUMBER%
)

if "%APPVEYOR_PULL_REQUEST_NUMBER%" == "" (
	echo // APPVEYOR_PULL_REQUEST_NUMBER     is not defined
	echo // APPVEYOR_PULL_REQUEST_NUMBER_INT is not defined
) else (
	echo #define APPVEYOR_PULL_REQUEST_NUMBER     "%APPVEYOR_PULL_REQUEST_NUMBER%"
	echo #define APPVEYOR_PULL_REQUEST_NUMBER_INT  %APPVEYOR_PULL_REQUEST_NUMBER%
)

if "%GITHUB_COMMIT_URL%" == "" (
	echo // GITHUB_COMMIT_URL is not defined
) else (
	echo #define GITHUB_COMMIT_URL             "%GITHUB_COMMIT_URL%"
)

if "%GITHUB_COMMIT_URL_PR_HEAD%" == "" (
	echo // GITHUB_COMMIT_URL_PR_HEAD is not defined
) else (
	echo #define GITHUB_COMMIT_URL_PR_HEAD     "%GITHUB_COMMIT_URL_PR_HEAD%"
)

if "%APPVEYOR_SHORTHASH%" == "" (
	echo // APPVEYOR_SHORTHASH is not defined
) else (
	echo #define APPVEYOR_SHORTHASH             "%APPVEYOR_SHORTHASH%"
)

if "%APPVEYOR_SHORTHASH_PR_HEAD%" == "" (
	echo // APPVEYOR_SHORTHASH_PR_HEAD is not defined
) else (
	echo #define APPVEYOR_SHORTHASH_PR_HEAD     "%APPVEYOR_SHORTHASH_PR_HEAD%"
)

if "%APPVEYOR_BUILD_URL%" == "" (
	echo // APPVEYOR_BUILD_URL is not defined
) else (
	echo #define APPVEYOR_BUILD_URL             "%APPVEYOR_BUILD_URL%"
)

exit /b
