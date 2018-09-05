@rem see readme.md
@echo off
set CMD_HHC=
set PATH_HHC_1=
set PATH_HHC_2=
set PATH_HHC_3=

if not "%ProgramFiles%"      == "" set "PATH_HHC_1=%ProgramFiles%\HTML Help Workshop\hhc.exe"
if not "%ProgramFiles(x86)%" == "" set "PATH_HHC_2=%ProgramFiles(x86)%\HTML Help Workshop\hhc.exe"
if not "%ProgramW6432%"      == "" set "PATH_HHC_3=%ProgramW6432%\HTML Help Workshop\hhc.exe"

set RESULT_PATH_HHC_1=--
set RESULT_PATH_HHC_2=--
set RESULT_PATH_HHC_3=--

if exist "%PATH_HHC_1%" (
	set RESULT_PATH_HHC_1=OK
	set "CMD_HHC=%PATH_HHC_1%"
) else if exist "%PATH_HHC_2%" (
	set RESULT_PATH_HHC_2=OK
	set "CMD_HHC=%PATH_HHC_2%"
) else if exist "%PATH_HHC_3%" (
	set RESULT_PATH_HHC_3=OK
	set "CMD_HHC=%PATH_HHC_3%"
)

@echo %RESULT_PATH_HHC_1% %PATH_HHC_1%
@echo %RESULT_PATH_HHC_2% %PATH_HHC_2%
@echo %RESULT_PATH_HHC_3% %PATH_HHC_3%
@echo.
@echo CMD_HHC "%CMD_HHC%"
@echo.
