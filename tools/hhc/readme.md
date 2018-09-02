<!-- TOC -->

- [HTML Help Compiler のパスを見つけるバッチファイル](#html-help-compiler-のパスを見つけるバッチファイル)
    - [ロジック](#ロジック)
    - [バッチファイルでのはまりポイント](#バッチファイルでのはまりポイント)

<!-- /TOC -->

# HTML Help Compiler のパスを見つけるバッチファイル

hhc.exe のパスを見つけて 環境変数 `CMD_HHC` に設定する

## ロジック

以下の順番でパスを検索して、見つかったパスを環境変数 `CMD_HHC` にセットする。
CMD_HHC にはダブルクオートを含む値が設定される。

- %ProgramFiles%\HTML Help Workshop\hhc.exe
- %ProgramFiles(x86)%\HTML Help Workshop\hhc.exe
- %ProgramW6432%\HTML Help Workshop\hhc.exe

## バッチファイルでのはまりポイント

たとえば以下のようなバッチファイルでの記述の場合

```
if not "%ProgramFiles(x86)%" == "" set HHC_PATH2=%ProgramFiles(x86)%\HTML Help Workshop\hhc.exe
if exist "%HHC_PATH2%" (
	set CMD_HHC=%HHC_PATH2%
)
```

以下のような意味不明のエラーメッセージが出る

```
\HTML の使い方が誤っています。
```

上記のバッチファイルは実行時に以下のような内容になる。

```
if exist "C:\Program Files (x86)\HTML Help Workshop\hhc.exe" (
	set CMD_HHC=C:\Program Files (x86)\HTML Help Workshop\hhc.exe
)
```

if の構文の `(` が `C:\Program Files (x86)\HTML Help Workshop\hhc.exe` の `)` と対応すると
解釈されるので `\HTML` が if の条件が成立したときのコマンドと認識されるのでエラーになる。

このエラーを回避するために以下のように環境変数 `HHC_PATH2` の値を代入するときにダブルクオートで囲む。
結果として 環境変数 `CMD_HHC` にはダブルクオートを含む値が設定されるが、仕方ない。

```
if exist "%HHC_PATH2%" (
	set CMD_HHC="%HHC_PATH2%"
)
```
