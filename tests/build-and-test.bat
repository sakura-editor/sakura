set platform=%1
set configuration=%2
set ERROR_RESULT=0

call %~dp0create-project.bat %platform% %configuration%
if %ERRORLEVEL% neq 0 (
	@echo ERROR in create-project.bat
	exit /b 1
)
call %~dp0build-project.bat %platform% %configuration%
if %ERRORLEVEL% neq 0 (
	@echo ERROR in build-project.bat
	exit /b 1
)

call %~dp0run-tests.bat %platform% %configuration%
if %ERRORLEVEL% neq 0 (
	@echo ERROR in run-tests.bat
	exit /b 1
)
