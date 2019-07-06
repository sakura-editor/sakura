@echo off
setlocal
set platform=%1
set configuration=%2

:: remove CMake binary dir in the PATH.
PATH=%PATH:C:\Program Files (x86)\CMake\bin;=%

pushd "%~dp0"

if not exist "googletest\CMakeLists.txt" (
    git submodule init
    git submodule update
)

if not exist build mkdir build

if not defined NUM_VSVERSION (
    for /f "usebackq delims=. tokens=1" %%a in (`vswhere -latest -requires Microsoft.Component.MSBuild -property installationVersion`) do (
        set /A NUM_VSVERSION=%%a
    )
)

set /A NUM_NEXTVSVERSION=%NUM_VSVERSION% + 1
for /f "usebackq" %%b in (`vswhere -version [%NUM_VSVERSION%^,%NUM_NEXTVSVERSION%^) -property catalog_productLineVersion`) do ( 
	set /A productLineVersion=%%b 
) 

for /f "usebackq delims=" %%a in (`vswhere -version [%NUM_VSVERSION%^,%NUM_NEXTVSVERSION%^) -find Common7\Tools\vsdevcmd.bat`) do (
    call "%%a" || exit /b 1
)

echo test for 'Visual Studio' generator.
set CMAKE_GEN_OPT=-G "Visual Studio %NUM_VSVERSION% %productLineVersion%" -A "%platform%"
for /L %%a IN (0,1,10) DO (
	set BUILDDIR=build\vs%%a
	call :run_cmake vs %%a || exit /b 1
)

echo test for 'Ninja' generator.
for /f "usebackq delims=;" %%c in (`vswhere -version [%NUM_VSVERSION%^,%NUM_NEXTVSVERSION%^) -find VC/Tools/MSVC/**/bin/Hostx86/x86/cl.exe`) do ( 
	set "CMD_CL=%%c"
)
set "CMD_CL=%CMD_CL:\=/%"
set CMAKE_GEN_OPT=-G "Ninja" "-DCMAKE_C_COMPILER=%CMD_CL%" "-DCMAKE_CXX_COMPILER=%CMD_CL%"
for /L %%a IN (0,1,10) DO (
	set BUILDDIR=build\nj%%a
	call :run_cmake Ninja %%a || exit /b 1
)

exit /b 0

@rem ----------------------------------------------
@rem  sub-routines
@rem ----------------------------------------------

:run_cmake
echo .
echo %1(%2) - start
cmake %CMAKE_GEN_OPT% -D BUILD_GMOCK=OFF -B"%BUILDDIR%" -Hgoogletest || exit /b 1
cmake --build "%BUILDDIR%" --config %configuration% || exit /b 1
echo %1(%2) - end
echo .
exit /b 0

:EOF
