set platform=%1
set configuration=%2
@echo off
if "%platform%" == "x64" (
	set ALPHA=1
) else (
	set ALPHA=0
)

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
	set TEMP_NAME=%APPVEYOR_REPO_TAG_NAME:/=_%

	@rem replace ' ' with '_'
	set TAG_NAME=tag_%TEMP_NAME: =_%
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

@echo on

@rem ----------------------------------------------------------------
@rem build WORKDIR
@rem ----------------------------------------------------------------
set WORKDIR=%BASENAME%
set WORKDIR_LOG=%WORKDIR%\Log
set WORKDIR_EXE=%WORKDIR%\EXE
set WORKDIR_INST=%WORKDIR%\Installer
set WORKDIR_ASM=%BASENAME%-Asm
set OUTFILE=%BASENAME%.zip
set OUTFILE_LOG=%BASENAME%-Log.zip
set OUTFILE_ASM=%BASENAME%-Asm.zip

@rem cleanup for local testing
if exist "%OUTFILE%" (
	del %OUTFILE%
)
if exist "%OUTFILE_LOG%" (
	del %OUTFILE_LOG%
)
if exist "%OUTFILE_ASM%" (
	del %OUTFILE_ASM%
)
if exist "%WORKDIR%" (
	rmdir /s /q %WORKDIR%
)
if exist "%WORKDIR_ASM%" (
	rmdir /s /q %WORKDIR_ASM%
)

mkdir %WORKDIR%
mkdir %WORKDIR_LOG%
mkdir %WORKDIR_EXE%
mkdir %WORKDIR_INST%
copy /Y /B %platform%\%configuration%\sakura.exe %WORKDIR_EXE%\
copy /Y /B %platform%\%configuration%\*.dll      %WORKDIR_EXE%\
copy /Y /B %platform%\%configuration%\*.pdb      %WORKDIR_EXE%\

copy /Y /B help\macro\macro.chm    %WORKDIR_EXE%\
copy /Y /B help\plugin\plugin.chm  %WORKDIR_EXE%\
copy /Y /B help\sakura\sakura.chm  %WORKDIR_EXE%\

copy /Y installer\warning.txt   %WORKDIR%\
if "%ALPHA%" == "1" (
	copy /Y installer\warning-alpha.txt   %WORKDIR%\
)
copy /Y /B installer\Output-%platform%\*.exe       %WORKDIR_INST%\
copy /Y msbuild-%platform%-%configuration%.log     %WORKDIR_LOG%\
copy /Y msbuild-%platform%-%configuration%.log.csv %WORKDIR_LOG%\
if exist "msbuild-%platform%-%configuration%.log.xlsx" (
	copy /Y /B "msbuild-%platform%-%configuration%.log.xlsx" %WORKDIR_LOG%\
)
copy /Y sakura_core\githash.h                      %WORKDIR_LOG%\
if exist "cppcheck-install.log" (
	copy /Y "cppcheck-install.log" %WORKDIR_LOG%\
)
if exist "cppcheck.xml" (
	copy /Y "cppcheck.xml" %WORKDIR_LOG%\
)

if exist "set_appveyor_env.bat" (
	copy /Y "set_appveyor_env.bat" %WORKDIR_LOG%\
)

set HASHFILE=sha256.txt
if exist "%HASHFILE%" (
	del %HASHFILE%
)
call calc-hash.bat %HASHFILE% %WORKDIR%\
if exist "%HASHFILE%" (
	copy /Y %HASHFILE%           %WORKDIR%\
)
call %ZIP_CMD%       %OUTFILE%      %WORKDIR%
call %LIST_ZIP_CMD%  %OUTFILE%

call %ZIP_CMD%       %OUTFILE_LOG%  %WORKDIR_LOG%
call %LIST_ZIP_CMD%  %OUTFILE_LOG%

@echo start zip asm
mkdir %WORKDIR_ASM%
copy /Y sakura\%platform%\%configuration%\*.asm %WORKDIR_ASM%\
call %ZIP_CMD%       %OUTFILE_ASM%  %WORKDIR_ASM%
call %LIST_ZIP_CMD%  %OUTFILE_ASM%

@echo end   zip asm

if exist "%WORKDIR%" (
	rmdir /s /q %WORKDIR%
)
if exist "%WORKDIR_ASM%" (
	rmdir /s /q %WORKDIR_ASM%
)
