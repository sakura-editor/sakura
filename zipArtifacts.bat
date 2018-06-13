set platform=%1
set configuration=%2
set OUTFILE=sakura-%platform%-%configuration%.zip

7z a %OUTFILE% %platform%\%configuration%\*.exe
7z a %OUTFILE% %platform%\%configuration%\*.dll
7z a %OUTFILE% %platform%\%configuration%\*.pdb
7z l %OUTFILE%
