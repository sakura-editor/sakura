<!-- TOC -->

- [インストーラ作成に必要なもの](#インストーラ作成に必要なもの)
- [インストーラ作成手順](#インストーラ作成手順)
    - [すべてビルドする場合](#すべてビルドする場合)
    - [個別にビルドする場合](#個別にビルドする場合)
- [インストーラの設定ファイル](#インストーラの設定ファイル)
- [インストーラのビルドに必要なファイル](#インストーラのビルドに必要なファイル)
- [インストーラのビルド](#インストーラのビルド)
    - [Win32](#win32)
    - [x64](#x64)

<!-- /TOC -->

## インストーラ作成に必要なもの

- Visual Studio 2017 Comminity Edition (sakura editor の実行ファイルのビルドに必要です。)
- HTML Help Workshop (chm ファイルのコンパイルに必要です。Visual Studio 2017 のインストールでインストールされます。)
- [Inno Setup](http://www.jrsoftware.org/isdl.php) の以下のいずれかのバージョンです。
    - [innosetup-5.5.9-unicode.exe](http://files.jrsoftware.org/is/5/) (appveyor でのビルドではこちらが使われます。)
    - [innosetup-5.6.1-unicode.exe](http://www.jrsoftware.org/isdl.php)

## インストーラ作成手順

### すべてビルドする場合

以下のコマンドを実行する

```
build-all.bat <Platform> <Configuration>
```


| 引数 | 名前 | 値 |
----|----|----
|第一引数 | platform      | "Win32" または "x64" |
|第二引数 | configuration | "Debug" または "Relelase" |

### 個別にビルドする場合

以下のコマンドを実行する

```
build-sln.bat <Platform> <Configuration>
build-chm.bat
build-installer.bat <Platform> <Configuration>
```

## インストーラの設定ファイル

Inno Setup の設定ファイルは拡張子が iss のファイルです。

| iss ファイル | 意味 |
----|----
|[sakura-common.iss](sakura-common.iss) |共通ファイル。以下の２つのファイルからインクルードされます。 |
|[sakura-Win32.iss](sakura-Win32.iss)   |Win32 用の iss ファイル |
|[sakura-x64.iss](sakura-x64.iss)       |x64   用の iss ファイル |

## インストーラのビルドに必要なファイル

事前に以下にファイルを配置する。(build-installer.bat を実行すると以下のファイルの配置～インストーラのビルドまで行う。)

- installer\sakura\sakura.exe
- installer\sakura\sakura_lang_en_US.dll
- installer\sakura\bregonig.dll
- installer\sakura\bsd_license.txt
- installer\sakura\sakura.exe.manifest.x
- installer\sakura\sakura.exe.manifest.v
- installer\sakura\sakura.chm
- installer\sakura\macro.chm
- installer\sakura\plugin.chm
- installer\sakura\sakura.exe.ini
- installer\sakura\keyword\*.kwd

## インストーラのビルド

以下のコマンドでインストーラをビルドします。(build-installer.bat に含まれます。)

### Win32

"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" installer\sakura-Win32.iss

→ installer\Output-Win32\ にインストーラが生成されます。

### x64

"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" installer\sakura-x64.iss

→ installer\Output-x64\ にインストーラが生成されます。
