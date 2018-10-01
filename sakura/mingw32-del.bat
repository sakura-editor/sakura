@echo off

SETLOCAL

set OUTFILES=%*
set OUTFILES=%OUTFILES:/=\%


:del_file
for /F "tokens=1,*" %%f in ("%OUTFILES%") DO (
  if exist %%f del /F /Q %%f
  set OUTFILES=%%g
)

if "%OUTFILES%" == "" goto :END
goto :del_file


:END
ENDLOCAL
exit /b
