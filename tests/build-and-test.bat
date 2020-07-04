set platform=%1
set configuration=%2
set ERROR_RESULT=0

if "%platform%" == "MinGW" (
	goto :RUN_CREATE_PROJECT
)

@echo ---- start build-sln.bat ----
pushd "%~dp0.."
call  build-sln.bat %platform% %configuration% || set ERROR_RESULT=1
if errorlevel 1 (
	@echo ERROR in build-sln.bat %errorlevel%
	popd
	exit /b 1
)
popd
@echo ---- end   build-sln.bat ----
goto :RUN_TESTS

:RUN_CREATE_PROJECT
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

:RUN_TESTS
@echo ---- start run-tests.bat ----
call "%~dp0run-tests.bat" %platform% %configuration%
if errorlevel 1 (
	@echo ERROR in run-tests.bat %errorlevel%
	exit /b 1
)
@echo ---- end   run-tests.bat ----
