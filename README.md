# Sakura Editor

A free text editor for Windows.
[![License: Zlib](https://img.shields.io/badge/License-Zlib-lightgrey.svg)](https://opensource.org/licenses/Zlib)
[![build sakura](https://github.com/sakura-editor/sakura/workflows/build%20sakura/badge.svg)](https://github.com/sakura-editor/sakura/actions?query=workflow%3A%22build+sakura%22)
[![CodeFactor](https://www.codefactor.io/repository/github/sakura-editor/sakura/badge)](https://www.codefactor.io/repository/github/sakura-editor/sakura)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=sakura-editor_sakura&metric=alert_status)](https://sonarcloud.io/dashboard?id=sakura-editor_sakura)
[![Github Releases All](https://img.shields.io/github/downloads/sakura-editor/sakura/total.svg)](https://github.com/sakura-editor/sakura/releases "All Releases")
[![Star History](https://img.shields.io/badge/star-histroy-yellow.svg)](https://star-history.t9t.io/#sakura-editor/sakura)

<!-- 以下は Markdownの参照形式によるリンク の定義です。 -->
<!-- 参照 https://hail2u.net/blog/coding/markdown-reference-style-links.html -->

[Visual Studio 以前のバージョン]: https://visualstudio.microsoft.com/ja/vs/older-downloads/ "Visual Studio 以前のバージョン"
[Visual Studio 最新版]: https://visualstudio.microsoft.com/ja/downloads/ "Visual Studio 最新版"
[ライセンスの OSI のページ]: https://opensource.org/license/zlib
[Visual Studio Community ライセンス]: https://visualstudio.microsoft.com/ja/license-terms/vs2022-ga-community/
[Markdown をローカルで確認する方法]: https://github.com/sakura-editor/sakura/wiki/markdown-%E3%82%92%E3%83%AD%E3%83%BC%E3%82%AB%E3%83%AB%E3%81%A7%E7%A2%BA%E8%AA%8D%E3%81%99%E3%82%8B%E6%96%B9%E6%B3%95
[How to extract currently installed Visual Studio component IDs?]: https://stackoverflow.com/questions/52946333/how-to-extract-currently-installed-visual-studio-component-ids
[Configure Visual Studio across your organization with .vsconfig]: https://devblogs.microsoft.com/setup/configure-visual-studio-across-your-organization-with-vsconfig/
[インストール構成をインポートまたはエクスポートする]: https://docs.microsoft.com/ja-jp/visualstudio/install/import-export-installation-configurations?view=vs-2019
[コマンド ライン パラメーターを使用して Visual Studio をインストールする]: https://docs.microsoft.com/ja-jp/visualstudio/install/use-command-line-parameters-to-install-visual-studio?view=vs-2019
[不足しているコンポーネントを自動的にインストールする]: https://docs.microsoft.com/ja-jp/visualstudio/install/import-export-installation-configurations?view=vs-2019#automatically-install-missing-components

<!-- TOC -->

- [Sakura Editor](#sakura-editor)
  - [Web Site](#web-site)
  - [ダウンロード](#ダウンロード)
    - [ビルド成果物を利用する上での注意事項](#ビルド成果物を利用する上での注意事項)
    - [開発中 の 最新](#開発中-の-最新)
  - [開発情報](#開発情報)
    - [Build Requirements](#build-requirements)
    - [How to build](#how-to-build)
    - [Details of build](#details-of-build)
      - [env for build](#env-for-build)
      - [batch for build](#batch-for-build)
      - [Visual Studio Install options required](#visual-studio-install-options-required)
        - [Reference of .vsconfig](#reference-of-vsconfig)
    - [How to debug](#how-to-debug)
    - [Static code analysis](#static-code-analysis)
    - [マクロのサンプル](#マクロのサンプル)
    - [開発参加ポリシー](#開発参加ポリシー)
  - [変更履歴](#変更履歴)

<!-- /TOC -->

## Web Site

- [Sakura Editor Portal](https://sakura-editor.github.io/)

## ダウンロード

リリース版は上記のポータルから取得してください。

### ビルド成果物を利用する上での注意事項

[`x64 版は alpha 版`](installer/warning-alpha.txt)です。  
対応中のため予期せぬ不具合がある可能性があります。 

### 開発中 の 最新

1. [GitHub Actionsのビルドページ](https://github.com/sakura-editor/sakura/actions/workflows/build-sakura.yml?query=branch%3Amaster) にアクセスします。
2. 右端にある `Jobs` をクリックします。
3. `ARTIFACTS` をクリックします。
4. 末尾 `Exe` が実行ファイルです。

## 開発情報

### Build Requirements

Sakura Editor のビルドには、C++17以降をサポートするWindows向けC++コンパイラーが必要です。

- [最新のVisual Studio][Visual Studio 最新版]
- [以前のバージョンのVisual Studio][Visual Studio 以前のバージョン]
- MinGW64 GCC(msys2経由でインストールした最新場のみ)

### How to build

- `sakura.sln` を Visual Studio で 開いてビルドします。

### Details of build

詳しくは [こちら](./tools/build.md) を参照

#### env for build

[ビルドで使用する環境変数](./tools/build-envvars.md) を参照してください。

#### batch for build

[ビルドに使用されるバッチファイル](./tools/build-batchfiles.md) を参照してください。

#### Visual Studio Install options required

Sakura Editor のコンパイルに必要なコンポーネントは、[.vsconfig](.vsconfig) ファイルで定義されています。

Visual Studio 2019 以降で `sakura.sln` を開くと、[.vsconfig](.vsconfig) から必要なコンポーネントが自動的に読み込まれます。不足しているコンポーネントがある場合は、インストールボタンが表示されるので、クリックすれば自動的にインストールされます。

> **Note**: [.vsconfig](.vsconfig) は [#1162](https://github.com/sakura-editor/sakura/pull/1162) で追加されました。

##### Reference of .vsconfig

- [How to extract currently installed Visual Studio component IDs?][How to extract currently installed Visual Studio component IDs?]
- [Configure Visual Studio across your organization with .vsconfig][Configure Visual Studio across your organization with .vsconfig]
- [インストール構成をインポートまたはエクスポートする][インストール構成をインポートまたはエクスポートする]
- [コマンド ライン パラメーターを使用して Visual Studio をインストールする][コマンド ライン パラメーターを使用して Visual Studio をインストールする]
- [不足しているコンポーネントを自動的にインストールする][不足しているコンポーネントを自動的にインストールする]

### How to debug

- [タスクトレイのメニュー項目をデバッグする方法](tools/debug-tasktray-menu.md) を参照
- [大きなファイルの作成方法](tools/create-big-file.md)

### Static code analysis

- CodeFactor を利用しています。
    - [cpplint.py](CPPLINT.md) で cpp のスタイルのチェックを行っています。
    - [CodeFactorのサイト](https://www.codefactor.io/repository/github/sakura-editor/sakura)で解析結果を確認できます。
- SonarQube を利用しています。
    - [SonarQube Cloudのサイト](https://sonarcloud.io/summary/overall?id=sakura-editor_sakura&branch=master)で解析結果を確認できます。

### マクロのサンプル

[こちら](tools/macro)でマクロのサンプルを提供してます。  
もしサンプルを作ってもいいよ～という方がおられましたら PR の作成お願いします。

### 開発参加ポリシー

開発ポリシーを以下にまとめていきます。開発にご参加いただける方はこちらご参照ください。  

- [Wiki](https://github.com/sakura-editor/sakura/wiki)

## 変更履歴

- 変更履歴は [CHANGELOG.md](https://github.com/sakura-editor/sakura/blob/master/CHANGELOG.md) を参照してください。
- 最新の `CHANGELOG.md` は [ここからダウンロード](https://ci.appveyor.com/project/sakuraeditor/changelog-sakura/branch/master/artifacts) できます([Markdown をローカルで確認する方法](https://github.com/sakura-editor/sakura/wiki/markdown-%E3%82%92%E3%83%AD%E3%83%BC%E3%82%AB%E3%83%AB%E3%81%A7%E7%A2%BA%E8%AA%8D%E3%81%99%E3%82%8B%E6%96%B9%E6%B3%95)
で説明している手順でローカルで確認できます)。
- `CHANGELOG.md` は [PullRequest](https://github.com/sakura-editor/sakura/pulls) から自動的に生成しています。
  - 具体的には [github-changelog-generator](https://github.com/github-changelog-generator/github-changelog-generator) というソフトを使用して [changelog-sakura](https://github.com/sakura-editor/changelog-sakura) のリポジトリで [appveyor](https://ci.appveyor.com/project/sakuraeditor/changelog-sakura) で生成しています。
  - 詳細は wiki の [CHANGELOG.mdについて](https://github.com/sakura-editor/sakura/wiki/CHANGELOG.md%E3%81%AB%E3%81%A4%E3%81%84%E3%81%A6) をご覧ください。
