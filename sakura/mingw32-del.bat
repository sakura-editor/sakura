@echo off

SETLOCAL

:引数で渡されたファイル群のパス区切りを置換してdelコマンドに渡す
set OUTFILES=%*
del /F /Q %OUTFILES:/=\%

ENDLOCAL
exit /b
