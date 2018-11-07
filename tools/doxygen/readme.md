<!-- TOC -->

- [doxygen のパスを見つけるバッチファイル](#doxygen-のパスを見つけるバッチファイル)
    - [ロジック](#ロジック)
<!-- /TOC -->

# doxygen のパスを見つけるバッチファイル

doxygen.exe のパスを見つけて 環境変数 `CMD_DOXYGEN` に設定する

## ロジック

以下の順番でパスを検索して、見つかったパスを環境変数 `CMD_DOXYGEN` にセットする。

- %ProgramFiles%\doxygen\bin\doxygen.exe
- %ProgramFiles(x86)%\doxygen\bin\doxygen.exe
- %ProgramW6432%\doxygen\bin\doxygen.exe
