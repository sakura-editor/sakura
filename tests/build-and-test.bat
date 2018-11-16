@echo off
setlocal
set platform=%1
set configuration=%2

if "%platform%" == "MinGW" set PATH=C:\msys64\mingw64\bin;%PATH:C:\Program Files\Git\usr\bin;=%

for %%B in (
	create-project.bat
	build-project.bat
	run-tests.bat
) do (
	call :ExecBat "%~dp0%%~B" %PLATFORM% %CONFIGURATION% ^
	|| (echo ERROR %%~B %ERRORLEVEL% & exit /b 1)
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

	call :StartWatch
	echo ---- start %Batch% [%StartDate% %StartTime%] ----
	echo.
	cmd /C "call "%Batch%" %Platform% %Config%" ^
	|| exit /b 1
	call :StopWatch
	echo ---- end   %Batch% [%StopDate% %StopTime% / %Elapsed% seconds elapsed.] ----
	echo.
exit /b 0

:StartWatch
	set StartDate=%DATE%
	set StartTime=%TIME: =0%
exit /b 0

:StopWatch
	rem Be aware of space-padding hours and octal number literal
	rem when calculating the Elapsed.

	set StopDate=%DATE%
	set StopTime=%TIME: =0%
	set /A Elapsed=(1%StopTime:~0,2%  * 3600 + 1%StopTime:~3,2%  * 60 + 1%StopTime:~6,2%) ^
	              -(1%StartTime:~0,2% * 3600 + 1%StartTime:~3,2% * 60 + 1%StartTime:~6,2%)
	if not "%StartDate%" == "%StopDate%" (
		set /A Elapsed=%Elapsed% + 60 * 3600
	)
exit /b 0
