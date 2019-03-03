@echo off

set OUT_DIR=%~1
if "%OUT_DIR%" == "" (
	set OUT_DIR=.
)

@rem replace '/' with '\'
set OUT_DIR=%OUT_DIR:/=\%

@echo.
@echo ---- Make githash.h ----

@rem ensure to be in the proper directory
pushd "%~dp0"

: Git enabled checking
set GIT_ENABLED=1
if not defined CMD_GIT call "%~dp0..\tools\find-tools.bat"
if not defined CMD_GIT (
	set GIT_ENABLED=0
	@echo NOTE: No git command
)
if not exist ..\.git (
	set GIT_ENABLED=0
	@echo NOTE: No .git directory
)

: Get git hash if git is enabled
if "%GIT_ENABLED%" == "1" (
	for /f "usebackq" %%s in (`"%CMD_GIT%" show -s --format^=%%h`) do (
		set GIT_SHORT_COMMIT_HASH=%%s
	)
	for /f "usebackq" %%s in (`"%CMD_GIT%" show -s --format^=%%H`) do (
		set GIT_COMMIT_HASH=%%s
	)
	for /f "usebackq" %%s in (`"%CMD_GIT%" config --get remote.origin.url`) do (
		set GIT_REMOTE_ORIGIN_URL=%%s
	)
) else (
	set GIT_SHORT_COMMIT_HASH=
	set GIT_COMMIT_HASH=
	set GIT_REMOTE_ORIGIN_URL=
)

@rem get back to the original directory
popd

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

@rem -- build APPVEYOR_BUILD_URL variable start ----
set APPVEYOR_BUILD_URL_VALID=1
if "%APPVEYOR_URL%"           == ""  set APPVEYOR_BUILD_URL_VALID=0
if "%APPVEYOR_ACCOUNT_NAME%"  == ""  set APPVEYOR_BUILD_URL_VALID=0
if "%APPVEYOR_PROJECT_SLUG%"  == ""  set APPVEYOR_BUILD_URL_VALID=0
if "%APPVEYOR_BUILD_VERSION%" == ""  set APPVEYOR_BUILD_URL_VALID=0
if "%APPVEYOR_BUILD_URL_VALID%" == "1" (
	set APPVEYOR_BUILD_URL=%APPVEYOR_URL%/project/%APPVEYOR_ACCOUNT_NAME%/%APPVEYOR_PROJECT_SLUG%/build/%APPVEYOR_BUILD_VERSION%
)
@rem -- build APPVEYOR_BUILD_URL variable end   ----

: Output githash.h
set GITHASH_H=%OUT_DIR%\githash.h
set GITHASH_H_TMP=%GITHASH_H%.tmp

@rem set SKIP_CREATE_GITHASH=1 to disable creation of githash.h
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

call :output_githash > "%GITHASH_H_TMP%"

fc "%GITHASH_H%" "%GITHASH_H_TMP%" 1>nul 2>&1
if not errorlevel 1 (
	del "%GITHASH_H_TMP%"
	@echo %GITHASH_H% was not updated.
) else (
	@echo GIT_SHORT_COMMIT_HASH : %GIT_SHORT_COMMIT_HASH%
	@echo GIT_COMMIT_HASH       : %GIT_COMMIT_HASH%
	@echo GIT_REMOTE_ORIGIN_URL : %GIT_REMOTE_ORIGIN_URL%
	@echo APPVEYOR_URL          : %APPVEYOR_URL%
	@echo APPVEYOR_REPO_NAME    : %APPVEYOR_REPO_NAME%
	@echo APPVEYOR_REPO_TAG_NAME: %APPVEYOR_REPO_TAG_NAME%
	@echo APPVEYOR_ACCOUNT_NAME : %APPVEYOR_ACCOUNT_NAME%
	@echo APPVEYOR_PROJECT_SLUG : %APPVEYOR_PROJECT_SLUG%
	@echo APPVEYOR_BUILD_VERSION: %APPVEYOR_BUILD_VERSION%
	@echo APPVEYOR_BUILD_NUMBER : %APPVEYOR_BUILD_NUMBER%
	@echo GITHUB_COMMIT_URL           : %GITHUB_COMMIT_URL%
	@echo GITHUB_COMMIT_URL_PR_HEAD   : %GITHUB_COMMIT_URL_PR_HEAD%
	@echo APPVEYOR_SHORTHASH          : %APPVEYOR_SHORTHASH%
	@echo APPVEYOR_SHORTHASH_PR_HEAD  : %APPVEYOR_SHORTHASH_PR_HEAD%
	@echo APPVEYOR_BUILD_URL          : %APPVEYOR_BUILD_URL%

	if exist "%GITHASH_H%" del "%GITHASH_H%"
	move /y "%GITHASH_H_TMP%" "%GITHASH_H%"
	@echo %GITHASH_H% was updated.
)

exit /b 0

:output_githash
echo /*! @file */
echo #pragma once
if "%GIT_SHORT_COMMIT_HASH%" == "" (
	echo // GIT_SHORT_COMMIT_HASH is not defined
) else (
	echo #define GIT_SHORT_COMMIT_HASH "%GIT_SHORT_COMMIT_HASH%"
)
if "%GIT_COMMIT_HASH%" == "" (
	echo // GIT_COMMIT_HASH is not defined
) else (
	echo #define GIT_COMMIT_HASH "%GIT_COMMIT_HASH%"
)
if "%GIT_REMOTE_ORIGIN_URL%" == "" (
	echo // GIT_REMOTE_ORIGIN_URL is not defined
) else (
	echo #define GIT_REMOTE_ORIGIN_URL "%GIT_REMOTE_ORIGIN_URL%"
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

@rem enable 'dev version' macro which will be disabled on release branches
echo #define APPVEYOR_DEV_VERSION

if "%APPVEYOR_REPO_TAG_NAME%" == "" (
	echo // APPVEYOR_REPO_TAG_NAME is not defined
) else (
	echo #define APPVEYOR_REPO_TAG_NAME "%APPVEYOR_REPO_TAG_NAME%"
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
	echo // APPVEYOR_BUILD_NUMBER_LABEL is not defined
) else (
	echo #define APPVEYOR_BUILD_NUMBER     "%APPVEYOR_BUILD_NUMBER%"
	echo #define APPVEYOR_BUILD_NUMBER_INT  %APPVEYOR_BUILD_NUMBER%
	echo #define APPVEYOR_BUILD_NUMBER_LABEL "Build %APPVEYOR_BUILD_NUMBER%"
)

if "%APPVEYOR_PULL_REQUEST_NUMBER%" == "" (
	echo // APPVEYOR_PULL_REQUEST_NUMBER     is not defined
	echo // APPVEYOR_PULL_REQUEST_NUMBER_INT is not defined
	echo // APPVEYOR_PR_NUMBER_LABEL         is not defined
) else (
	echo #define APPVEYOR_PULL_REQUEST_NUMBER     "%APPVEYOR_PULL_REQUEST_NUMBER%"
	echo #define APPVEYOR_PULL_REQUEST_NUMBER_INT  %APPVEYOR_PULL_REQUEST_NUMBER%
	echo #define APPVEYOR_PR_NUMBER_LABEL         "PR %APPVEYOR_PULL_REQUEST_NUMBER%"
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

exit /b 0
