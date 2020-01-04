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

set ZIP_CMD=%~dp0tools\zip\zip.bat
set LIST_ZIP_CMD=%~dp0tools\zip\listzip.bat

@rem for GIT_TAG_NAME
call %~dp0sakura\githash.bat %~dp0sakura_core

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

@echo checking GIT_TAG_NAME %GIT_TAG_NAME%
if not "%GIT_TAG_NAME%" == "" (
	@rem replace '/' with '_'
	set TEMP_NAME1=!GIT_TAG_NAME:/=_!
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

@echo hash name
set SHORTHASH=%TEMP_GIT_SHORT_COMMIT_HASH%

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
set WORKDIR=%BASENAME%

set RELDIR_LOG=Log
set RELDIR_EXE=EXE
set RELDIR_DEV=DEV
set RELDIR_INST=Installer
set RELDIR_ASM=Asm

set WORKDIR_LOG=%WORKDIR%\%RELDIR_LOG%
set WORKDIR_EXE=%WORKDIR%\%RELDIR_EXE%
set WORKDIR_DEV=%WORKDIR%\%RELDIR_DEV%
set WORKDIR_INST=%WORKDIR%\%RELDIR_INST%
set WORKDIR_ASM=%WORKDIR%\%RELDIR_ASM%

set OUTFILE=%~dp0%BASENAME%-All.zip
set OUTFILE_LOG=%~dp0%BASENAME%-Log.zip
set OUTFILE_ASM=%~dp0%BASENAME%-Asm.zip
set OUTFILE_INST=%~dp0%BASENAME%-Installer.zip
set OUTFILE_EXE=%~dp0%BASENAME%-Exe.zip
set OUTFILE_DEV=%~dp0%BASENAME%-Dev.zip

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
if exist "%OUTFILE_INST%" (
	del %OUTFILE_INST%
)
if exist "%OUTFILE_EXE%" (
	del %OUTFILE_EXE%
)
if exist "%OUTFILE_DEV%" (
	del %OUTFILE_DEV%
)
if exist "%WORKDIR%" (
	rmdir /s /q "%WORKDIR%"
)
if exist "%WORKDIR_ASM%" (
	rmdir /s /q "%WORKDIR_ASM%"
)

mkdir %WORKDIR%
mkdir %WORKDIR_LOG%
mkdir %WORKDIR_EXE%
mkdir %WORKDIR_EXE%\license\
mkdir %WORKDIR_EXE%\license\bregonig\
mkdir %WORKDIR_EXE%\license\ctags\
mkdir %WORKDIR_DEV%
mkdir %WORKDIR_INST%
copy /Y /B %platform%\%configuration%\sakura.exe %WORKDIR_EXE%\
copy /Y /B %platform%\%configuration%\*.dll      %WORKDIR_EXE%\
copy /Y /B %platform%\%configuration%\*.pdb      %WORKDIR_DEV%\

: LICENSE
copy /Y .\LICENSE                                   %WORKDIR_EXE%\license\ > NUL

: bregonig
set INSTALLER_RESOURCES_BRON=%~dp0installer\temp\bron
copy /Y %INSTALLER_RESOURCES_BRON%\*.txt            %WORKDIR_EXE%\license\bregonig\

: ctags.exe
set INSTALLER_RESOURCES_CTAGS=%~dp0installer\temp\ctags
copy /Y /B %INSTALLER_RESOURCES_CTAGS%\ctags.exe    %WORKDIR_EXE%\
copy /Y /B %INSTALLER_RESOURCES_CTAGS%\README.md    %WORKDIR_EXE%\license\ctags\
copy /Y /B %INSTALLER_RESOURCES_CTAGS%\license\*.*  %WORKDIR_EXE%\license\ctags\

copy /Y /B help\macro\macro.chm    %WORKDIR_EXE%\
copy /Y /B help\plugin\plugin.chm  %WORKDIR_EXE%\
copy /Y /B help\sakura\sakura.chm  %WORKDIR_EXE%\
copy /Y /B html\sakura-doxygen.chm %WORKDIR_DEV%\
copy /Y /B html\sakura-doxygen.chi %WORKDIR_DEV%\

copy /Y /B installer\Output-%platform%\*.exe       %WORKDIR_INST%\
copy /Y msbuild-%platform%-%configuration%.log     %WORKDIR_LOG%\
copy /Y msbuild-%platform%-%configuration%.log.csv %WORKDIR_LOG%\
if exist "msbuild-%platform%-%configuration%.log.xlsx" (
	copy /Y /B "msbuild-%platform%-%configuration%.log.xlsx" %WORKDIR_LOG%\
)
set ISS_LOG_FILE=iss-%platform%-%configuration%.log
if exist "%ISS_LOG_FILE%" (
	copy /Y /B "%ISS_LOG_FILE%" %WORKDIR_LOG%\
)

copy /Y sakura_core\githash.h                      %WORKDIR_LOG%\
if exist "cppcheck-install.log" (
	copy /Y "cppcheck-install.log" %WORKDIR_LOG%\
)
if exist "cppcheck-%platform%-%configuration%.xml" (
	copy /Y "cppcheck-%platform%-%configuration%.xml" %WORKDIR_LOG%\
)
if exist "cppcheck-%platform%-%configuration%.log" (
	copy /Y "cppcheck-%platform%-%configuration%.log" %WORKDIR_LOG%\
)
if exist "doxygen-%platform%-%configuration%.log" (
	copy /Y "doxygen-%platform%-%configuration%.log" %WORKDIR_LOG%\
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

copy /Y installer\warning.txt   %WORKDIR%\
if "%ALPHA%" == "1" (
	copy /Y installer\warning-alpha.txt   %WORKDIR%\
)
@rem temporally disable to zip all files to a file to workaround #514.
@rem pushd %WORKDIR% && call %ZIP_CMD%       %OUTFILE%      .             && popd

pushd %WORKDIR_LOG%  && call %ZIP_CMD%       %OUTFILE_LOG%  .  && popd

@rem copy text files for warning after zipping %OUTFILE% because %WORKDIR% is the parent directory of %WORKDIR_EXE% and %WORKDIR_INST%.
if "%ALPHA%" == "1" (
	copy /Y installer\warning-alpha.txt   %WORKDIR_EXE%\
	copy /Y installer\warning-alpha.txt   %WORKDIR_INST%\
)
copy /Y installer\warning.txt        %WORKDIR_EXE%\
copy /Y installer\warning.txt        %WORKDIR_INST%\

pushd %WORKDIR_INST% && call %ZIP_CMD%       %OUTFILE_INST% .  && popd
pushd %WORKDIR_EXE%  && call %ZIP_CMD%       %OUTFILE_EXE%  .  && popd
pushd %WORKDIR_DEV%  && call %ZIP_CMD%       %OUTFILE_DEV%  .  && popd

@echo start zip asm
mkdir %WORKDIR_ASM%
copy /Y sakura\%platform%\%configuration%\*.asm %WORKDIR_ASM%\ > NUL
pushd %WORKDIR_ASM%  && call %ZIP_CMD%       %OUTFILE_ASM%  .  && popd

@echo end   zip asm

if exist "%WORKDIR%" (
	rmdir /s /q "%WORKDIR%"
)
if exist "%WORKDIR_ASM%" (
	rmdir /s /q "%WORKDIR_ASM%"
)


@echo start generate MD5 hash
set CMD_FIND=%SystemRoot%\System32\find.exe
certutil -hashfile %OUTFILE_EXE% MD5  | %CMD_FIND% /v "MD5" | %CMD_FIND% /v "CertUtil" > %OUTFILE_EXE%.md5
certutil -hashfile %OUTFILE_INST% MD5 | %CMD_FIND% /v "MD5" | %CMD_FIND% /v "CertUtil" > %OUTFILE_INST%.md5
@echo end generate MD5 hash


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
