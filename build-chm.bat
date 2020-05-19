if not defined CMD_HHC call %~dp0tools\find-tools.bat
if not defined CMD_HHC (
	echo hhc.exe was not found.
	exit /b 1
)

set SRC_HELP=%~dp0help
set TMP_HELP=%~dp0temphelp

@rem create sakura.hh before copying because sakura.hh will be uploaded as an artifact.
set HH_SCRIPT=%SRC_HELP%\remove-comment.py
set HH_INPUT=sakura_core\sakura.hh
set HH_OUTPUT=help\sakura\sakura.hh

if exist "%HH_OUTPUT%" del /F "%HH_OUTPUT%"
python "%HH_SCRIPT%" "%HH_INPUT%" "%HH_OUTPUT%"  || (echo error && exit /b 1)

if exist "%TMP_HELP%" rmdir /s /q    "%TMP_HELP%"
xcopy /i /k /s "%SRC_HELP%" "%TMP_HELP%"

set HHP_MACRO=%TMP_HELP%\macro\macro.HHP
set HHP_PLUGIN=%TMP_HELP%\plugin\plugin.hhp
set HHP_SAKURA=%TMP_HELP%\sakura\sakura.hhp

set CHM_MACRO=%TMP_HELP%\macro\macro.chm
set CHM_PLUGIN=%TMP_HELP%\plugin\plugin.chm
set CHM_SAKURA=%TMP_HELP%\sakura\sakura.chm

if defined APPVEYOR (
	if "%PLATFORM%" neq "BuildChm" (
		goto :download_archive
		exit /b 0
	)
)

set "TOOL_SLN_FILE=%~dp0tools\ChmSourceConverter\ChmSourceConverter.sln"
@echo "%CMD_MSBUILD%" %TOOL_SLN_FILE% "/p:Platform=Any CPU" /p:Configuration=Release /t:"Build" /v:q
      "%CMD_MSBUILD%" %TOOL_SLN_FILE% "/p:Platform=Any CPU" /p:Configuration=Release /t:"Build" /v:q
if errorlevel 1 exit /b 1

%~dp0tools\ChmSourceConverter\ChmSourceConverter\bin\Release\ChmSourceConverter.exe "%TMP_HELP%"
if errorlevel 1 exit /b 1

call :BuildChm %HHP_MACRO%  %CHM_MACRO%   || (echo error && exit /b 1)
call :BuildChm %HHP_PLUGIN% %CHM_PLUGIN%  || (echo error && exit /b 1)
call :BuildChm %HHP_SAKURA% %CHM_SAKURA%  || (echo error && exit /b 1)

copy /Y %TMP_HELP%\macro\*.chm   %SRC_HELP%\macro\   || (echo error && exit /b 1)
copy /Y %TMP_HELP%\plugin\*.chm  %SRC_HELP%\plugin\  || (echo error && exit /b 1)
copy /Y %TMP_HELP%\sakura\*.chm  %SRC_HELP%\sakura\  || (echo error && exit /b 1)

copy /Y %TMP_HELP%\macro\*.Log   %SRC_HELP%\macro\   || (echo error && exit /b 1)
copy /Y %TMP_HELP%\plugin\*.Log  %SRC_HELP%\plugin\  || (echo error && exit /b 1)
copy /Y %TMP_HELP%\sakura\*.Log  %SRC_HELP%\sakura\  || (echo error && exit /b 1)

rmdir /s /q %TMP_HELP%
exit /b 0

@rem ------------------------------------------------------------------------------
@rem BuildChm
@rem ------------------------------------------------------------------------------
:BuildChm
set PROJECT_HHP=%1
set PROJECT_CHM=%2
set PROJECT_LOG=%~dp2\Compile.log

if exist "%PROJECT_CHM%" del /F "%PROJECT_CHM%"
if exist "%PROJECT_LOG%" del /F "%PROJECT_LOG%"

if defined CMD_LEPROC (
	for /L %%i in (1,1,2) do (
		"%CMD_LEPROC%" %COMSPEC% /c """%CMD_HHC%"" %PROJECT_HHP%"
		@rem wait to create chm
		for /L %%j in (1,1,30) do (
			ping -n 2 localhost > NUL
			copy "%PROJECT_LOG%" nul > NUL 2>&1
			if not errorlevel 1 exit /b 0
		)
		echo retry creating %PROJECT_CHM%
	)
	echo fail to create %PROJECT_CHM%
) else (
	for /L %%i in (1,1,2) do (
		@rem hhc.exe returns 1 on success, and returns 0 on failure
		"%CMD_HHC%" %PROJECT_HHP%
		if errorlevel 1 exit /b 0
		echo error %PROJECT_HHP% errorlevel %errorlevel%
		del /F "%PROJECT_CHM%"
	)
)
exit /b 1

:download_archive
pwsh.exe -ExecutionPolicy RemoteSigned -File %SRC_HELP%\extract-chm-from-artifact.ps1
if errorlevel 1 exit /b 1
exit /b 0
