# .vsconfig について

<!-- TOC -->

- [.vsconfig について](#vsconfig-について)
  - [概要](#概要)
  - [自動インストール](#自動インストール)
  - [コマンドラインからのインストール](#コマンドラインからのインストール)
  - [参考資料](#参考資料)

<!-- /TOC -->

## 概要

`.vsconfig` は、Visual Studio でビルドに必要なコンポーネントを定義する設定ファイルです。

Sakura Editor では、コンパイルに必要な Visual Studio のコンポーネントを [.vsconfig](.vsconfig) ファイルで管理しています。このファイルは [#1162](https://github.com/sakura-editor/sakura/pull/1162) で追加されました。

## 自動インストール

Visual Studio 2019 以降で `sakura.sln` を開くと、`.vsconfig` から必要なコンポーネントが自動的に読み込まれます。

不足しているコンポーネントがある場合は、インストールボタンが表示されるので、クリックすれば自動的にインストールされます。

## コマンドラインからのインストール

Visual Studio インストーラーを `--config` オプション付きで実行することで、必要なコンポーネントを自動的にインストールできます。

```pwsh
vs_community__XXXXX.exe --config <.vsconfig のファイルパス>
```

## 参考資料

- [How to extract currently installed Visual Studio component IDs?][How to extract currently installed Visual Studio component IDs?]
- [Configure Visual Studio across your organization with .vsconfig][Configure Visual Studio across your organization with .vsconfig]
- [インストール構成をインポートまたはエクスポートする][インストール構成をインポートまたはエクスポートする]
- [コマンド ライン パラメーターを使用して Visual Studio をインストールする][コマンド ライン パラメーターを使用して Visual Studio をインストールする]
- [不足しているコンポーネントを自動的にインストールする][不足しているコンポーネントを自動的にインストールする]

<!-- リンク定義 -->
[How to extract currently installed Visual Studio component IDs?]: https://stackoverflow.com/questions/52946333/how-to-extract-currently-installed-visual-studio-component-ids
[Configure Visual Studio across your organization with .vsconfig]: https://devblogs.microsoft.com/setup/configure-visual-studio-across-your-organization-with-vsconfig/
[インストール構成をインポートまたはエクスポートする]: https://docs.microsoft.com/ja-jp/visualstudio/install/import-export-installation-configurations?view=vs-2019
[コマンド ライン パラメーターを使用して Visual Studio をインストールする]: https://docs.microsoft.com/ja-jp/visualstudio/install/use-command-line-parameters-to-install-visual-studio?view=vs-2019
[不足しているコンポーネントを自動的にインストールする]: https://docs.microsoft.com/ja-jp/visualstudio/install/import-export-installation-configurations?view=vs-2019#automatically-install-missing-components
