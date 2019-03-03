@echo off
setlocal

if defined FIND_TOOLS_CALLED (
    echo find-tools.bat already called
    exit /b
)

echo find-tools.bat
if not defined CMD_GIT call :Git 2> nul
if not defined CMD_7Z call :7z 2> nul
if not defined CMD_HHC call :hhc 2> nul
if not defined CMD_ISCC call :iscc 2> nul
if not defined CMD_CPPCHECK call :cppcheck 2> nul
if not defined CMD_DOXYGEN call :doxygen 2> nul
if not defined CMD_MSBUILD call :msbuild 2> nul
echo ^|- CMD_GIT=%CMD_GIT%
echo ^|- CMD_7Z=%CMD_7Z%
echo ^|- CMD_HHC=%CMD_HHC%
echo ^|- CMD_ISCC=%CMD_ISCC%
echo ^|- CMD_CPPCHECK=%CMD_CPPCHECK%
echo ^|- CMD_DOXYGEN=%CMD_DOXYGEN%
echo ^|- CMD_MSBUILD=%CMD_MSBUILD%
endlocal && set "CMD_GIT=%CMD_GIT%" && set "CMD_7Z=%CMD_7Z%" && set "CMD_HHC=%CMD_HHC%" && set "CMD_ISCC=%CMD_ISCC%" && set "CMD_CPPCHECK=%CMD_CPPCHECK%" && set "CMD_DOXYGEN=%CMD_DOXYGEN%"&& set "CMD_MSBUILD=%CMD_MSBUILD%"
set FIND_TOOLS_CALLED=1
exit /b

:Git
set APPDIR=Git\Cmd
set PATH2=%PATH%;%ProgramFiles%\%APPDIR%\;%ProgramFiles(x86)%\%APPDIR%\;%ProgramW6432%\%APPDIR%\;
for /f "usebackq delims=" %%a in (`where $PATH2:Git`) do ( 
    set "CMD_GIT=%%a"
    exit /b
)
exit /b

:7z
if "%FORCE_POWERSHELL_ZIP%" == "1" (
	exit /b
)
set APPDIR=7-Zip
set PATH2=%PATH%;%ProgramFiles%\%APPDIR%\;%ProgramFiles(x86)%\%APPDIR%\;%ProgramW6432%\%APPDIR%\;
for /f "usebackq delims=" %%a in (`where $PATH2:7z`) do ( 
    set "CMD_7Z=%%a"
    exit /b
)
exit /b

:hhc
set APPDIR=HTML Help Workshop
set PATH2=%PATH%;%ProgramFiles%\%APPDIR%\;%ProgramFiles(x86)%\%APPDIR%\;%ProgramW6432%\%APPDIR%\;
for /f "usebackq delims=" %%a in (`where $PATH2:hhc.exe`) do ( 
    set "CMD_HHC=%%a"
    exit /b
)
exit /b

:iscc
set APPDIR=Inno Setup 5
set PATH2=%PATH%;%ProgramFiles%\%APPDIR%\;%ProgramFiles(x86)%\%APPDIR%\;%ProgramW6432%\%APPDIR%\;
for /f "usebackq delims=" %%a in (`where $PATH2:ISCC.exe`) do ( 
    set "CMD_ISCC=%%a"
    exit /b
)
exit /b

:cppcheck
set APPDIR=cppcheck
set PATH2=%PATH%;%ProgramFiles%\%APPDIR%\;%ProgramFiles(x86)%\%APPDIR%\;%ProgramW6432%\%APPDIR%\;
for /f "usebackq delims=" %%a in (`where $PATH2:cppcheck.exe`) do ( 
    set "CMD_CPPCHECK=%%a"
    exit /b
)
exit /b

:doxygen
set APPDIR=doxygen\bin
set PATH2=%PATH%;%ProgramFiles%\%APPDIR%\;%ProgramFiles(x86)%\%APPDIR%\;%ProgramW6432%\%APPDIR%\;
for /f "usebackq delims=" %%a in (`where $PATH2:doxygen.exe`) do ( 
    set "CMD_DOXYGEN=%%a"
    exit /b
)
exit /b

:: ref https://github.com/Microsoft/vswhere
:msbuild
for /f "usebackq delims=" %%a in (`where MSBuild.exe`) do ( 
    set "CMD_MSBUILD=%%a"
    exit /b
)

setlocal
set APPDIR=Microsoft Visual Studio\Installer
PATH=%PATH%;%ProgramFiles%\%APPDIR%\;%ProgramFiles(x86)%\%APPDIR%\;%ProgramW6432%\%APPDIR%\;
for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    endlocal && set "CMD_MSBUILD=%%i\MSBuild\15.0\Bin\MSBuild.exe"
    exit /b
)
endlocal
exit /b
