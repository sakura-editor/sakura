@echo off
setlocal
set platform=%1
set configuration=%2

if "%platform%" == "Win32" (
	@rem OK
) else if "%platform%" == "x64" (
	@rem OK
) else if "%platform%" == "MinGW" (
	@rem OK
) else (
	call :showhelp %0
	exit /b 1
)

if "%configuration%" == "Release" (
	@rem OK
) else if "%configuration%" == "Debug" (
	@rem OK
) else (
	call :showhelp %0
	exit /b 1
)

@echo PLATFORM      %PLATFORM%
@echo CONFIGURATION %CONFIGURATION%
@echo.

rem plarform & config specialization

if "%PLATFORM%" == "MinGW" (
	set BatchJobs=build-gnu.bat
	rem Skip all other batch files because they reject MinGW platform.
) else (
	set BatchJobs=build-sln.bat^
		build-chm.bat^
		build-installer.bat^
		run-cppcheck.bat^
		run-doxygen.bat^
		zipArtifacts.bat
)

rem run

for %%B in (%BatchJobs%) do (
	call :ExecBat "%~dp0%%~B" %PLATFORM% %CONFIGURATION% ^
	|| (echo error %%~B & exit /b 1)
)
exit /b 0

rem ----------------------------------------------
rem  sub-routines
rem ----------------------------------------------

:ExecBat
	setlocal
	set    Batch=%~1
	set Platform=%~2
	set   Config=%~3

	echo ---- start %Batch% ----
	echo.
	cmd /C "call "%Batch%" %Platform% %Config%" ^
	|| exit /b 1
	echo ---- end   %Batch% ----
	echo.
exit /b 0

@rem ------------------------------------------------------------------------------
@rem show help
@rem see http://orangeclover.hatenablog.com/entry/20101004/1286120668
@rem ------------------------------------------------------------------------------
:showhelp
@echo off
@echo usage
@echo    %~nx1 platform configuration
@echo.
@echo parameter
@echo    platform      : Win32   or x64   or MinGW
@echo    configuration : Release or Debug
@echo.
@echo example
@echo    %~nx1 Win32 Release
@echo    %~nx1 Win32 Debug
@echo    %~nx1 x64   Release
@echo    %~nx1 x64   Debug
@echo    %~nx1 MinGW Release
@echo    %~nx1 MinGW Debug
exit /b 0
