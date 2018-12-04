@echo off
setlocal ENABLEDELAYEDEXPANSION

rem Definitions & Dependencies

set RECIPE=%~1
set SRC=%~2
set DST=%~3
set TAB=	
set UNZIP_BAT=%~dp0zip\unzip.bat

rem Processor object

set WORKING_ZIP=
set WORKING_PATH=.\
set WORKING_FILE=

goto :end_Processor

:OnZip
	set WORKING_ZIP=
	set WORKING_PATH=.\
	set WORKING_FILE=

	setlocal ENABLEDELAYEDEXPANSION

	set rpnx1=%~dpnx1
	set rpnx1=!rpnx1:%CD%\=!

	@echo ZIP  %rpnx1%

	rem Prepare working directory for a zip.
	mkdir 2>nul  "%DST%\%rpnx1%"
	if not exist "%DST%\%rpnx1%" (
		exit /b 1
	)

	endlocal & set WORKING_ZIP=%rpnx1%
exit /b 0

:OnPath
	setlocal ENABLEDELAYEDEXPANSION

	set rp1=%~dp1
	set rp1=!rp1:%CD%\=!

	@echo PATH %rp1%

	rem Unfinished preparation.
	if not exist "%DST%\%WORKING_ZIP%" (
		@echo>&2 ERROR: Missing directory: !DST!\!WORKING_ZIP!
		exit /b 1
	)

	rem Prepare working directory for a path.
	mkdir 2>nul  "%DST%\%WORKING_ZIP%\%rp1%"
	if not exist "%DST%\%WORKING_ZIP%\%rp1%" (
		exit /b 1
	)

	endlocal & set WORKING_PATH=%rp1%& set WORKING_FILE=%~nx1
exit /b 0

:OnFile
	setlocal ENABLEDELAYEDEXPANSION

	@echo FILE %~1

	rem Unfinished preparation.
	if not exist "%DST%\%WORKING_ZIP%\%WORKING_PATH%" (
		@echo>&2 ERROR: Missing directory: !DST!\!WORKING_ZIP!\!WORKING_PATH!
		exit /b 1
	)

	rem Prepare working file.
	set SourcePath=%SRC%\%~1
	call :TryUnzipPath SourcePath
	if not "%SourcePath%" == "%SRC%\%~1" echo FILE %SourcePath%
	copy /Y /B "%SourcePath%" "%DST%\%WORKING_ZIP%\%WORKING_PATH%%WORKING_FILE%"
exit /b 0

:end_Processor

rem Main loop

for /F "usebackq tokens=* eol=# delims=" %%L in ("%RECIPE%" 'FINISHED') do (
	rem Prevent the next 'for' command from merging empty columns.
	set L=%%L%TAB%%TAB%%TAB%
	set L=!L:%TAB%= %TAB%!
for /F "usebackq tokens=1,2,3 delims=%TAB%" %%A in ('!L!') do (
	rem First column: Zip name (relative to %DST% for working & relative to %SRC% for output)
	call :OnColumn "%%~A" OnZip^
	|| (call :Clean & exit /b 1)

	rem Second column: Path (destination, relative to Zip)		
	call :OnColumn "%%~B" OnPath^
	|| (call :Clean & exit /b 1)

	rem Third column: File (source, relative to %SRC%)		
	call :OnColumn "%%~C" OnFile^
	|| (call :Clean & exit /b 1)
))
call :Clean & exit /b 0

:OnColumn
	setlocal
	set C=%~1
	call :Trim C
	endlocal & if not "%C%" == "" call :%2 "%C%"^
	|| exit /b 1
exit /b 0

:Clean
	if exist "%LastZipDir%" rmdir /S /Q "%LastZipDir%"
	if defined WORKING_ZIP  rmdir /S /Q "%DST%\%WORKING_ZIP%" 2>nul
	rmdir /Q "%DST%" 2>nul
exit /b 0

rem -------------------------------------------------------

:TryUnzipPath
	setlocal ENABLEDELAYEDEXPANSION

	set VAR=%~1
	if not defined VAR exit /b 1
	set VAL=!%VAR%!
	if not defined VAL exit /b 1
	if exist "%VAL%"   exit /b 1

	set L=
	set R=%VAL%
	:continue
	for /F "tokens=1,* delims=\" %%P in ("%R%") do (
		if not exist "!L!%%P" goto :break
		set L=!L!%%P\
		set R=%%Q
		goto :continue
	)
	:break
	if not defined L exit /b 1
	if exist "%L%" (
		rem %L% has a trailing \ char, so this is
		rem testing a directory existense.
		rem But the test is not always correct.
		rem If %L% is a path to a file under NTFS Junction...
		exit /b 1
	)

	set Zip=%L:~0,-1%
	set ZipDir=%TEMP%\%Zip::=%
	set VAL=%ZipDir%\%R%

	if not exist "%ZipDir%" (
		@echo Unzipping !Zip!.
		call | "!UNZIP_BAT!" "!Zip!" "!ZipDir!"^
		|| exit /b 1
	) else (
		@echo Destination folder has already existed. Skip unzipping.
		@echo Destination: !ZipDir!
	)
	if not "%LastZipDir%" == "%ZipDir%" if exist "%LastZipDir%" (
		@echo Clean the last unzipped temporary folder.
		@echo Cleaning !LastZipDir!.
		rmdir /S /Q "!LastZipDir!"
	)

	endlocal & set LastZipDir=%ZipDir%& set %VAR%=%VAL%
exit /b 0

rem -------------------------------------------------------

:Trim
	setlocal ENABLEDELAYEDEXPANSION

	set VAR=%~1
	if not defined VAR exit /b 1
	set VAL=!%VAR%!
	if not defined VAL exit /b 0

	call :Set_TRIMMED %VAL%

	endlocal & set %VAR%=%TRIMMED%
exit /b 0

:Set_TRIMMED
	set TRIMMED=%*
exit /b 0
