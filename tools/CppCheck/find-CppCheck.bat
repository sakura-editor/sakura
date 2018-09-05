@rem see readme.md
@echo off
set CMD_CPPCHECK=
set PATH_CPPCHECK_1=
set PATH_CPPCHECK_2=
set PATH_CPPCHECK_3=

if not "%ProgramFiles%"      == "" set "PATH_CPPCHECK_1=%ProgramFiles%\Cppcheck\cppcheck.exe"
if not "%ProgramFiles(x86)%" == "" set "PATH_CPPCHECK_2=%ProgramFiles(x86)%\Cppcheck\cppcheck.exe"
if not "%ProgramW6432%"      == "" set "PATH_CPPCHECK_3=%ProgramW6432%\Cppcheck\cppcheck.exe"

set RESULT_PATH_CPPCHECK_1=--
set RESULT_PATH_CPPCHECK_2=--
set RESULT_PATH_CPPCHECK_3=--

if exist "%PATH_CPPCHECK_1%" (
	set RESULT_PATH_CPPCHECK_1=OK
	set "CMD_CPPCHECK=%PATH_CPPCHECK_1%"
) else if exist "%PATH_CPPCHECK_2%" (
	set RESULT_PATH_CPPCHECK_2=OK
	set "CMD_CPPCHECK=%PATH_CPPCHECK_2%"
) else if exist "%PATH_CPPCHECK_3%" (
	set RESULT_PATH_CPPCHECK_3=OK
	set "CMD_CPPCHECK=%PATH_CPPCHECK_3%"
)

@echo %RESULT_PATH_CPPCHECK_1% %PATH_CPPCHECK_1%
@echo %RESULT_PATH_CPPCHECK_2% %PATH_CPPCHECK_2%
@echo %RESULT_PATH_CPPCHECK_3% %PATH_CPPCHECK_3%
@echo.
@echo CMD_CPPCHECK "%CMD_CPPCHECK%"
@echo.
