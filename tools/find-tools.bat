@echo off
setlocal

if "%1" equ "clear" (
    call :clear_variables
    echo find-tools.bat has been cleared
    exit /b
) else if "%~1" neq "" (
    set "ARG_VSVERSION=%~1"
)
if defined FIND_TOOLS_CALLED (
    echo find-tools.bat already called
    exit /b
)

echo find-tools.bat
if not defined CMD_GIT      call :Git      2> nul
if not defined CMD_7Z       call :7z       2> nul
if not defined CMD_HHC      call :hhc      2> nul
if not defined CMD_ISCC     call :iscc     2> nul
if not defined CMD_VSWHERE  call :vswhere  2> nul
if not defined CMD_MSBUILD  call :msbuild  2> nul
if not defined CMD_CMAKE    call :cmake    2> nul
if not defined CMD_NINJA    call :cmake    2> nul
if not defined CMD_LEPROC   call :leproc   2> nul
if not defined CMD_PYTHON   call :python   2> nul
echo ^|- CMD_GIT=%CMD_GIT%
echo ^|- CMD_7Z=%CMD_7Z%
echo ^|- CMD_HHC=%CMD_HHC%
echo ^|- CMD_ISCC=%CMD_ISCC%
echo ^|- CMD_VSWHERE=%CMD_VSWHERE%
echo ^|- CMD_MSBUILD=%CMD_MSBUILD%
echo ^|- CMD_CMAKE=%CMD_CMAKE%
echo ^|- CMD_NINJA=%CMD_NINJA%
echo ^|- CMD_LEPROC=%CMD_LEPROC%
echo ^|- CMD_PYTHON=%CMD_PYTHON%
echo ^|- CMAKE_G_PARAM=%CMAKE_G_PARAM%
endlocal ^
    && set "CMD_GIT=%CMD_GIT%"                  ^
    && set "CMD_7Z=%CMD_7Z%"                    ^
    && set "CMD_HHC=%CMD_HHC%"                  ^
    && set "CMD_ISCC=%CMD_ISCC%"                ^
    && set "CMD_VSWHERE=%CMD_VSWHERE%"          ^
    && set "CMD_MSBUILD=%CMD_MSBUILD%"          ^
    && set "CMD_CMAKE=%CMD_CMAKE%"              ^
    && set "CMD_NINJA=%CMD_NINJA%"              ^
    && set "CMD_LEPROC=%CMD_LEPROC%"            ^
    && set "CMD_PYTHON=%CMD_PYTHON%"            ^
    && set "NUM_VSVERSION=%NUM_VSVERSION%"      ^
    && set "CMAKE_G_PARAM=%CMAKE_G_PARAM%"      ^
    && echo end

set FIND_TOOLS_CALLED=1
exit /b

:clear_variables
    endlocal
    set CMD_GIT=
    set CMD_7Z=
    set CMD_HHC=
    set CMD_ISCC=
    set CMD_VSWHERE=
    set CMD_MSBUILD=
    set CMD_CMAKE=
    set CMD_NINJA=
    set CMD_LEPROC=
    set CMD_PYTHON=
    set NUM_VSVERSION=
    set CMAKE_G_PARAM=
    set FIND_TOOLS_CALLED=
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
if exist "%CMD_ISCC%" exit /b

set APPDIR=Inno Setup 6
set PATH2=%PATH%;%ProgramFiles%\%APPDIR%\;%ProgramFiles(x86)%\%APPDIR%\;%ProgramW6432%\%APPDIR%\;
for /f "usebackq delims=" %%a in (`where $PATH2:ISCC.exe`) do ( 
    set "CMD_ISCC=%%a"
    exit /b
)
exit /b

:vswhere
:: ref https://github.com/Microsoft/vswhere
set APPDIR=Microsoft Visual Studio\Installer
set PATH2=%PATH%;%ProgramFiles%\%APPDIR%\;%ProgramFiles(x86)%\%APPDIR%\;%ProgramW6432%\%APPDIR%\;
for /f "usebackq delims=" %%a in (`where $PATH2:vswhere.exe`) do ( 
    set "CMD_VSWHERE=%%a"
    exit /b
)
exit /b

:: ---------------------------------------------------------------------------------------------------------------------
:: sub routine for finding msbuild
::
:: ARG_VSVERSION
::     latest => the latest version of installed Visual Studio
::     2017   => Visual Studio 2017
::     2019   => Visual Studio 2019
::     15     => Visual Studio 2017
::     16     => Visual Studio 2019
:: ---------------------------------------------------------------------------------------------------------------------
:msbuild
    if defined ARG_VSVERSION (
        goto :convert_arg_vsversion
    )
    goto :varidate_num_vsversion

:convert_arg_vsversion
    :: convert productLineVersion to Internal Major Version
    if "%ARG_VSVERSION%" == "2017" (
        set NUM_VSVERSION=15
    ) else if "%ARG_VSVERSION%" == "2019" (
        set NUM_VSVERSION=16
    ) else if "%ARG_VSVERSION%" == "latest" (
        call :check_latest_installed_vsversion
    ) else (
        set NUM_VSVERSION=%ARG_VSVERSION%
    )

:varidate_num_vsversion
    if not defined NUM_VSVERSION (
        set NUM_VSVERSION=15
    )

    call :check_installed_vsversion

    call :find_msbuild
    if not exist "%CMD_MSBUILD%" (
        call :find_msbuild_legacy
        set NUM_VSVERSION=15
    )

    if "%NUM_VSVERSION%" == "15" (
        set CMAKE_G_PARAM=Visual Studio 15 2017
    ) else if "%NUM_VSVERSION%" == "16" (
        set CMAKE_G_PARAM=Visual Studio 16 2019
    ) else (
        call :set_cmake_gparam_automatically
    )
    exit /b

:set_cmake_gparam_automatically
    call :get_product_line_version
    set CMAKE_G_PARAM=Visual Studio %NUM_VSVERSION% %VS_PRODUCT_LINE_VERSION%
    exit /b

:get_product_line_version
    set /a NUM_VSVERSION_NEXT=NUM_VSVERSION + 1
    for /f "usebackq delims=" %%v in (`"%CMD_VSWHERE%" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property catalog_productLineVersion -version [%NUM_VSVERSION%^,%NUM_VSVERSION_NEXT%^)`) do (
        set VS_PRODUCT_LINE_VERSION=%%v
    )
    exit /b

:check_latest_installed_vsversion
    for /f "usebackq delims=" %%v in (`"%CMD_VSWHERE%" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion -latest`) do (
        set VSVERSION=%%v
    )
    set NUM_VSVERSION=%VSVERSION:~0,2%
    exit /b

:check_installed_vsversion
    set /a NUM_VSVERSION_NEXT=NUM_VSVERSION + 1
    for /f "usebackq delims=" %%d in (`"%CMD_VSWHERE%" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath -version [%NUM_VSVERSION%^,%NUM_VSVERSION_NEXT%^)`) do (
        if exist "%%d" exit /b
    )
    call :check_latest_installed_vsversion
    exit /b

:find_msbuild
    set /a NUM_VSVERSION_NEXT=NUM_VSVERSION + 1
    for /f "usebackq delims=" %%a in (`"%CMD_VSWHERE%" -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe -version [%NUM_VSVERSION%^,%NUM_VSVERSION_NEXT%^)`) do (
        set "CMD_MSBUILD=%%a"
    )
    if exist "%CMD_MSBUILD%" (
        exit /b
    )
    set CMD_MSBUILD=
    exit /b

:find_msbuild_legacy
    for /f "usebackq delims=" %%d in (`"%CMD_VSWHERE%" -requires Microsoft.Component.MSBuild -property installationPath -version [15^,16^)`) do (
        set "CMD_MSBUILD=%%d\MSBuild\15.0\Bin\MSBuild.exe"
    )
    if exist "%CMD_MSBUILD%" (
        exit /b
    )
    set CMD_MSBUILD=
    exit /b

:cmake
for /f "usebackq delims=" %%a in (`"%CMD_VSWHERE%" -property installationPath -version [%NUM_VSVERSION%^,%NUM_VSVERSION_NEXT%^)`) do (
    pushd "%%a"
    call "%%a\Common7\Tools\vsdevcmd\ext\cmake.bat"
    call :resolve_cmake
    call :resolve_ninja
    popd
    exit /b
)
exit /b

:resolve_cmake
if exist "%CMD_CMAKE%" goto :EOF
set APPDIR=CMake\bin
set PATH2=%PATH%;%ProgramFiles%\%APPDIR%\;%ProgramFiles(x86)%\%APPDIR%\;%ProgramW6432%\%APPDIR%\;
for /f "usebackq delims=" %%a in (`where $PATH2:cmake`) do ( 
    set "CMD_CMAKE=%%a"
    exit /b
)
exit /b

:resolve_ninja
if exist "%CMD_NINJA%" goto :EOF
for /f "usebackq delims=" %%a in (`where $PATH:ninja`) do ( 
    set "CMD_NINJA=%%a"
    exit /b
)
exit /b

:leproc
set PATH2=%PATH%
for /f "usebackq delims=" %%a in (`where $PATH2:LEProc.exe`) do (
    set "CMD_LEPROC=%%a"
    exit /b
)
exit /b

:python
call :find_py
call :check_python_version
if defined CMD_PYTHON (
	exit /b 0
)

call :find_python
call :check_python_version
exit /b 0

:find_py
set PATH2=%PATH%
for /f "usebackq delims=" %%a in (`where $PATH2:py.exe`) do (
    set "CMD_PYTHON=%%a"
    exit /b 0
)
exit /b 0

:find_python
set PATH2=%PATH%
for /f "usebackq delims=" %%a in (`where $PATH2:python.exe`) do (
    set "CMD_PYTHON=%%a"
    exit /b 0
)
exit /b 0

:check_python_version
set PYTHON_VERSION=
for /F "usebackq tokens=2*" %%v in (`"%CMD_PYTHON%" --version`) do (
    set PYTHON_VERSION=%%v
)
if not defined PYTHON_VERSION (
    set CMD_PYTHON=
)
exit /b 0
