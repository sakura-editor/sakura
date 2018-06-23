set platform=%1
set configuration=%2
set WORKDIR=sakura-%platform%-%configuration%
set WORKDIR_EXE=%WORKDIR%\EXE
set WORKDIR_INST=%WORKDIR%\Installer
set OUTFILE=sakura-%platform%-%configuration%.zip

@rem cleanup for local testing
if exist "%OUTFILE%" (
	del %OUTFILE%
)
if exist "%WORKDIR%" (
	rmdir /s /q %WORKDIR%
)

mkdir %WORKDIR%
mkdir %WORKDIR_EXE%
mkdir %WORKDIR_INST%
copy %platform%\%configuration%\*.exe %WORKDIR_EXE%\
copy %platform%\%configuration%\*.dll %WORKDIR_EXE%\
copy %platform%\%configuration%\*.pdb %WORKDIR_EXE%\

copy help\macro\macro.chm    %WORKDIR_EXE%\
copy help\plugin\plugin.chm  %WORKDIR_EXE%\
copy help\sakura\sakura.chm  %WORKDIR_EXE%\

copy installer\warning.txt   %WORKDIR%\
copy installer\Output\*.exe  %WORKDIR_INST%\

7z a %OUTFILE%  -r %WORKDIR%
7z l %OUTFILE%

if exist %WORKDIR% (
	rmdir /s /q %WORKDIR%
)
