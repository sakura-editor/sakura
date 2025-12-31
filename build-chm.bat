@echo off
set platform=%1

call %~dp0tools\find-tools.bat
if not defined CMD_CMAKE (
    echo cmake.exe was not found.
    exit /b 1
)

if not defined platform (
    set platform=x64
)

set BUILD_DIR=%~dp0build\%platform%\build-chm

chcp 65001 > NUL

rem Build CHM files using CMake
"%CMD_CMAKE%" -G "%CMAKE_G_PARAM%" -A %platform% -S . -B "%BUILD_DIR%" -DCMAKE_CONFIGURATION_TYPES="Debug;Release"
if errorlevel 1 (
    echo CMake configuration failed
    exit /b 1
)

"%CMD_CMAKE%" --build "%BUILD_DIR%" --target generate_all_chm -- /nologo
if errorlevel 1 (
    echo CMake build failed
    exit /b 1
)

set DST_HELP=%BUILD_DIR%\converted_help_source

call :CopyOutputs %DST_HELP%\macro\macro.chm    || (echo error && exit /b 1)
call :CopyOutputs %DST_HELP%\plugin\plugin.chm  || (echo error && exit /b 1)
call :CopyOutputs %DST_HELP%\sakura\sakura.chm  || (echo error && exit /b 1)

echo CHM build completed successfully
exit /b 0

@rem ------------------------------------------------------------------------------
@rem CopyOutputs
@rem ------------------------------------------------------------------------------
:CopyOutputs
set PROJECT_CHM=%1

copy /Y "%~dpn1.chm"        "%~dp0help\%~n1\" > NUL || (echo error && exit /b 1)
copy /Y "%~dp1\Compile.Log" "%~dp0help\%~n1\" > NUL || (echo error && exit /b 1)

exit /b 0
