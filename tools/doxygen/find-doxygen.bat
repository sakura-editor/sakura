@rem see readme.md
@echo off
set CMD_DOXYGEN=
set PATH_DOXYGEN_1=
set PATH_DOXYGEN_2=
set PATH_DOXYGEN_3=

if not "%ProgramFiles%"      == "" set "PATH_DOXYGEN_1=%ProgramFiles%\doxygen\bin\doxygen.exe"
if not "%ProgramFiles(x86)%" == "" set "PATH_DOXYGEN_2=%ProgramFiles(x86)%\doxygen\bin\doxygen.exe"
if not "%ProgramW6432%"      == "" set "PATH_DOXYGEN_3=%ProgramW6432%\doxygen\bin\doxygen.exe"

set RESULT_PATH_DOXYGEN_1=--
set RESULT_PATH_DOXYGEN_2=--
set RESULT_PATH_DOXYGEN_3=--

if exist "%PATH_DOXYGEN_1%" (
	set RESULT_PATH_DOXYGEN_1=OK
	set "CMD_DOXYGEN=%PATH_DOXYGEN_1%"
) else if exist "%PATH_DOXYGEN_2%" (
	set RESULT_PATH_DOXYGEN_2=OK
	set "CMD_DOXYGEN=%PATH_DOXYGEN_2%"
) else if exist "%PATH_DOXYGEN_3%" (
	set RESULT_PATH_DOXYGEN_3=OK
	set "CMD_DOXYGEN=%PATH_DOXYGEN_3%"
)

@echo %RESULT_PATH_DOXYGEN_1% %PATH_DOXYGEN_1%
@echo %RESULT_PATH_DOXYGEN_2% %PATH_DOXYGEN_2%
@echo %RESULT_PATH_DOXYGEN_3% %PATH_DOXYGEN_3%
@echo.
@echo CMD_DOXYGEN "%CMD_DOXYGEN%"
@echo.
