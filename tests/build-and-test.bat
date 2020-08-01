set platform=%1
set configuration=%2
set ERROR_RESULT=0

if "%platform%" == "MinGW" (
	set BUILD_EDITOR_BAT=build-gnu.bat
) else (
	set BUILD_EDITOR_BAT=build-sln.bat
)

@echo ---- start %BUILD_EDITOR_BAT% ----
call "%~dp0..\%BUILD_EDITOR_BAT%" %platform% %configuration% || set ERROR_RESULT=1
@echo ---- end   %BUILD_EDITOR_BAT% ----

if "%ERROR_RESULT%" == "1" (
	@echo ERROR
	exit /b 1
)

@echo ---- start run-tests.bat ----
call "%~dp0run-tests.bat" %platform% %configuration%
if errorlevel 1 (
	@echo ERROR in run-tests.bat %errorlevel%
	exit /b 1
)
@echo ---- end   run-tests.bat ----
