<!-- TOC -->

- [Inno Setup Compiler のパスを見つけるバッチファイル](#inno-setup-compiler-のパスを見つけるバッチファイル)
    - [ロジック](#ロジック)
    - [バッチファイルでのはまりポイント](#バッチファイルでのはまりポイント)

<!-- /TOC -->

# Inno Setup Compiler のパスを見つけるバッチファイル

ISCC.exe のパスを見つけて 環境変数 `CMD_ISCC` に設定する

## ロジック

以下の順番でパスを検索して、見つかったパスを環境変数 `CMD_ISCC` にセットする。

- %ProgramFiles%\Inno Setup 5\ISCC.exe
- %ProgramFiles(x86)%\Inno Setup 5\ISCC.exe
- %ProgramW6432%\Inno Setup 5\ISCC.exe

## バッチファイルでのはまりポイント

[hhc.exe の readme](../hhc/readme.md) を参照
