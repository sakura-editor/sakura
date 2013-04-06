@echo off

REM 指定ファイルのタイムスタンプを現在時刻に更新する
REM VC++2005/2008 で不必要にビルドの問い合わせが出るのを回避するため、
REM ビルド後のイベントで .manifest の時刻を更新するのに使用。
REM ※問い合わせを減らしたいだけなのでエラーがあっても正常終了の 0 で終了

if %1.==. goto end
if not exist %1 goto end
echo touch %1
copy /b %1+,,%1 > nul
:end
exit 0
