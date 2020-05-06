# Sakura Editor
[![Build status](https://ci.appveyor.com/api/projects/status/xlsp22h1q91mh96j/branch/master?svg=true)](https://ci.appveyor.com/project/sakuraeditor/sakura/branch/master)
[![Github Releases All](https://img.shields.io/github/downloads/sakura-editor/sakura/total.svg)](https://github.com/sakura-editor/sakura/releases "All Releases")
[![License: Zlib](https://img.shields.io/badge/License-Zlib-lightgrey.svg)](https://opensource.org/licenses/Zlib)
[![CodeFactor](https://www.codefactor.io/repository/github/sakura-editor/sakura/badge)](https://www.codefactor.io/repository/github/sakura-editor/sakura)
[![Build Status](https://dev.azure.com/sakuraeditor/sakura/_apis/build/status/sakura-editor.sakura?branchName=master)](https://dev.azure.com/sakuraeditor/sakura/_build/latest?definitionId=3&branchName=master)
[![build sakura](https://github.com/sakura-editor/sakura/workflows/build%20sakura/badge.svg)](https://github.com/sakura-editor/sakura/actions?query=workflow%3A%22build+sakura%22)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=sakura-editor_sakura&metric=alert_status)](https://sonarcloud.io/dashboard?id=sakura-editor_sakura)
[![Star History](https://img.shields.io/badge/star-histroy-yellow.svg)](https://star-history.t9t.io/#sakura-editor/sakura)

<!-- 以下は Markdownの参照形式によるリンク の定義です。 -->
<!-- 参照 https://hail2u.net/blog/coding/markdown-reference-style-links.html -->

[Visual Studio Community 2017]: https://visualstudio.microsoft.com/vs/older-downloads/ "Visual Studio Community 2017"
[Visual Studio Community 2019]: https://visualstudio.microsoft.com/ja/downloads/ "Visual Studio Community 2019"
[Markdown をローカルで確認する方法]: https://github.com/sakura-editor/sakura/wiki/markdown-%E3%82%92%E3%83%AD%E3%83%BC%E3%82%AB%E3%83%AB%E3%81%A7%E7%A2%BA%E8%AA%8D%E3%81%99%E3%82%8B%E6%96%B9%E6%B3%95
[How to extract currently installed Visual Studio component IDs?]: https://stackoverflow.com/questions/52946333/how-to-extract-currently-installed-visual-studio-component-ids
[Configure Visual Studio across your organization with .vsconfig]: https://devblogs.microsoft.com/setup/configure-visual-studio-across-your-organization-with-vsconfig/
[インストール構成をインポートまたはエクスポートする]: https://docs.microsoft.com/ja-jp/visualstudio/install/import-export-installation-configurations?view=vs-2019
[コマンド ライン パラメーターを使用して Visual Studio をインストールする]: https://docs.microsoft.com/ja-jp/visualstudio/install/use-command-line-parameters-to-install-visual-studio?view=vs-2019
[不足しているコンポーネントを自動的にインストールする]: https://docs.microsoft.com/ja-jp/visualstudio/install/import-export-installation-configurations?view=vs-2019#automatically-install-missing-components

<!-- TOC -->

- [Sakura Editor](#sakura-editor)
    - [Hot topic](#hot-topic)
    - [Web Site](#web-site)
    - [開発参加ポリシー](#開発参加ポリシー)
    - [Build Requirements](#build-requirements)
        - [Visual Studio Install options required](#visual-studio-install-options-required)
        - [Visual Studio Community 2019 対応に関して](#visual-studio-community-2019-対応に関して)
        - [vsconfig に関して](#vsconfig-に関して)
            - [Visaul Studio 2017/2019 共通](#visaul-studio-20172019-共通)
            - [Visaul Studio 2019 のみ](#visaul-studio-2019-のみ)
            - [参照](#参照)
    - [How to build](#how-to-build)
        - [詳細情報](#詳細情報)
    - [PR(Pull Request) を簡単にローカルに取得する方法](#prpull-request-を簡単にローカルに取得する方法)
    - [CI Buildおよびローカルビルドの環境変数](#CI-Buildおよびローカルビルドの環境変数)
    - [CI Build (Azure Pipelines)](#ci-build-azure-pipelines)
        - [ビルドの仕組み (Azure Pipelines)](#ビルドの仕組み-azure-pipelines)
    - [CI Build (AppVeyor)](#ci-build-appveyor)
        - [ビルドの仕組み (AppVeyor)](#ビルドの仕組み-appveyor)
        - [ビルド成果物を利用する上での注意事項](#ビルド成果物を利用する上での注意事項)
        - [ビルド成果物のダウンロード(バイナリ、インストーラなど)](#ビルド成果物のダウンロードバイナリインストーラなど)
            - [master の 最新](#master-の-最新)
            - [master の 最新以外](#master-の-最新以外)
    - [開発情報](#開発情報)
        - [単体テスト](#単体テスト)
        - [デバッグ方法](#デバッグ方法)
    - [変更履歴](#変更履歴)
    - [マクロのサンプル](#マクロのサンプル)
    - [静的コード解析](#静的コード解析)

<!-- /TOC -->

A free Japanese text editor for Windows

## Hot topic
Project(カンバン)運用を始めます。

- [Projects](https://github.com/orgs/sakura-editor/projects)
- [カンバン運用](https://github.com/sakura-editor/sakura/wiki/ProjectOperation)

## Web Site
- [Sakura Editor Portal](https://sakura-editor.github.io/)

## 開発参加ポリシー
開発ポリシーを以下にまとめていきます。開発にご参加いただける方はこちらご参照ください。  
https://github.com/sakura-editor/sakura/wiki

## Build Requirements

Sakura Editor をコンパイルするためにはいずれかが必要
-  [Visual Studio Community 2017][Visual Studio Community 2017]
-  [Visual Studio Community 2019][Visual Studio Community 2019]

正式バイナリは [Visual Studio Community 2017][Visual Studio Community 2017] でビルドされます。

### Visual Studio Install options required
- Windows SDK
- Windows XP Support for C++
- Windows 8.1 SDK と UCRT SDK
- C++ に関する Windows XP サポート

More information: https://github.com/sakura-editor/sakura/issues/6

### Visual Studio Community 2019 対応に関して

Visual Studio 2017/2019 の両対応に関しては  [#866](https://github.com/sakura-editor/sakura/issues/866) で対処済みです。  
[仕組みに関してはこちらを参照](vcx-props/project-PlatformToolset.md)

### .vsconfig に関して

Sakura Editor のコンパイルに必要なコンポーネントを Visual Studio 2017/2019 にインストールするために [.vsconfig](.vsconfig) という設定ファイルを用意しています。

[#1162](https://github.com/sakura-editor/sakura/pull/1162) で [.vsconfig](.vsconfig) というファイルを sakura.sln と同じディレクトリに配置しています。

#### Visaul Studio 2017/2019 共通

`vs_community__XXXXX.exe` でインストールする際に、--config オプションをつけてインストールする。
あるいは構成変更することにより、必要なコンポーネントを自動的にインストールします。

```
vs_community__XXXXX.exe --config <.vsconfig のファイルパス>
```

#### Visaul Studio 2019 のみ

`sakura.sln` と同じディレクトリに [.vsconfig](.vsconfig) が存在するので、
Visaul Studio 2019 で `sakura.sln` を開くと必要なコンポーネントが足りない場合、インストールを促す表示が出るので、インストールをクリックすると自動的にインストールする。

#### 参照

- [How to extract currently installed Visual Studio component IDs?][How to extract currently installed Visual Studio component IDs?]
- [Configure Visual Studio across your organization with .vsconfig][Configure Visual Studio across your organization with .vsconfig]
- [インストール構成をインポートまたはエクスポートする][インストール構成をインポートまたはエクスポートする]
- [コマンド ライン パラメーターを使用して Visual Studio をインストールする][コマンド ライン パラメーターを使用して Visual Studio をインストールする]
- [不足しているコンポーネントを自動的にインストールする][不足しているコンポーネントを自動的にインストールする]

## How to build

- [7Zip](https://sevenzip.osdn.jp/) のインストールして 7z.exe へのパスを通します。
- Visual Studio Community 2017 で `sakura.sln` を開いてビルドします。

### 詳細情報

詳しくは [こちら](build.md) を参照

## CI Buildおよびローカルビルドの環境変数

[CI でのビルド](ci-build.md) を参照

## PR(Pull Request) を簡単にローカルに取得する方法

- [PR(Pull Request) を簡単にローカルに取得する方法](get-PR.md)

## CI Build (Azure Pipelines)

### ビルドの仕組み (Azure Pipelines)

[azure-pipelines.md](azure-pipelines.md) でビルドの仕組みを説明しています。

## CI Build (AppVeyor)

### ビルドの仕組み (AppVeyor)

[appveyor.md](appveyor.md) でビルドの仕組みを説明しています。

### ビルド成果物を利用する上での注意事項

[`これ`](installer/warning.txt) を読んでからご利用ください。

[`x64 版は alpha 版`](installer/warning-alpha.txt)です。  
対応中のため予期せぬ不具合がある可能性があります。 

### ビルド成果物のダウンロード(バイナリ、インストーラなど)

#### master の 最新

1. https://ci.appveyor.com/project/sakuraeditor/sakura/branch/master にアクセスする
2. 右端にある `Jobs` をクリックします。
3. 自分がダウンロードしたいビルド構成 (例: `Configuration: Release; Platform: Win32`) をクリックします。
4. 右端にある `ARTIFACTS` をクリックします。
5. 自分がダウンロードしたいものをクリックしてダウンロードします。
   - (ユーザー用) 末尾に `Exe` がついてるのが実行ファイルのセットです。
   - (ユーザー用) 末尾に `Installer` がついてるのがインストーラのセットです。
   - ~~(すべて欲しい人向け) `All` がついてるのがバイナリ、インストーラ、ビルドログ、アセンブラ出力のフルセットです。~~ ([#514](https://github.com/sakura-editor/sakura/issues/514) の軽減のため無効化中) 
   - (開発者用) 末尾に `Log` がついてるのがビルドログのセットです。
   - (開発者用) 末尾に `Asm` がついてるのがアセンブラ出力セットです。

#### master の 最新以外

以下から取得したいビルドを選択後、同様にしてダウンロードできます。  
https://ci.appveyor.com/project/sakuraeditor/sakura/history

## 開発情報

### 単体テスト

[単体テスト](unittest.md) を参照

### デバッグ方法

- [タスクトレイのメニュー項目をデバッグする方法](debug-tasktray-menu.md) を参照
- [大きなファイルの作成方法](create-big-file.md)

## 変更履歴

変更履歴は [github-changelog-generator](https://github.com/github-changelog-generator/github-changelog-generator) というソフトを使用して
[changelog-sakura](https://github.com/sakura-editor/changelog-sakura) のリポジトリで [appveyor](https://ci.appveyor.com/project/sakuraeditor/changelog-sakura) で自動的に生成します。 

[生成した CHANGELOG.md は ここからダウンロードできます](https://ci.appveyor.com/project/sakuraeditor/changelog-sakura/branch/master/artifacts) 

ダウンロードした `CHANGELOG.md` は
[Markdown をローカルで確認する方法](https://github.com/sakura-editor/sakura/wiki/markdown-%E3%82%92%E3%83%AD%E3%83%BC%E3%82%AB%E3%83%AB%E3%81%A7%E7%A2%BA%E8%AA%8D%E3%81%99%E3%82%8B%E6%96%B9%E6%B3%95)
で説明している手順でローカルで確認できます。 

[CHANGELOG.mdについて](https://github.com/sakura-editor/sakura/wiki/CHANGELOG.md%E3%81%AB%E3%81%A4%E3%81%84%E3%81%A6) のページに`CHANGELOG.md` に関する説明を記載しています。

## マクロのサンプル

[こちら](tools/macro)でマクロのサンプルを提供してます。  
もしサンプルを作ってもいいよ～という方がおられましたら PR の作成お願いします。

## 静的コード解析

- CodeFactor を利用しています。
    - [cpplint.py](CPPLINT.md) で cpp のスタイルのチェックを行っています。


