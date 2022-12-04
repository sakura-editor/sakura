@echo off

set OUT_DIR=%~1
if "%OUT_DIR%" == "" (
	set OUT_DIR=.
)

@rem replace '/' with '\'
set OUT_DIR=%OUT_DIR:/=\%

@echo.
@echo ---- Make githash.h ----
call :set_git_variables
call :set_repo_and_pr_variables
call :set_ci_build_url
call :update_output_githash
exit /b 0

:set_git_variables
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

	@rem clear variable in advance
	set GIT_SHORT_COMMIT_HASH=
	set GIT_COMMIT_HASH=
	set GIT_REMOTE_ORIGIN_URL=
	set GIT_TAG_NAME=

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
		
		@rem get tag of 'HEAD'
		@rem Ignore errors when `HEAD` is not tagged.
		@rem https://superuser.com/questions/743735/suppressing-errors-from-an-embedded-command-in-a-batch-file-for-loop
		for /f "usebackq" %%s in (`"%CMD_GIT%" describe --tags --contains 2^>nul`) do (
			set GIT_TAG_NAME=%%s
		)

		:: Gitリポジトリの累積コミット数(取れない場合は0)
		call :set_build_version
	) else (
		set GIT_SHORT_COMMIT_HASH=
		set GIT_COMMIT_HASH=
		set GIT_REMOTE_ORIGIN_URL=
		set GIT_TAG_NAME=
		set BUILD_VERSION=0
	)

	:: ビルド環境の名前が未定義なら local とする
	if not defined BUILD_ENV_NAME (
		set BUILD_ENV_NAME=Local
	)

	@rem get back to the original directory
	popd

	exit /b 0

:set_build_version
	if not "%GIT_ENABLED%" == "1" exit /b 0
	if defined BUILD_VERSION exit /b 0
	for /f "usebackq" %%s in (`"%CMD_GIT%" rev-list --count --no-merges @`) do (
		set BUILD_VERSION=%%s
	)
	exit /b 0

:set_repo_and_pr_variables
	if defined APPVEYOR_REPO_NAME (
		set CI_REPO_NAME=%APPVEYOR_REPO_NAME%
	) else if defined BUILD_REPOSITORY_NAME (
		set CI_REPO_NAME=%BUILD_REPOSITORY_NAME%
	)

	if defined APPVEYOR_ACCOUNT_NAME (
		set CI_ACCOUNT_NAME=%APPVEYOR_ACCOUNT_NAME%
	) else if defined BUILD_DEFINITIONNAME (
		set CI_ACCOUNT_NAME=%BUILD_DEFINITIONNAME%
	)

	@rem ----------------------------------------------------------------------------------------------------------
	@rem Be aware the following rules.
	@rem BUILD_BUILDID     (at azure pipeline) is the counterpart of APPVEYOR_BUILD_NUMBER  (at appveyor).
	@rem BUILD_BUILDNUMBER (at azure pipeline) is the counterpart of APPVEYOR_BUILD_VERSION (at appveyor).
	@rem ----------------------------------------------------------------------------------------------------------
	@rem This is super confusing.
	@rem BUILD_BUILDNUMBER (at azure pipeline) and APPVEYOR_BUILD_NUMBER (at appveyor) are different information.
	@rem ----------------------------------------------------------------------------------------------------------
	if defined APPVEYOR_BUILD_NUMBER (
		@rem APPVEYOR_BUILD_NUMBER=1624
		set CI_BUILD_NUMBER=%APPVEYOR_BUILD_NUMBER%
	) else if defined BUILD_BUILDID (
		@rem example BUILD_BUILDID=672
		set CI_BUILD_NUMBER=%BUILD_BUILDID%
	)

	if defined APPVEYOR_BUILD_VERSION (
		@rem APPVEYOR_BUILD_VERSION=1.0.1624
		set CI_BUILD_VERSION=%APPVEYOR_BUILD_VERSION%
	) else if defined BUILD_BUILDNUMBER (
		@rem example BUILD_BUILDNUMBER=20200205.4
		set CI_BUILD_VERSION=%BUILD_BUILDNUMBER%
	)

	if defined APPVEYOR_PULL_REQUEST_NUMBER (
		set GITHUB_PR_NUMBER=%APPVEYOR_PULL_REQUEST_NUMBER%
	) else if defined SYSTEM_PULLREQUEST_PULLREQUESTNUMBER (
		set GITHUB_PR_NUMBER=%SYSTEM_PULLREQUEST_PULLREQUESTNUMBER%
	)

	if defined APPVEYOR_PULL_REQUEST_HEAD_COMMIT (
		set GITHUB_PR_HEAD_SHORT_COMMIT=%APPVEYOR_PULL_REQUEST_HEAD_COMMIT:~0,8%
		set GITHUB_PR_HEAD_COMMIT=%APPVEYOR_PULL_REQUEST_HEAD_COMMIT%
	) else if defined SYSTEM_PULLREQUEST_SOURCECOMMITID (
		set GITHUB_PR_HEAD_SHORT_COMMIT=%SYSTEM_PULLREQUEST_SOURCECOMMITID:~0,8%
		set GITHUB_PR_HEAD_COMMIT=%SYSTEM_PULLREQUEST_SOURCECOMMITID%
	)

	if "%BUILD_REPOSITORY_PROVIDER%"=="GitHub" (
		set GITHUB_ON=1
	)
	if "%APPVEYOR_REPO_PROVIDER%"=="gitHub" (
		set GITHUB_ON=1
	)

	set PREFIX_GITHUB=https://github.com
	if "%GITHUB_ON%" == "1" (
		set "GITHUB_COMMIT_URL=%PREFIX_GITHUB%/%CI_REPO_NAME%/commit/%GIT_COMMIT_HASH%"
		@rem Not Pull Request
		if "%GITHUB_PR_NUMBER%" == "" (
			@rem No PR
		) else (
			@rem PR URL
			set "GITHUB_PR_HEAD_URL=%PREFIX_GITHUB%/%CI_REPO_NAME%/pull/%GITHUB_PR_NUMBER%/commits/%GITHUB_PR_HEAD_COMMIT%"
		)
	)
	exit /b 0

:set_ci_build_url
	call :set_ci_build_url_for_appveyor
	call :set_ci_build_url_for_azurepipelines
	call :set_ci_build_url_for_githubactions
	exit /b 0

:set_ci_build_url_for_appveyor
	if not defined APPVEYOR               exit /b 0
	if not defined APPVEYOR_URL           exit /b 0
	if not defined APPVEYOR_ACCOUNT_NAME  exit /b 0
	if not defined APPVEYOR_PROJECT_SLUG  exit /b 0
	if not defined APPVEYOR_BUILD_VERSION exit /b 0
	set BUILD_ENV_NAME=Appveyor
	set CI_BUILD_URL=%APPVEYOR_URL%/project/%APPVEYOR_ACCOUNT_NAME%/%APPVEYOR_PROJECT_SLUG%/build/%APPVEYOR_BUILD_VERSION%
	exit /b 0

:set_ci_build_url_for_azurepipelines
	if not defined SYSTEM_TEAMFOUNDATIONSERVERURI exit /b 0
	if not defined SYSTEM_TEAMPROJECT             exit /b 0
	if not defined BUILD_BUILDID                  exit /b 0
	set BUILD_ENV_NAME=AZP
	set CI_BUILD_URL=%SYSTEM_TEAMFOUNDATIONSERVERURI%%SYSTEM_TEAMPROJECT%/_build/results?buildId=%BUILD_BUILDID%
	exit /b 0

:set_ci_build_url_for_githubactions
	if not defined GITHUB_ACTIONS exit /b 0
	if not "%GITHUB_ACTIONS%" == "true" exit /b 0
	set BUILD_ENV_NAME=GHA
	exit /b 0

:update_output_githash
	@rem update githash.h if necessary
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
		@echo GIT_TAG_NAME          : %GIT_TAG_NAME%
		@echo GIT_SHORT_COMMIT_HASH : %GIT_SHORT_COMMIT_HASH%
		@echo GIT_COMMIT_HASH       : %GIT_COMMIT_HASH%
		@echo GIT_REMOTE_ORIGIN_URL : %GIT_REMOTE_ORIGIN_URL%
		@echo.
		@echo CI_ACCOUNT_NAME       : %CI_ACCOUNT_NAME%
		@echo CI_REPO_NAME          : %CI_REPO_NAME%
		@echo CI_BUILD_VERSION      : %CI_BUILD_VERSION%
		@echo CI_BUILD_NUMBER       : %CI_BUILD_NUMBER%
		@echo CI_BUILD_URL          : %CI_BUILD_URL%
		@echo.
		@echo GITHUB_COMMIT_URL           : %GITHUB_COMMIT_URL%
		@echo GITHUB_PR_NUMBER            : %GITHUB_PR_NUMBER%
		@echo GITHUB_PR_HEAD_SHORT_COMMIT : %GITHUB_PR_HEAD_SHORT_COMMIT%
		@echo GITHUB_PR_HEAD_COMMIT       : %GITHUB_PR_HEAD_COMMIT%
		@echo GITHUB_PR_HEAD_URL          : %GITHUB_PR_HEAD_URL%
		@echo.
		@echo APPVEYOR_URL          : %APPVEYOR_URL%
		@echo APPVEYOR_PROJECT_SLUG : %APPVEYOR_PROJECT_SLUG%
		@echo.
		@echo BUILD_ENV_NAME        : %BUILD_ENV_NAME%
		@echo BUILD_VERSION         : %BUILD_VERSION%
		@echo.

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
	if "%GIT_TAG_NAME%" == "" (
		echo // GIT_TAG_NAME is not defined
	) else (
		echo #define GIT_TAG_NAME "%GIT_TAG_NAME%"
	)

	if "%CI_REPO_NAME%" == "" (
		echo // CI_REPO_NAME is not defined
	) else (
		echo #define CI_REPO_NAME "%CI_REPO_NAME%"
	)

	@rem enable 'dev version' macro which will be disabled on release branches
	@rem echo #define DEV_VERSION

	if "%CI_ACCOUNT_NAME%" == "" (
		echo // CI_ACCOUNT_NAME is not defined
	) else (
		echo #define CI_ACCOUNT_NAME "%CI_ACCOUNT_NAME%"
	)

	if "%CI_BUILD_VERSION%" == "" (
		echo // CI_BUILD_VERSION is not defined
	) else (
		echo #define CI_BUILD_VERSION "%CI_BUILD_VERSION%"
	)

	if "%CI_BUILD_NUMBER%" == "" (
		echo // CI_BUILD_NUMBER     is not defined
		echo // CI_BUILD_NUMBER_INT is not defined
		echo // CI_BUILD_NUMBER_LABEL is not defined
	) else (
		echo #define CI_BUILD_NUMBER             "%CI_BUILD_NUMBER%"
		echo #define CI_BUILD_NUMBER_INT          %CI_BUILD_NUMBER%
		echo #define CI_BUILD_NUMBER_LABEL "Build %CI_BUILD_NUMBER%"
	)

	if "%GITHUB_PR_NUMBER%" == "" (
		echo // GITHUB_PR_NUMBER       is not defined
		echo // GITHUB_PR_NUMBER_INT   is not defined
		echo // GITHUB_PR_NUMBER_LABEL is not defined
	) else (
		echo #define GITHUB_PR_NUMBER                 "%GITHUB_PR_NUMBER%"
		echo #define GITHUB_PR_NUMBER_INT              %GITHUB_PR_NUMBER%
		echo #define GITHUB_PR_NUMBER_LABEL        "PR %GITHUB_PR_NUMBER%"
	)

	if "%GITHUB_COMMIT_URL%" == "" (
		echo // GITHUB_COMMIT_URL is not defined
	) else (
		echo #define GITHUB_COMMIT_URL             "%GITHUB_COMMIT_URL%"
	)

	if "%GITHUB_PR_HEAD_URL%" == "" (
		echo // GITHUB_PR_HEAD_URL is not defined
	) else (
		echo #define GITHUB_PR_HEAD_URL     "%GITHUB_PR_HEAD_URL%"
	)

	if "%GITHUB_PR_HEAD_COMMIT%" == "" (
		echo // GITHUB_PR_HEAD_COMMIT is not defined
	) else (
		echo #define GITHUB_PR_HEAD_COMMIT         "%GITHUB_PR_HEAD_COMMIT%"
	)

	if "%GITHUB_PR_HEAD_SHORT_COMMIT%" == "" (
		echo // GITHUB_PR_HEAD_SHORT_COMMIT is not defined
	) else (
		echo #define GITHUB_PR_HEAD_SHORT_COMMIT   "%GITHUB_PR_HEAD_SHORT_COMMIT%"
	)

	if "%CI_BUILD_URL%" == "" (
		echo // CI_BUILD_URL is not defined
	) else (
		echo #define CI_BUILD_URL                  "%CI_BUILD_URL%"
	)

	echo // APPVEYOR specific variables

	if "%APPVEYOR_URL%" == "" (
		echo // APPVEYOR_URL is not defined
	) else (
		echo #define APPVEYOR_URL "%APPVEYOR_URL%"
	)

	if "%APPVEYOR_PROJECT_SLUG%" == "" (
		echo // APPVEYOR_PROJECT_SLUG is not defined
	) else (
		echo #define APPVEYOR_PROJECT_SLUG "%APPVEYOR_PROJECT_SLUG%"
	)
	echo // APPVEYOR specific variables end
	echo //

	echo #define BUILD_ENV_NAME "%BUILD_ENV_NAME%"
	echo #define BUILD_VERSION %BUILD_VERSION%

	exit /b 0
