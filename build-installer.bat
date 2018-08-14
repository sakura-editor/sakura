set platform=%1
set configuration=%2
set INSTALLER_RESOURCES=installer\temp
set INSTALLER_WORK=installer\sakura
set INSTALLER_OUTPUT=installer\Output-%platform%

set INSTALLER_RESOURCES_SINT=installer\sinst_src
set INSTALLER_RESOURCES_BRON=installer\temp\bron

if exist "%INSTALLER_RESOURCES%" rmdir /s /q "%INSTALLER_RESOURCES%"
if exist "%INSTALLER_WORK%"      rmdir /s /q "%INSTALLER_WORK%"
if exist "%INSTALLER_OUTPUT%"    rmdir /s /q "%INSTALLER_OUTPUT%"

if exist "%INSTALLER_RESOURCES_BRON%" rmdir /s /q "%INSTALLER_RESOURCES_BRON%"

7z x installer\externals\bregonig\bron412.zip  -O"%INSTALLER_RESOURCES_BRON%" || (echo error && exit /b 1)

mkdir %INSTALLER_WORK%
mkdir %INSTALLER_WORK%\keyword

copy %INSTALLER_RESOURCES_SINT%\sakura.exe.manifest.x %INSTALLER_WORK%\
copy %INSTALLER_RESOURCES_SINT%\sakura.exe.manifest.v %INSTALLER_WORK%\
copy %INSTALLER_RESOURCES_SINT%\sakura.exe.ini        %INSTALLER_WORK%\
copy %INSTALLER_RESOURCES_SINT%\keyword\*.*           %INSTALLER_WORK%\keyword\

set DLL_BREGONIG_NAME=bregonig.dll
if "%platform%" == "x64" (
	set INSTALLER_RESOURCES_BRON_DLL=%INSTALLER_RESOURCES_BRON%\x64
) else (
	set INSTALLER_RESOURCES_BRON_DLL=%INSTALLER_RESOURCES_BRON%
)
copy %INSTALLER_RESOURCES_BRON_DLL%\*.dll             %INSTALLER_WORK%\
copy %INSTALLER_RESOURCES_BRON%\*.txt                 %INSTALLER_WORK%\

copy help\sakura\sakura.chm                      %INSTALLER_WORK%\
copy help\plugin\plugin.chm                      %INSTALLER_WORK%\
copy help\macro\macro.chm                        %INSTALLER_WORK%\

copy %platform%\%configuration%\*.exe                 %INSTALLER_WORK%\
copy %platform%\%configuration%\sakura_lang_en_US.dll %INSTALLER_WORK%\

set DLL_BREGONIG_0=%INSTALLER_RESOURCES_BRON_DLL%\%DLL_BREGONIG_NAME%
set DLL_BREGONIG_1=%INSTALLER_WORK%\%DLL_BREGONIG_NAME%
set DLL_BREGONIG_2=installer\externals\bregonig\%platform%\%DLL_BREGONIG_NAME%
set DLL_BREGONIG_3=%platform%\%configuration%\%DLL_BREGONIG_NAME%

FOR %%a IN (%DLL_BREGONIG_0%) DO SET TIMESTAMP_0=%%~ta
FOR %%b IN (%DLL_BREGONIG_1%) DO SET TIMESTAMP_1=%%~tb
FOR %%c IN (%DLL_BREGONIG_2%) DO SET TIMESTAMP_2=%%~tc
FOR %%d IN (%DLL_BREGONIG_3%) DO SET TIMESTAMP_3=%%~td

@echo %TIMESTAMP_0% %DLL_BREGONIG_0%
@echo %TIMESTAMP_1% %DLL_BREGONIG_1%
@echo %TIMESTAMP_2% %DLL_BREGONIG_2%
@echo %TIMESTAMP_3% %DLL_BREGONIG_3%

@rem compare file contents
set COMPARE_RESULT=0
fc /B %DLL_BREGONIG_0% %DLL_BREGONIG_1% 1>nul 2>&1
if "%ERRORLEVEL%" == "0" (
	@echo %DLL_BREGONIG_0% and %DLL_BREGONIG_1% for %platform%: matched
) else (
	@echo %DLL_BREGONIG_0% and %DLL_BREGONIG_1% for %platform%: unmatched
	set COMPARE_RESULT=1
)

fc /B %DLL_BREGONIG_0% %DLL_BREGONIG_2% 1>nul 2>&1
if "%ERRORLEVEL%" == "0" (
	@echo %DLL_BREGONIG_0% and %DLL_BREGONIG_2% for %platform%: matched
) else (
	@echo %DLL_BREGONIG_0% and %DLL_BREGONIG_2% for %platform%: unmatched
	set COMPARE_RESULT=1
)

fc /B %DLL_BREGONIG_0% %DLL_BREGONIG_3% 1>nul 2>&1
if "%ERRORLEVEL%" == "0" (
	@echo %DLL_BREGONIG_0% and %DLL_BREGONIG_3% for %platform%: matched
) else (
	@echo %DLL_BREGONIG_0% and %DLL_BREGONIG_3% for %platform%: unmatched
	set COMPARE_RESULT=1
)

if "%COMPARE_RESULT%" == "1" (
	echo unmatch file contents
	exit /b 1
)
@rem compare timestamps
if "%TIMESTAMP_0%" == "%TIMESTAMP_1%" (
	@echo %DLL_BREGONIG_0% and %DLL_BREGONIG_1% for %platform%: timestamps matched
) else (
	@echo %DLL_BREGONIG_0% and %DLL_BREGONIG_1% for %platform%: timestamps unmatched
	set COMPARE_RESULT=1
)

if "%COMPARE_RESULT%" == "1" (
	echo unmatch timestamps
	exit /b 1
)

set SAKURA_ISS=installer\sakura-%platform%.iss
"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" %SAKURA_ISS% || (echo error && exit /b 1)
