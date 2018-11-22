set platform=%1
set configuration=%2
set ERROR_RESULT=0

@echo ---- start run-tests.bat ----
call "%~dp0run-tests.bat" %platform% %configuration%
if errorlevel 1 (
	@echo ERROR in run-tests.bat %errorlevel%
	exit /b 1
)
@echo ---- end   run-tests.bat ----
