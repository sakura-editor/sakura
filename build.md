# ビルド方法

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
    - [ビルドの仕組み](#ビルドの仕組み)
        - [appveyor でのビルドの仕組み](#appveyor-でのビルドの仕組み)
        - [インストーラの仕組み](#インストーラの仕組み)
    - [開発者向けの情報](#開発者向けの情報)
        - [githash.h の更新のスキップ](#githashh-の更新のスキップ)

<!-- /TOC -->

## 必要なもの

### 実行ファイルのビルドに必要なもの

- [7Zip](https://sevenzip.osdn.jp/) (外部依存ファイルの解凍に使用) (7z.exe へのパスを通しておく)
- Visual Studio 2017 Comminity Edition

### HTML ヘルプのビルドに必要なもの

- HTML Help Workshop (Visual Studio 2017 のインストールでインストールされます。)

### インストーラのビルドに必要なもの

[こちら](installer/readme.md) を参照

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

## ビルドの仕組み

### appveyor でのビルドの仕組み

[こちら](appveyor.md) で appveyor 上でのビルドの仕組みを説明しています。

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
