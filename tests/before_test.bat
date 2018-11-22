set platform=%1
set configuration=%2
set ERROR_RESULT=0

@echo ---- start create-project.bat ----
call "%~dp0create-project.bat" %platform% %configuration%
if errorlevel 1 (
	@echo ERROR in create-project.bat %errorlevel%
	exit /b 1
)
@echo ---- end   create-project.bat ----

@echo ---- start build-project.bat ----
call "%~dp0build-project.bat" %platform% %configuration%
if errorlevel 1 (
	@echo ERROR in build-project.bat %errorlevel%
	exit /b 1
)
@echo ---- end   build-project.bat ----
