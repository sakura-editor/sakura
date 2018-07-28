# Sakura Editor

<!-- TOC -->

- [Sakura Editor](#sakura-editor)
    - [Web Site](#web-site)
    - [開発参加ポリシー](#開発参加ポリシー)
    - [Build Requirements](#build-requirements)
        - [Visual Studio Community 2017](#visual-studio-community-2017)
            - [Visual Studio Install options required](#visual-studio-install-options-required)
    - [How to build](#how-to-build)
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

