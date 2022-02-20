@echo off

:: バッチファイル名から、処理対象のファイル名を取得する
set IMAGE_NAME=%~n0
set IMAGE_NAME=%IMAGE_NAME:~6%

:: ToolBarImageSplitter.exeの生成チェック
if not exist "%~dp0ToolBarImageSplitter\bin\Debug\ToolBarImageSplitter.exe" (
    call :BuildToolBarTools
    if not exist "%~dp0ToolBarImageSplitter\bin\Debug\ToolBarImageSplitter.exe" (
        pause
        exit /b
    )
)

:: ツール実行
pushd "%~dp0"
.\ToolBarImageSplitter\bin\Debug\ToolBarImageSplitter.exe ..\..\resource\%IMAGE_NAME%.bmp .\%IMAGE_NAME%

:: 実行結果を表示するため一時停止
pause

:: バッチ終了
exit /b

:: ツールのビルド
:BuildToolBarTools
echo ToolBarImageSplitter.exe was not found.
echo .

if not defined CMD_MSBUILD call %~dp0..\find-tools.bat
if not defined CMD_MSBUILD (
	echo msbuild.exe was not found.
	exit /b
)

echo "%CMD_MSBUILD%" ToolBarTools.sln /p:Platform="Any CPU" /p:Configuration=Debug /verbosity:quiet
     "%CMD_MSBUILD%" ToolBarTools.sln /p:Platform="Any CPU" /p:Configuration=Debug /verbosity:quiet
