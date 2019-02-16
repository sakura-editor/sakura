@echo off
@setlocal enabledelayedexpansion
set platform=%1
set configuration=%2

if "%platform%" == "Win32" (
	@rem OK
) else if "%platform%" == "x64" (
	@rem OK
) else (
	call :showhelp %0
	exit /b 1
)

if "%configuration%" == "Release" (
	@rem OK
) else if "%configuration%" == "Debug" (
	@rem OK
) else (
	call :showhelp %0
	exit /b 1
)

if "%platform%" == "x64" (
	set ALPHA=1
) else (
	set ALPHA=0
)

call "%~dp0tools\find-tools.bat"
set ZIP_CMD=%~dp0tools\zip\zip.bat
set LIST_ZIP_CMD=%~dp0tools\zip\listzip.bat

@rem ----------------------------------------------------------------
@rem prepare environment variable
@rem ----------------------------------------------------------------
@echo checking APPVEYOR_ACCOUNT_NAME %APPVEYOR_ACCOUNT_NAME%
set BUILD_ACCOUNT=
if "%APPVEYOR_ACCOUNT_NAME%" == "sakuraeditor" (
	set BUILD_ACCOUNT=
) else if "%APPVEYOR_ACCOUNT_NAME%" == "" (
	set BUILD_ACCOUNT=
) else (
	set BUILD_ACCOUNT=%APPVEYOR_ACCOUNT_NAME%
)

@echo checking APPVEYOR_BUILD_NUMBER %APPVEYOR_BUILD_NUMBER%
if not "%APPVEYOR_BUILD_NUMBER%" == "" (
	set BUILD_NUMBER=build%APPVEYOR_BUILD_NUMBER%
) else (
	set BUILD_NUMBER=buildLocal
)

@echo checking APPVEYOR_REPO_TAG_NAME %APPVEYOR_REPO_TAG_NAME%
if not "%APPVEYOR_REPO_TAG_NAME%" == "" (
	@rem replace '/' with '_'
	set TEMP_NAME1=!APPVEYOR_REPO_TAG_NAME:/=_!
	@echo TEMP_NAME1 = !TEMP_NAME1!
	
	@rem replace ' ' with '_'
	set TEMP_NAME2=!TEMP_NAME1: =_!
	@echo TEMP_NAME2 = !TEMP_NAME2!

	@rem replace ' ' with '_'
	set TAG_NAME=tag-!TEMP_NAME2!
	@echo TAG_NAME = !TEMP_NAME2!
)

@echo checking APPVEYOR_PULL_REQUEST_NUMBER %APPVEYOR_PULL_REQUEST_NUMBER%
if not "%APPVEYOR_PULL_REQUEST_NUMBER%" == "" (
	set PR_NAME=PR%APPVEYOR_PULL_REQUEST_NUMBER%
)

@echo checking APPVEYOR_REPO_COMMIT %APPVEYOR_REPO_COMMIT%
if not "%APPVEYOR_REPO_COMMIT%" == "" (
	set SHORTHASH=%APPVEYOR_REPO_COMMIT:~0,8%
)

if "%ALPHA%" == "1" (
	set RELEASE_PHASE=alpha
) else (
	set RELEASE_PHASE=
)

@rem ----------------------------------------------------------------
@rem build BASENAME
@rem ----------------------------------------------------------------
set BASENAME=sakura

@echo adding BUILD_ACCOUNT
if not "%BUILD_ACCOUNT%" == "" (
	set BASENAME=%BASENAME%-%BUILD_ACCOUNT%
)
@echo BASENAME = %BASENAME%

@echo adding TAG_NAME
if not "%TAG_NAME%" == "" (
	set BASENAME=%BASENAME%-%TAG_NAME%
)
@echo BASENAME = %BASENAME%

@echo adding PR_NAME
if not "%PR_NAME%" == "" (
	set BASENAME=%BASENAME%-%PR_NAME%
)
@echo BASENAME = %BASENAME%

@echo adding BUILD_NUMBER
if not "%BUILD_NUMBER%" == "" (
	set BASENAME=%BASENAME%-%BUILD_NUMBER%
)
@echo BASENAME = %BASENAME%

@echo adding SHORTHASH
if not "%SHORTHASH%" == "" (
	set BASENAME=%BASENAME%-%SHORTHASH%
)
@echo BASENAME = %BASENAME%

@echo adding platform and configuration
set BASENAME=%BASENAME%-%platform%-%configuration%
@echo BASENAME = %BASENAME%

@echo adding RELEASE_PHASE
if not "%RELEASE_PHASE%" == "" (
	set BASENAME=%BASENAME%-%RELEASE_PHASE%
)
@echo BASENAME = %BASENAME%

@rem ---------------------- BASENAME ---------------------------------
@rem "sakura"
@rem BUILD_ACCOUNT: (option) APPVEYOR_ACCOUNT_NAME
@rem TAG_NAME     : (option) tag Name
@rem PR_NAME      : (option) PRxxx (xxx is a PR number)
@rem BUILD_NUMBER : (option) buildYYY or "buildLocal" (YYY is build number)
@rem SHORTHASH    : (option) hash or "buildLocal" (hash is leading 8 charactors)
@rem platform     : Platform ("Win32" or "x64")
@rem configuration: Configuration ("Debug" or "Release")
@rem RELEASE_PHASE: (option) "alpha" (x64 build only)
@rem ----------------------------------------------------------------

@rem ----------------------------------------------------------------
@rem build WORKDIR
@rem ----------------------------------------------------------------
set OUTFILE_LOG=%~dp0%BASENAME%-Log.zip
set OUTFILE_ASM=%~dp0%BASENAME%-Asm.zip
set OUTFILE_INST=%~dp0%BASENAME%-Installer.zip
set OUTFILE_EXE=%~dp0%BASENAME%-Exe.zip

@rem cleanup for local testing
del %OUTFILE_LOG% 2> NUL
del %OUTFILE_ASM% 2> NUL
del %OUTFILE_INST% 2> NUL
del %OUTFILE_EXE% 2> NUL

setlocal
cd /d "%~dp0%platform%\%configuration%\"
"%CMD_7Z%" a "%OUTFILE_EXE%" "@%~dp0artifactsList_exe.txt"
cd /d "%~dp0"
"%CMD_7Z%" a "%OUTFILE_LOG%" "@%~dp0artifactsList_log.txt"
endlocal

"%CMD_7Z%" a "%OUTFILE_ASM%" "%~dp0sakura\%platform%\%configuration%\*.asm"
"%CMD_7Z%" a "%OUTFILE_INST%" .\installer\Output-%platform%\*.exe .\installer\warning.txt

if "%ALPHA%" == "1" (
	"%CMD_7Z%" "%OUTFILE_EXE%" .\installer\warning-alpha.txt 
	"%CMD_7Z%" "%OUTFILE_INST%" .\installer\warning-alpha.txt 
)

exit /b 0

@rem ------------------------------------------------------------------------------
@rem show help
@rem see http://orangeclover.hatenablog.com/entry/20101004/1286120668
@rem ------------------------------------------------------------------------------
:showhelp
@echo off
@echo usage
@echo    %~nx1 platform configuration
@echo.
@echo parameter
@echo    platform      : Win32   or x64
@echo    configuration : Release or Debug
@echo.
@echo example
@echo    %~nx1 Win32 Release
@echo    %~nx1 Win32 Debug
@echo    %~nx1 x64   Release
@echo    %~nx1 x64   Debug
exit /b 0
