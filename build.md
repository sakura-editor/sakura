﻿# ビルド方法

<!-- TOC -->

- [ビルド方法](#ビルド方法)
  - [必要なもの](#必要なもの)
    - [実行ファイルのビルドに必要なもの](#実行ファイルのビルドに必要なもの)
    - [HTML ヘルプのビルドに必要なもの](#html-ヘルプのビルドに必要なもの)
    - [インストーラのビルドに必要なもの](#インストーラのビルドに必要なもの)
    - [(オプション) ログの解析等に必要なもの](#オプション-ログの解析等に必要なもの)
  - [ビルド方法](#ビルド方法-1)
    - [実行ファイルだけをビルドする場合](#実行ファイルだけをビルドする場合)
      - [方法1 (GUI)](#方法1-gui)
      - [方法2 (コマンドライン)](#方法2-コマンドライン)
        - [具体例 (x64 の Release)](#具体例-x64-の-release)
    - [すべてビルドする場合](#すべてビルドする場合)
        - [具体例 (Win32 の Release)](#具体例-win32-の-release)
      - [Visual Studio 2019 を使用してコマンドラインでビルド](#visual-studio-2019-を使用してコマンドラインでビルド)
  - [ビルドの仕組み](#ビルドの仕組み)
    - [appveyor でのビルドの仕組み](#appveyor-でのビルドの仕組み)
    - [Azure Pipelines でのビルドの仕組み](#azure-pipelines-でのビルドの仕組み)
    - [インストーラの仕組み](#インストーラの仕組み)
  - [開発者向けの情報](#開発者向けの情報)
    - [githash.h の更新のスキップ](#githashh-の更新のスキップ)
    - [Powershell によるZIPファイルの圧縮、解凍、内容確認の強制](#powershell-によるzipファイルの圧縮解凍内容確認の強制)
    - [MinGW w64 ビルド](#mingw-w64-ビルド)

<!-- /TOC -->

## 必要なもの

### 実行ファイルのビルドに必要なもの

- (オプション) [7Zip](https://sevenzip.osdn.jp/) (外部依存ファイルの解凍に使用)
- Visual Studio 2017 Comminity Edition
   以下オプションも必要です。
   - Windows XP Support for C++
   - Windows 8.1 SDK と UCRT SDK
   - C++ に関する Windows XP サポート

### HTML ヘルプのビルドに必要なもの

- HTML ヘルプコンパイラ (hhc.exe)
   - Visual Studio 2017 のインストールにて以下のオプションを有効にすることにより導入されます。
      - 「C++ によるデスクトップ開発」を有効にする
      - 右のペインで 「C++ によるデスクトップ開発」を選ぶ
      - 「x86用とx64用のVisual C++ MFC」をチェックする
      - 変更を確定する
   - VSインストール後でもVisual Studio Installerを起動して導入可能です。 
![vsi](https://user-images.githubusercontent.com/39618965/44622575-012dcc80-a8f6-11e8-906a-14d8cd6dfac9.PNG)
   - ヘルプファイルを編集する場合 HTML Help Workshop 等の編集ソフトも別途必要になります。

### インストーラのビルドに必要なもの

インストーラビルドの仕組みは [こちら](installer/readme.md) を参照

- [Inno Setup](http://www.jrsoftware.org/isdl.php) の以下のいずれかのバージョンです。
    - [innosetup-5.5.9-unicode.exe](http://files.jrsoftware.org/is/5/) (appveyor でのビルドではこちらが使われます。)
    - [innosetup-5.6.1-unicode.exe](http://www.jrsoftware.org/isdl.php)

### (オプション) ログの解析等に必要なもの

- [Python](https://www.python.org/) の 2.7 系、3.x 系のどちらでも OK (のはず)

## ビルド方法

### 実行ファイルだけをビルドする場合

#### 方法1 (GUI)

Visual Studio Community 2017 で `sakura.sln` をダブルクリックして開いてビルドします。

#### 方法2 (コマンドライン)

```
build-sln.bat <Platform> <Configuration>
```

##### 具体例 (x64 の Release)

```
build-sln.bat x64 Release
```


### すべてビルドする場合

```
build-all.bat <Platform> <Configuration>
```

##### 具体例 (Win32 の Release)

```
build-all.bat Win32 Release
```

#### Visual Studio 2019 を使用してコマンドラインでビルド

```
set ARG_VSVERSION=16
build-all.bat Win32 Release
```

参考

[こちら](tools/find-tools.md#MSBuild) で ```ARG_VSVERSION``` に関して説明しています。


## ビルドの仕組み

### appveyor でのビルドの仕組み

[こちら](appveyor.md) で appveyor 上でのビルドの仕組みを説明しています。

### Azure Pipelines でのビルドの仕組み

[こちら](azure-pipelines.md) で [Azure Pipelines](https://azure.microsoft.com/ja-jp/services/devops/pipelines/) 上でのビルドの仕組みを説明しています。

### インストーラの仕組み

[こちら](installer/readme.md) でビルドの仕組みを説明しています。

## 開発者向けの情報

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


### Powershell によるZIPファイルの圧縮、解凍、内容確認の強制

`7z.exe` へのパスが通っている場合または `C:\Program Files\7-Zip\7z.exe` が存在している場合は
`7z.exe` を、ZIP ファイルの解凍、圧縮、内容確認に使用します。

上記以外の場合は [powershell によるスクリプト](tools/zip/readme.md) により処理を行います。

`7z.exe` のほうがはるかに処理速度が速いので `7z.exe` が利用可能なら [powershell によるスクリプト](tools/zip/readme.md) を
使う理由は殆どないのですが、デバッグ目的で強制的に [powershell によるスクリプト](tools/zip/readme.md) を使用する手段を
提供します。

コマンドラインでビルドするときに事前に FORCE_POWERSHELL_ZIP を 1 に設定することにより
強制的に [powershell によるスクリプト](tools/zip/readme.md) を使用します。

コマンド実行例

```
set FORCE_POWERSHELL_ZIP=1
build-sln.bat Win32 Release
build-sln.bat Win32 Debug
build-sln.bat x64   Release
build-sln.bat x64   Debug
```

### MinGW w64 ビルド

生成されるバイナリは正しく動作しないが、MinGWでのビルドも可能。


MinGW64のビルド環境

* [pleiades 4.6 Neon](http://mergedoc.osdn.jp/)
* [MSYS2+MinGW-w64](https://gist.github.com/Hamayama/eb4b4824ada3ac71beee0c9bb5fa546d)
* [MinGW-w64](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/)

cpu | thread モデル | 例外モデル | コメント
---- | ---- | ---- | ----
x86_64 | win32 | seh | windows向け。外部DLL不要
x86_64 | posix | seh | 標準。pthreadのDLLが必要
x86_64 | win32 | sjlj | 外部DLL不要
x86_64 | posix | sjlj | pthreadのDLLが必要

標準的なMinGWセットアップでビルドしたバイナリは ```libwinpthread-1.dll``` に依存することに注意。


コマンド実行例

```
path=C:\msys64\mingw64\bin;%path%
mingw32-make -C sakura_core -j4
```
