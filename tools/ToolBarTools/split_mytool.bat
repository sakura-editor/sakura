@echo off

:: バッチファイル名から、処理対象のファイル名を取得する
set IMAGE_NAME=%~n0
set IMAGE_NAME=%IMAGE_NAME:~6%

:: 共通バッチを呼び出す
call %~dp0ToolBarImageSplitter\runToolBarImageSplitter.bat %IMAGE_NAME%
