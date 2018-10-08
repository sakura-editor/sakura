# doxgyen `@file` コメント

<!-- TOC -->

- [doxgyen `@file` コメント](#doxgyen-file-コメント)
    - [概要](#概要)
    - [ファイル構成](#ファイル構成)

<!-- /TOC -->

## 概要

doxygen は解析対象かどうかを `@file` コメントの有無で認識します。
確実にすべてのファイルが解析対象となるようにするために @file コメントを
付与するスクリプトを提供します。

## ファイル構成

- `addDoxygenFileComment.py`  : スクリプト本体。第一引数で指定したフォルダ以下のソースに対して `@file` コメントがなければ先頭行に付与します。
- `addDoxygenFileComment.bat` : `sakura_core` の引数を渡して `addDoxygenFileComment.py` を実行します。
