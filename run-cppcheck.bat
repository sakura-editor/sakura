@echo off
set platform=%1
@rem doesn't use
set configuration=%2

set CPPCHECK_EXE=C:\Program Files\Cppcheck\cppcheck.exe
set CPPCHECK_OUT=cppcheck.xml

set CPPCHECK_PLATFORM=
if "%PLATFORM%" == "Win32" (
	set CPPCHECK_PLATFORM=win32W
) else if "%PLATFORM%" == "x64" (
	set CPPCHECK_PLATFORM=win64
) else (
	@echo not supported platform
	exit /b 1
)

if exist "%CPPCHECK_OUT%" (
	del %CPPCHECK_OUT%
)
if exist "%CPPCHECK_EXE%" (
	"%CPPCHECK_EXE%" --enable=all --xml --platform=%CPPCHECK_PLATFORM% %~dp0sakura_core 2> %CPPCHECK_OUT%
)
