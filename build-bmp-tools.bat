set configuration=%1
if "%configuration%" == "Release" (
	@rem OK
) else if "%configuration%" == "Debug" (
	@rem OK
) else (
	call :showhelp %0
	exit /b 1
)

if not defined CMD_MSBUILD call %~dp0tools\find-tools.bat
if not defined CMD_MSBUILD (
	echo msbuild.exe was not found.
	exit /b 1
)

set "TOOL_SLN_FILE=%~dp0tools\ToolBarTools\ToolBarTools.sln"
@echo "%CMD_MSBUILD%" %TOOL_SLN_FILE% "/p:Platform=Any CPU" /p:Configuration=%configuration% /t:"Build" /v:q
      "%CMD_MSBUILD%" %TOOL_SLN_FILE% "/p:Platform=Any CPU" /p:Configuration=%configuration% /t:"Build" /v:q
if errorlevel 1 (echo error && exit /b 1)

set MUXER=%~dp0tools\ToolBarTools\ToolBarImageMuxer\bin\%configuration%\ToolBarImageMuxer.exe
set SPLITTER=%~dp0tools\ToolBarTools\ToolBarImageSplitter\bin\%configuration%\ToolBarImageSplitter.exe

set SRC_BMP1=%~dp0resource\mytool.bmp
set SRC_BMP2=%~dp0resource\my_icons.bmp

set DST_DIR1=%~dp0resource\mytool
set DST_DIR2=%~dp0resource\my_icons

if exist %DST_DIR1% rmdir /s /q %DST_DIR1%
if exist %DST_DIR2% rmdir /s /q %DST_DIR2%
if exist %DST_DIR1% rmdir /s /q %DST_DIR1%
if exist %DST_DIR2% rmdir /s /q %DST_DIR2%

@rem split input bmp
%SPLITTER% %SRC_BMP1% %DST_DIR1%
if errorlevel 1 (echo error && exit /b 1)

%SPLITTER% %SRC_BMP2% %DST_DIR2%
if errorlevel 1 (echo error && exit /b 1)

@rem these steps are for tests.
set OUT1_BMP1=%~dp0resource\out1-mytool.bmp
set OUT1_BMP2=%~dp0resource\out1-my_icons.bmp
set OUT2_BMP1=%~dp0resource\out2-mytool.bmp
set OUT2_BMP2=%~dp0resource\out2-my_icons.bmp

set OUT_DIR1=%~dp0resource\mytool
set OUT_DIR2=%~dp0resource\my_icons

@rem merge multiple bmp to one bmp
%MUXER% %DST_DIR1% %OUT1_BMP1%
if errorlevel 1 (echo error && exit /b 1)

%MUXER% %DST_DIR2% %OUT1_BMP2%
if errorlevel 1 (echo error && exit /b 1)

%SPLITTER% %OUT1_BMP1% %OUT_DIR1%
if errorlevel 1 (echo error && exit /b 1)

%SPLITTER% %OUT1_BMP2% %OUT_DIR2%
if errorlevel 1 (echo error && exit /b 1)

%MUXER% %OUT_DIR1% %OUT2_BMP1%
if errorlevel 1 (echo error && exit /b 1)

%MUXER% %OUT_DIR2% %OUT2_BMP2%
if errorlevel 1 (echo error && exit /b 1)

fc /a /b %OUT1_BMP1% %OUT2_BMP1% >NUL
if errorlevel 1 (echo error && exit /b 1)

fc /a /b %OUT1_BMP2% %OUT2_BMP2% >NUL
if errorlevel 1 (echo error && exit /b 1)

exit /b 0

@rem ------------------------------------------------------------------------------
@rem show help
@rem see http://orangeclover.hatenablog.com/entry/20101004/1286120668
@rem ------------------------------------------------------------------------------
:showhelp
@echo off
@echo usage
@echo    %~nx1 configuration
@echo.
@echo parameter
@echo    configuration : Release or Debug
@echo.
@echo example
@echo    %~nx1 Release
@echo    %~nx1 Debug
exit /b 0
