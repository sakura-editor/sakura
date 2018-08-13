# Sakura Editor

<!-- TOC -->

- [Sakura Editor](#sakura-editor)
    - [Web Site](#web-site)
    - [開発参加ポリシー](#開発参加ポリシー)
    - [Build Requirements](#build-requirements)
        - [Visual Studio Community 2017](#visual-studio-community-2017)
            - [Visual Studio Install options required](#visual-studio-install-options-required)
    - [How to build](#how-to-build)
        - [インストーラの仕組み](#インストーラの仕組み)
        - [githash.h の更新のスキップ](#githashh-の更新のスキップ)
    - [CI Build (AppVeyor)](#ci-build-appveyor)
        - [ビルドの仕組み](#ビルドの仕組み)
        - [ビルド成果物を利用する上での注意事項](#ビルド成果物を利用する上での注意事項)
        - [ビルド成果物](#ビルド成果物)
        - [単体テスト](#単体テスト)

<!-- /TOC -->

[![Build status](https://ci.appveyor.com/api/projects/status/xlsp22h1q91mh96j/branch/master?svg=true)](https://ci.appveyor.com/project/sakuraeditor/sakura/branch/master)

A free Japanese text editor for Windows

## Web Site
- [Sakura Editor Portal](https://sakura-editor.github.io/)

## 開発参加ポリシー
開発ポリシーを以下にまとめていきます。開発にご参加いただける方はこちらご参照ください。  
https://github.com/sakura-editor/sakura/wiki

## Build Requirements
### Visual Studio Community 2017
- [Visual Studio Community 2017](https://www.visualstudio.com/downloads/)

#### Visual Studio Install options required
- Windows SDK
- Windows XP Support for C++
- C++に関するWindows XP サポート

More information: https://github.com/sakura-editor/sakura/issues/6

## How to build
Visual Studio Community 2017 で `sakura.sln` を開いてビルド。


### インストーラの仕組み

[こちら](installer/readme.md) でビルドの仕組みを説明しています。

### githash.h の更新のスキップ

sakura editor ではビルド時に git の commit hash 等の情報を githash.h というファイルに出力します。
ビルド時に commit hash 等を生成することでビルド済みのバイナリがどの commit hash を元にビルドされたか
簡単に判断できて便利なのですが、

バイナリが変化しないリファクタリングをしたときでも、commit hash 等の変更が原因でバイナリ一致しなくなります。
これだと検証が面倒になるので、ローカルビルドで githash.h が変化しない手段を提供します。

コマンドラインで環境変数 ```SKIP_CREATE_GITHASH``` を 1 に設定することにより commit hash の
更新処理をスキップすることができます。githash.h が存在しない場合には、この環境変数が設定されていても
githash.h を生成します。

コマンド実行例

```
set SKIP_CREATE_GITHASH=1
build-sln.bat Win32 Release
build-sln.bat Win32 Debug
build-sln.bat x64   Release
build-sln.bat x64   Debug
```

## CI Build (AppVeyor)

### ビルドの仕組み

[appveyor.md](appveyor.md) でビルドの仕組みを説明しています。

### ビルド成果物を利用する上での注意事項

[`x64 版は alpha 版`](installer/warning-alpha.txt)です。  
対応中のため予期せぬ不具合がある可能性があります。  

### ビルド成果物

本リポジトリの最新 master は以下の AppVeyor プロジェクト上で自動ビルドされます。  
https://ci.appveyor.com/project/sakuraeditor/sakura/branch/master

最新のビルド結果（バイナリ）はここから取得できます。  
https://ci.appveyor.com/project/sakuraeditor/sakura/branch/master/artifacts  
[`これ`](installer/warning.txt) を読んでからご利用ください。

最新以外のビルド結果は以下から参照できます。  
https://ci.appveyor.com/project/sakuraeditor/sakura/history

### 単体テスト

[単体テスト](unittest.md) を参照

