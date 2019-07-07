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

set vcvarsArchitecture=
if "%PROCESSOR_ARCHITECTURE%" == "x86" (
	if "%platform%" == "x64" (
		set vcvarsArchitecture=x86_x64
	) else (
		set vcvarsArchitecture=x86
	)
) else (
	if "%platform%" == "x64" (
		set vcvarsArchitecture=x64
	) else (
		set vcvarsArchitecture=x64_x86
	)
)

for /f "usebackq delims=" %%a in (`vswhere -version [%NUM_VSVERSION%^,%NUM_NEXTVSVERSION%^) -find VC/Auxiliary/Build/vcvarsall.bat`) do (
    call "%%a" %vcvarsArchitecture% || exit /b 1
)

echo test for 'Visual Studio' generator.
if "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
	set CMAKE_GEN_OPT=-G "Visual Studio %NUM_VSVERSION% %productLineVersion% Win64"
) else (
	set CMAKE_GEN_OPT=-G "Visual Studio %NUM_VSVERSION% %productLineVersion%" -A "%platform%"
)
for /L %%a IN (0,1,9) DO (
	set BUILDDIR=build\vs%%a
	call :run_cmake vs %%a || exit /b 1
)

echo test for 'Ninja' generator.
set CMAKE_GEN_OPT=-G "Ninja"
for /L %%a IN (0,1,9) DO (
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
cmake %CMAKE_GEN_OPT% -DCMAKE_BUILD_TYPE=%configuration% -Dgtest_build_tests=OFF -Dgtest_build_samples=OFF -D BUILD_GMOCK=OFF -B"%BUILDDIR%" -Hgoogletest || exit /b 1
cmake --build "%BUILDDIR%" --config %configuration% || exit /b 1
echo %1(%2) - end
echo .
exit /b 0

:EOF
