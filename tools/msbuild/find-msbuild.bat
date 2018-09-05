@rem see readme.md
@echo off
set CMD_MSBUILD=
set PATH_MSBUILD_1=
set PATH_MSBUILD_2=
set PATH_MSBUILD_3=

if not "%ProgramFiles%"      == "" set "PATH_MSBUILD_1=%ProgramFiles%\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"
if not "%ProgramFiles(x86)%" == "" set "PATH_MSBUILD_2=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"
if not "%ProgramW6432%"      == "" set "PATH_MSBUILD_3=%ProgramW6432%\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"

set RESULT_PATH_MSBUILD_1=--
set RESULT_PATH_MSBUILD_2=--
set RESULT_PATH_MSBUILD_3=--

if exist "%PATH_MSBUILD_1%" (
	set RESULT_PATH_MSBUILD_1=OK
	set "CMD_MSBUILD=%PATH_MSBUILD_1%"
) else if exist "%PATH_MSBUILD_2%" (
	set RESULT_PATH_MSBUILD_2=OK
	set "CMD_MSBUILD=%PATH_MSBUILD_2%"
) else if exist "%PATH_MSBUILD_3%" (
	set RESULT_PATH_MSBUILD_3=OK
	set "CMD_MSBUILD=%PATH_MSBUILD_3%"
)

@echo %RESULT_PATH_MSBUILD_1% %PATH_MSBUILD_1%
@echo %RESULT_PATH_MSBUILD_2% %PATH_MSBUILD_2%
@echo %RESULT_PATH_MSBUILD_3% %PATH_MSBUILD_3%
@echo.
@echo CMD_MSBUILD "%CMD_MSBUILD%"
@echo.
