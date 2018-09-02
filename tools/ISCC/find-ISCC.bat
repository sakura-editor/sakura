@rem see readme.md
@echo off
set CMD_ISCC=
set PATH_ISCC_1=
set PATH_ISCC_2=
set PATH_ISCC_3=

if not "%ProgramFiles%"      == "" set "PATH_ISCC_1=%ProgramFiles%\Inno Setup 5\ISCC.exe"
if not "%ProgramFiles(x86)%" == "" set "PATH_ISCC_2=%ProgramFiles(x86)%\Inno Setup 5\ISCC.exe"
if not "%ProgramW6432%"      == "" set "PATH_ISCC_3=%ProgramW6432%\Inno Setup 5\ISCC.exe"

set RESULT_PATH_ISCC_1=--
set RESULT_PATH_ISCC_2=--
set RESULT_PATH_ISCC_3=--

if exist "%PATH_ISCC_1%" (
	set RESULT_PATH_ISCC_1=OK
	set "CMD_ISCC=%PATH_ISCC_1%"
) else if exist "%PATH_ISCC_2%" (
	set RESULT_PATH_ISCC_2=OK
	set "CMD_ISCC=%PATH_ISCC_2%"
) else if exist "%PATH_ISCC_3%" (
	set RESULT_PATH_ISCC_3=OK
	set "CMD_ISCC=%PATH_ISCC_3%"
)

@echo %RESULT_PATH_ISCC_1% %PATH_ISCC_1%
@echo %RESULT_PATH_ISCC_2% %PATH_ISCC_2%
@echo %RESULT_PATH_ISCC_3% %PATH_ISCC_3%
@echo.
@echo CMD_ISCC "%CMD_ISCC%"
@echo.
