@echo off

@echo =======================
@echo preBuild
@echo =======================

@echo.
@echo ---- HeaderMake ----
HeaderMake -in=..\sakura_core\Funccode_x.hsrc -out=..\sakura_core\Funccode_define.h -mode=define
HeaderMake -in=..\sakura_core\Funccode_x.hsrc -out=..\sakura_core\Funccode_enum.h -mode=enum -enum=EFunctionCode

@echo.
@echo ---- MakefileMake ----
MakefileMake -file=..\sakura_core\Makefile -dir=..\sakura_core

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

: Output githash.h
set GITHASH_H=..\sakura_core\githash.h
set GITHASH_H_TMP=%GITHASH_H%.tmp
type nul                                  > %GITHASH_H_TMP%
echo #pragma once                        >> %GITHASH_H_TMP%
if "%COMMITID%" == "" (
	type nul                                  >> %GITHASH_H_TMP%
) else (
	echo #define GIT_COMMIT_HASH "%COMMITID%" >> %GITHASH_H_TMP%
)
if "%SHORT_COMMITID%" == "" (
	type nul                                              >> %GITHASH_H_TMP%
) else (
	echo #define GIT_SHORT_COMMIT_HASH "%SHORT_COMMITID%" >> %GITHASH_H_TMP%
)
if "%GIT_URL%" == "" (
	type nul                                              >> %GITHASH_H_TMP%
) else (
	echo #define GIT_URL "%GIT_URL%"                      >> %GITHASH_H_TMP%
)

if "%APPVEYOR_URL%" == "" (
	type nul                                              >> %GITHASH_H_TMP%
) else (
	echo #define APPVEYOR_URL "%APPVEYOR_URL%"            >> %GITHASH_H_TMP%
)

if "%APPVEYOR_REPO_NAME%" == "" (
	type nul                                                          >> %GITHASH_H_TMP%
) else (
	echo #define APPVEYOR_REPO_NAME "%APPVEYOR_REPO_NAME%"            >> %GITHASH_H_TMP%
)

if "%APPVEYOR_ACCOUNT_NAME%" == "" (
	type nul                                                          >> %GITHASH_H_TMP%
) else (
	echo #define APPVEYOR_ACCOUNT_NAME "%APPVEYOR_ACCOUNT_NAME%"      >> %GITHASH_H_TMP%
)

if "%APPVEYOR_PROJECT_SLUG%" == "" (
	type nul                                                          >> %GITHASH_H_TMP%
) else (
	echo #define APPVEYOR_PROJECT_SLUG "%APPVEYOR_PROJECT_SLUG%"      >> %GITHASH_H_TMP%
)

if "%APPVEYOR_BUILD_VERSION%" == "" (
	type nul                                                          >> %GITHASH_H_TMP%
) else (
	echo #define APPVEYOR_BUILD_VERSION "%APPVEYOR_BUILD_VERSION%"    >> %GITHASH_H_TMP%
)

if "%APPVEYOR_BUILD_NUMBER%" == "" (
	type nul                                                          >> %GITHASH_H_TMP%
) else (
	echo #define APPVEYOR_BUILD_NUMBER     "%APPVEYOR_BUILD_NUMBER%"      >> %GITHASH_H_TMP%
	echo #define APPVEYOR_BUILD_NUMBER_INT  %APPVEYOR_BUILD_NUMBER%       >> %GITHASH_H_TMP%
)

if "%APPVEYOR_PULL_REQUEST_NUMBER%" == "" (
	type nul                                                                            >> %GITHASH_H_TMP%
) else (
	echo #define APPVEYOR_PULL_REQUEST_NUMBER     "%APPVEYOR_PULL_REQUEST_NUMBER%"      >> %GITHASH_H_TMP%
	echo #define APPVEYOR_PULL_REQUEST_NUMBER_INT  %APPVEYOR_PULL_REQUEST_NUMBER%       >> %GITHASH_H_TMP%
)

if "%GITHUB_COMMIT_URL%" == "" (
	type nul                                                          >> %GITHASH_H_TMP%
) else (
	echo #define GITHUB_COMMIT_URL             "%GITHUB_COMMIT_URL%"  >> %GITHASH_H_TMP%
)

if "%GITHUB_COMMIT_URL_PR_HEAD%" == "" (
	type nul                                                                    >> %GITHASH_H_TMP%
) else (
	echo #define GITHUB_COMMIT_URL_PR_HEAD     "%GITHUB_COMMIT_URL_PR_HEAD%"    >> %GITHASH_H_TMP%
)

if "%APPVEYOR_SHORTHASH%" == "" (
	type nul                                                                    >> %GITHASH_H_TMP%
) else (
	echo #define APPVEYOR_SHORTHASH             "%APPVEYOR_SHORTHASH%"          >> %GITHASH_H_TMP%
)

if "%APPVEYOR_SHORTHASH_PR_HEAD%" == "" (
	type nul                                                                    >> %GITHASH_H_TMP%
) else (
	echo #define APPVEYOR_SHORTHASH_PR_HEAD     "%APPVEYOR_SHORTHASH_PR_HEAD%"  >> %GITHASH_H_TMP%
)

fc %GITHASH_H% %GITHASH_H_TMP% 1>nul 2>&1
if "%ERRORLEVEL%" == "0" (
	del %GITHASH_H_TMP%
) else (
	if exist %GITHASH_H% del %GITHASH_H%
	move /y %GITHASH_H_TMP% %GITHASH_H%
	@echo %GITHASH_H% updated.
)

ENDLOCAL
rem exit 0
