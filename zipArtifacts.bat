set platform=%1
set configuration=%2
if "%platform%" == "x64" (
	set ALPHA=1
) else (
	set ALPHA=0
)
if "%ALPHA%" == "1" (
	set SUFFIX=-alpha
) else (
	set SUFFIX=""
)
set BASENAME=sakura-%platform%-%configuration%%SUFFIX%
set WORKDIR=%BASENAME%
set WORKDIR_LOG=%WORKDIR%\Log
set WORKDIR_EXE=%WORKDIR%\EXE
set WORKDIR_INST=%WORKDIR%\Installer
set OUTFILE=%BASENAME%.zip

@rem cleanup for local testing
if exist "%OUTFILE%" (
	del %OUTFILE%
)
if exist "%WORKDIR%" (
	rmdir /s /q %WORKDIR%
)

mkdir %WORKDIR%
mkdir %WORKDIR_LOG%
mkdir %WORKDIR_EXE%
mkdir %WORKDIR_INST%
copy %platform%\%configuration%\*.exe %WORKDIR_EXE%\
copy %platform%\%configuration%\*.dll %WORKDIR_EXE%\
copy %platform%\%configuration%\*.pdb %WORKDIR_EXE%\

copy help\macro\macro.chm    %WORKDIR_EXE%\
copy help\plugin\plugin.chm  %WORKDIR_EXE%\
copy help\sakura\sakura.chm  %WORKDIR_EXE%\

copy installer\warning.txt   %WORKDIR%\
if "%ALPHA%" == "1" (
	copy installer\warning-alpha.txt   %WORKDIR%\
)
copy installer\Output\*.exe  %WORKDIR_INST%\
copy msbuild-%platform%-%configuration%.log %WORKDIR_LOG%\
copy sakura_core\gitHash.h                  %WORKDIR_LOG%\

7z a %OUTFILE%  -r %WORKDIR%
7z l %OUTFILE%

if exist %WORKDIR% (
	rmdir /s /q %WORKDIR%
)
