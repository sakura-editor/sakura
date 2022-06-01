@echo off

set IMAGE_NAME=%1
if not defined IMAGE_NAME (
    set /p IMAGE_NAME="Enter file name of bitmap to be splited>>>"
)
if not defined IMAGE_NAME (
    echo file name must not be empty.
    pause
    exit /b 1
)

:: ToolBarImageSplitter.exeの生成チェック
if not exist "%~dp0bin\Debug\ToolBarImageSplitter.exe" (
    call :BuildToolBarTools
    if not exist "%~dp0bin\Debug\ToolBarImageSplitter.exe" (
        pause
        exit /b
    )
)

:: ツール実行
pushd "%~dp0../"
"%~dp0bin\Debug\ToolBarImageSplitter.exe" ..\..\resource\%IMAGE_NAME%.bmp .\%IMAGE_NAME%

:: 実行結果を表示するため一時停止
pause

:: バッチ終了
exit /b

:: ツールのビルド
:BuildToolBarTools
echo ToolBarImageSplitter.exe was not found.
echo .

if not defined CMD_MSBUILD call %~dp0..\..\find-tools.bat
if not defined CMD_MSBUILD (
	echo msbuild.exe was not found.
	exit /b
)

pushd "%~dp0../"
echo "%CMD_MSBUILD%" ToolBarTools.sln /p:Platform="Any CPU" /p:Configuration=Debug /verbosity:quiet
     "%CMD_MSBUILD%" ToolBarTools.sln /p:Platform="Any CPU" /p:Configuration=Debug /verbosity:quiet
