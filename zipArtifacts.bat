set platform=%1
set configuration=%2
set WORKDIR=sakura-%platform%-%configuration%
set OUTFILE=sakura-%platform%-%configuration%.zip

@rem cleanup for local testing
if exist "%OUTFILE%" (
	del %OUTFILE%
)
if exist "%WORKDIR%" (
	rmdir /s /q %WORKDIR%
)

mkdir %WORKDIR%
copy %platform%\%configuration%\*.exe %WORKDIR%\
copy %platform%\%configuration%\*.dll %WORKDIR%\
copy %platform%\%configuration%\*.pdb %WORKDIR%\

copy help\macro\macro.chm    %WORKDIR%\
copy help\plugin\plugin.chm  %WORKDIR%\
copy help\sakura\sakura.chm  %WORKDIR%\

copy installer\warning.txt   %WORKDIR%\
copy installer\Output\*.exe  %WORKDIR%\

7z a %OUTFILE%  -r %WORKDIR%
7z l %OUTFILE%

if exist %WORKDIR% (
	rmdir /s /q %WORKDIR%
)
