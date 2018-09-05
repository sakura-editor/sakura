<!-- TOC -->

- [cppcheck のパスを見つけるバッチファイル](#cppcheck-のパスを見つけるバッチファイル)
    - [ロジック](#ロジック)
    - [バッチファイルでのはまりポイント](#バッチファイルでのはまりポイント)

<!-- /TOC -->

# cppcheck のパスを見つけるバッチファイル

cppcheck.exe のパスを見つけて 環境変数 `CMD_CPPCHECK` に設定する

## ロジック

以下の順番でパスを検索して、見つかったパスを環境変数 `CMD_CPPCHECK` にセットする。

- %ProgramFiles%\Cppcheck\cppcheck.exe
- %ProgramFiles(x86)%\Cppcheck\cppcheck.exe
- %ProgramW6432%\Cppcheck\cppcheck.exe

## バッチファイルでのはまりポイント

[hhc.exe の readme](../hhc/readme.md) を参照
