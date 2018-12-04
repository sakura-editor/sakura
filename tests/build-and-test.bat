@echo off
setlocal
set platform=%1
set configuration=%2

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

	echo ---- start %Batch% ----
	echo.
	cmd /C "call "%Batch%" %Platform% %Config%" ^
	|| exit /b 1
	echo ---- end   %Batch% ----
	echo.
exit /b 0
