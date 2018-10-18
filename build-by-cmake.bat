@echo off
set platform=%1
set configuration=%2
set argument=%3
set BUILDDIR=build\%platform%
set PROJECT_TOP=%~dp0

if "%platform%" == "" (
	call :showhelp %0
	exit /b 1
)

if "%configuration%" == "" (
	call :showhelp %0
	exit /b 1
)

cd /d %~dp0

if "%argument%" == "clean" (
	for /l %%n in (1,1,10) do (
		if exist "%BUILDDIR%" (
			@echo ---- removing %BUILDDIR% -----
			rmdir /s /q "%BUILDDIR%"
		)
	)
)

if not exist "%BUILDDIR%" (
	@echo ---- create %BUILDDIR% -----
	mkdir "%BUILDDIR%" || exit /b 1
)
set ERROR_RESULT=0

if "%platform%" == "MinGW" (
	@echo disable build for MinGW temporally.
	exit /b 0
)
if "%platform%" == "MinGW" (
	set GENERATER_PARAM=-G"MinGW Makefiles"
) else (
	set GENERATER_PARAM=-DCMAKE_GENERATOR_PLATFORM=%platform%
)

@echo ---- creating project -----
@echo cmake %GENERATER_PARAM%  -B%BUILDDIR% -H%PROJECT_TOP%
cmake %GENERATER_PARAM% -B%BUILDDIR% -H%PROJECT_TOP% || set ERROR_RESULT=1
if "%ERROR_RESULT%" == "1" (
	@echo ERROR
	exit /b 1
)

@echo ---- building project -----
@echo cmake --build %BUILDDIR%  --config %configuration%
cmake --build %BUILDDIR%  --config %configuration% || set ERROR_RESULT=1
if "%ERROR_RESULT%" == "1" (
	@echo ERROR
	exit /b 1
)

@echo ---- build succeeded -----
exit /b 0

@rem ------------------------------------------------------------------------------
@rem show help
@rem ------------------------------------------------------------------------------
:showhelp
@echo off
@echo usage %1 platform configuration
exit /b
