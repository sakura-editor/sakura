if not defined CMD_HHC call %~dp0tools\find-tools.bat
if not defined CMD_HHC (
	echo hhc.exe was not found.
	exit /b 1
)

if not defined CMD_CMAKE (
    echo cmake.exe was not found.
    exit /b 1
)

set SRC_HELP=%~dp0help
set TMP_HELP=%~dp0build\build-chm\converted_help_source

set HHP_MACRO=%TMP_HELP%\macro\macro.hhp
set HHP_PLUGIN=%TMP_HELP%\plugin\plugin.hhp
set HHP_SAKURA=%TMP_HELP%\sakura\sakura.hhp

set CHM_MACRO=%TMP_HELP%\macro\macro.chm
set CHM_PLUGIN=%TMP_HELP%\plugin\plugin.chm
set CHM_SAKURA=%TMP_HELP%\sakura\sakura.chm

rem Build CHM files using CMake
cd /d %~dp0help
"%CMD_CMAKE%" -G "%CMAKE_G_PARAM%" -A x64 -B "%~dp0build\build-chm"
if errorlevel 1 (
    echo CMake configuration failed
    exit /b 1
)

"%CMD_CMAKE%" --build "%~dp0build\build-chm" -- /nologo
if errorlevel 1 (
    echo CMake build failed
    exit /b 1
)

call :CopyOutputs %HHP_MACRO%  %CHM_MACRO%   || (echo error && exit /b 1)
call :CopyOutputs %HHP_PLUGIN% %CHM_PLUGIN%  || (echo error && exit /b 1)
call :CopyOutputs %HHP_SAKURA% %CHM_SAKURA%  || (echo error && exit /b 1)

echo CHM build completed successfully
exit /b 0

@rem ------------------------------------------------------------------------------
@rem CopyOutputs
@rem ------------------------------------------------------------------------------
:CopyOutputs
set PROJECT_HHP=%1
set PROJECT_CHM=%2

copy /Y %TMP_HELP%\%~n2\*.chm   %SRC_HELP%\%~n2\ || (echo error && exit /b 1)
copy /Y %TMP_HELP%\%~n2\*.Log   %SRC_HELP%\%~n2\ || (echo error && exit /b 1)

exit /b 0
