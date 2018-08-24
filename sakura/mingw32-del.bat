@echo off

SETLOCAL

:引数で渡されたファイル群を取得
set OUTFILES=%*
:パス区切りを置換
set OUTFILES=%OUTFILES:/=\%


:del_file
:1ファイルずつdelコマンドに渡して削除
for /F "tokens=1,*" %%f in ("%OUTFILES%") DO (
  if exist %%f del /F /Q %%f
  set OUTFILES=%%g
)

:ぜんぶ削除できたら終了
if "%OUTFILES%" == "" goto :EOF
goto :del_file


:END

ENDLOCAL
exit /b
