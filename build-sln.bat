set platform=%1
set configuration=%2
set SLN_FILE=sakura.sln

@rem https://www.appveyor.com/docs/environment-variables/

if "%APPVEYOR%"=="True" (
    set EXTRA_CMD=/verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
) else (
    set EXTRA_CMD=
)

set LOG_FILE=msbuild-%platform%-%configuration%.log
@rem https://msdn.microsoft.com/ja-jp/library/ms171470.aspx
set LOG_OPTION=/flp:logfile=%LOG_FILE%

set MSBUILD_EXE="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"

@echo %MSBUILD_EXE% %SLN_FILE% /p:Platform=%platform% /p:Configuration=%configuration%      /t:"Clean","Rebuild"  %EXTRA_CMD% %LOG_OPTION%
      %MSBUILD_EXE% %SLN_FILE% /p:Platform=%platform% /p:Configuration=%configuration%      /t:"Clean","Rebuild"  %EXTRA_CMD% %LOG_OPTION%
if %errorlevel% neq 0 (echo error && exit /b 1)

@echo call parse-buildlog.bat %LOG_FILE%
      call parse-buildlog.bat %LOG_FILE%
if %errorlevel% neq 0 (echo error && exit /b 1)

exit /b 0
