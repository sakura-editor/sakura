if not defined CMD_HHC call %~dp0tools\find-tools.bat
if not defined CMD_HHC (
	echo hhc.exe was not found.
	exit /b 1
)

set HHP_MACRO=help\macro\macro.HHP
set HHP_PLUGIN=help\plugin\plugin.hhp
set HHP_SAKURA=help\sakura\sakura.hhp

set CHM_MACRO=help\macro\macro.chm
set CHM_PLUGIN=help\plugin\plugin.chm
set CHM_SAKURA=help\sakura\sakura.chm
set HH_SCRIPT=%~dp0help\remove-comment.py
set HH_INPUT=sakura_core\sakura.hh
set HH_OUTPUT=help\sakura\sakura.hh

if "%PLATFORM%" neq "BuildChm" (
	goto :download_archive
	exit /b 0
)

if exist "%HH_OUTPUT%" del /F "%HH_OUTPUT%"
python "%HH_SCRIPT%" "%HH_INPUT%" "%HH_OUTPUT%"  || (echo error && exit /b 1)

call :BuildChm %HHP_MACRO%  %CHM_MACRO%   || (echo error && exit /b 1)
call :BuildChm %HHP_PLUGIN% %CHM_PLUGIN%  || (echo error && exit /b 1)
call :BuildChm %HHP_SAKURA% %CHM_SAKURA%  || (echo error && exit /b 1)
exit /b 0

@rem ------------------------------------------------------------------------------
@rem BuildChm
@rem ------------------------------------------------------------------------------
:BuildChm
set PROJECT_HHP=%1
set PROJECT_CHM=%2

if exist "%PROJECT_CHM%" del /F "%PROJECT_CHM%"

@rem hhc.exe returns 1 on success, and returns 0 on failure
"%CMD_HHC%" %PROJECT_HHP%
if not errorlevel 1 (
	echo error %PROJECT_HHP% errorlevel %errorlevel%

	del /F "%PROJECT_CHM%"
	"%CMD_HHC%" %PROJECT_HHP%
)
if not errorlevel 1 (
	echo retry error %PROJECT_HHP% errorlevel %errorlevel%
	exit /b 1
)
exit /b 0

:download_archive
powershell -ExecutionPolicy RemoteSigned -File %~dp0help\extract-chm-from-artifact.ps1
if errorlevel 1 exit /b 1
exit /b 0
