<!-- TOC -->

- [msbuild のパスを見つけるバッチファイル](#msbuild-のパスを見つけるバッチファイル)
    - [ロジック](#ロジック)
    - [バッチファイルでのはまりポイント](#バッチファイルでのはまりポイント)

<!-- /TOC -->

# msbuild のパスを見つけるバッチファイル

msbuild.exe のパスを見つけて 環境変数 `CMD_MSBUILD` に設定する

## ロジック

以下の順番でパスを検索して、見つかったパスを環境変数 `CMD_MSBUILD` にセットする。

- %ProgramFiles%\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe
- %ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe
- %ProgramW6432%\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe

## バッチファイルでのはまりポイント

[hhc.exe の readme](../hhc/readme.md) を参照
