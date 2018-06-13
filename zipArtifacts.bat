set platform=%1
set configuration=%2
set OUTFILE=archive-%platform%-%configuration%.zip

7z a %OUTFILE% %platform%\%configuration%\*.exe
7z a %OUTFILE% %platform%\%configuration%\*.dll
7z a %OUTFILE% %platform%\%configuration%\*.pdb
