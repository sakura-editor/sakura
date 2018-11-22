set platform=%1
set configuration=%2
set ERROR_RESULT=0

@echo ---- start before_test.bat ----
call "%~dp0before_test.bat" %platform% %configuration%
if errorlevel 1 (
	@echo ERROR in before_test.bat %errorlevel%
	exit /b 1
)
@echo ---- end   before_test.bat ----

@echo ---- start test_script.bat ----
call "%~dp0test_script.bat" %platform% %configuration%
if errorlevel 1 (
	@echo ERROR in test_script.bat %errorlevel%
	exit /b 1
)
@echo ---- end   test_script.bat ----
