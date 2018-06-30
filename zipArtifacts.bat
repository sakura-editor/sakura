set platform=%1
set configuration=%2
@echo off
if "%platform%" == "x64" (
	set ALPHA=1
) else (
	set ALPHA=0
)

@echo checking APPVEYOR_BUILD_NUMBER
if not "%APPVEYOR_BUILD_NUMBER%" == "" (
	set BUILD_NUMBER=build%APPVEYOR_BUILD_NUMBER%
)

@echo checking APPVEYOR_REPO_TAG_NAME
if not "%APPVEYOR_REPO_TAG_NAME%" == "" (
	@rem replace '/' with '_'
	set TAG_NAME=tag_%APPVEYOR_REPO_TAG_NAME:/=_%
)

@echo checking APPVEYOR_PULL_REQUEST_NUMBER
if not "%APPVEYOR_PULL_REQUEST_NUMBER%" == "" (
	set PR_NAME=PR%APPVEYOR_PULL_REQUEST_NUMBER%
)

@echo checking APPVEYOR_REPO_COMMIT
set SHORTHASH=%APPVEYOR_REPO_COMMIT:~0,8%
if not "%SHORTHASH%" == "" (
	set COMMITHASH=%SHORTHASH%
)

if "%ALPHA%" == "1" (
	set RELEASE_PHASE=alpha
) else (
	set RELEASE_PHASE=
)

set SUFFIX=

@echo adding BUILD_NUMBER
if not "%BUILD_NUMBER%" == "" (
	set SUFFIX=%SUFFIX%-%BUILD_NUMBER%
)
@echo SUFFIX = %SUFFIX%

@echo adding TAG_NAME
if not "%TAG_NAME%" == "" (
	set SUFFIX=%SUFFIX%-%TAG_NAME%
)
@echo SUFFIX = %SUFFIX%

@echo adding PR_NAME
if not "%PR_NAME%" == "" (
	set SUFFIX=%SUFFIX%-%PR_NAME%
)
@echo SUFFIX = %SUFFIX%

@echo adding COMMITHASH
if not "%COMMITHASH%" == "" (
	set SUFFIX=%SUFFIX%-%COMMITHASH%
)
@echo SUFFIX = %SUFFIX%

@echo adding RELEASE_PHASE
if not "%RELEASE_PHASE%" == "" (
	set SUFFIX=%SUFFIX%-%RELEASE_PHASE%
)
@echo SUFFIX = %SUFFIX%

@echo on
set BASENAME=sakura-%platform%-%configuration%%SUFFIX%
set WORKDIR=%BASENAME%
set WORKDIR_LOG=%WORKDIR%\Log
set WORKDIR_EXE=%WORKDIR%\EXE
set WORKDIR_INST=%WORKDIR%\Installer
set OUTFILE=%BASENAME%.zip

@rem cleanup for local testing
if exist "%OUTFILE%" (
	del %OUTFILE%
)
if exist "%WORKDIR%" (
	rmdir /s /q %WORKDIR%
)

mkdir %WORKDIR%
mkdir %WORKDIR_LOG%
mkdir %WORKDIR_EXE%
mkdir %WORKDIR_INST%
copy %platform%\%configuration%\*.exe %WORKDIR_EXE%\
copy %platform%\%configuration%\*.dll %WORKDIR_EXE%\
copy %platform%\%configuration%\*.pdb %WORKDIR_EXE%\

copy help\macro\macro.chm    %WORKDIR_EXE%\
copy help\plugin\plugin.chm  %WORKDIR_EXE%\
copy help\sakura\sakura.chm  %WORKDIR_EXE%\

copy installer\warning.txt   %WORKDIR%\
if "%ALPHA%" == "1" (
	copy installer\warning-alpha.txt   %WORKDIR%\
)
copy installer\Output\*.exe  %WORKDIR_INST%\
copy msbuild-%platform%-%configuration%.log %WORKDIR_LOG%\
copy sakura_core\githash.h                  %WORKDIR_LOG%\

set HASHFILE=sha256.txt
if exist "%HASHFILE%" (
	del %HASHFILE%
)
call calc-hash.bat %HASHFILE% %WORKDIR%\
if exist "%HASHFILE%" (
	copy %HASHFILE%           %WORKDIR%\
)
7z a %OUTFILE%  -r %WORKDIR%
7z l %OUTFILE%

if exist "%WORKDIR%" (
	rmdir /s /q %WORKDIR%
)
