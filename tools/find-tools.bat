@rem echo off
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
if not defined CMD_VSWHERE call :vswhere 2> nul
if not defined CMD_MSBUILD call :msbuild 2> nul
echo ^|- CMD_GIT=%CMD_GIT%
echo ^|- CMD_7Z=%CMD_7Z%
echo ^|- CMD_HHC=%CMD_HHC%
echo ^|- CMD_ISCC=%CMD_ISCC%
echo ^|- CMD_CPPCHECK=%CMD_CPPCHECK%
echo ^|- CMD_DOXYGEN=%CMD_DOXYGEN%
echo ^|- CMD_VSWHERE=%CMD_VSWHERE%
echo ^|- CMD_MSBUILD=%CMD_MSBUILD%
echo ^|- NUM_VSVERSION=%NUM_VSVERSION%
endlocal && set "CMD_GIT=%CMD_GIT%" && set "CMD_7Z=%CMD_7Z%" && set "CMD_HHC=%CMD_HHC%" && set "CMD_ISCC=%CMD_ISCC%" && set "CMD_CPPCHECK=%CMD_CPPCHECK%" && set "CMD_DOXYGEN=%CMD_DOXYGEN%" && set "CMD_VSWHERE=%CMD_VSWHERE%" && set "CMD_MSBUILD=%CMD_MSBUILD%" && set /A NUM_VSVERSION=%NUM_VSVERSION%
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

:vswhere
:: ref https://github.com/Microsoft/vswhere
set APPDIR=Microsoft Visual Studio\Installer
set PATH2=%ProgramFiles(x86)%\%APPDIR%;%ProgramFiles%\%APPDIR%;%ProgramW6432%\%APPDIR%;%PATH%
for /f "usebackq delims=" %%a in (`where $PATH2:vswhere.exe`) do ( 
    set "CMD_VSWHERE=%%a"
    exit /b
)
exit /b

:msbuild
::find vs2017 install directory
for /f "usebackq delims=" %%d in (`"%CMD_VSWHERE%" -version [15^,16^) -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set "Vs2017InstallRoot=%%d"
)
if not defined Vs2017InstallRoot goto :msbuild_latest
if defined USE_LATEST_MSBUILD (
    for /f "usebackq delims=" %%d in (`"%CMD_VSWHERE%" -latest -requires Microsoft.Component.MSBuild -property installationPath`) do (
        set "VsInstallRoot=%%d"
    )
    if "%VsInstallRoot%" neq "%Vs2017InstallRoot%" goto :msbuild_latest
)
goto :msbuild_vs2017
exit /b

:msbuild_vs2017
::find msbuild under vs2017 install directory
set PATH3=%Vs2017InstallRoot%\MSBuild\15.0\Bin\amd64;%Vs2017InstallRoot%\MSBuild\15.0\Bin
for /f "usebackq delims=" %%a in (`where $PATH3:MSBuild.exe`) do ( 
    set "CMD_MSBUILD=%%a"
    set /A NUM_VSVERSION=15
    exit /b
)
exit /b

:msbuild_latest
::find msbuild bundled with latest visual studio(vs2019 or lator).
for /f "usebackq delims=" %%a in (`"%CMD_VSWHERE%" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
    set "CMD_MSBUILD=%%a"
    for /f "usebackq delims=. tokens=1" %%b in (`"%CMD_VSWHERE%" -latest -requires Microsoft.Component.MSBuild -property installationVersion`) do (
        set /A NUM_VSVERSION=%%b
    )
    exit /b
)
exit /b
