<!-- TOC -->

- [インストーラ作成に必要なもの](#インストーラ作成に必要なもの)
- [インストーラ作成手順](#インストーラ作成手順)
    - [すべて一括でビルドする場合](#すべて一括でビルドする場合)
        - [具体例 (全構成をビルドする場合)](#具体例-全構成をビルドする場合)
    - [個別にビルドする場合](#個別にビルドする場合)
        - [具体例 (x64 の Release)](#具体例-x64-の-release)
- [インストーラの設定ファイル](#インストーラの設定ファイル)
- [インストーラのビルドに必要なファイル](#インストーラのビルドに必要なファイル)
- [インストーラのビルド](#インストーラのビルド)
    - [Win32](#win32)
    - [x64](#x64)
- [インストーラのテスト](#インストーラのテスト)
    - [インストーラーのデバッグ](#インストーラーのデバッグ)
    - [英語版インストーラーの動作確認について](#英語版インストーラーの動作確認について)

<!-- /TOC -->

## インストーラ作成に必要なもの

[こちら](../build.md#必要なもの) を参照

## インストーラ作成手順

### すべて一括でビルドする場合

以下のコマンドを実行する

```
build-all.bat <Platform> <Configuration>
```


| 引数 | 名前 | 値 |
----|----|----
|第一引数 | platform      | "Win32" または "x64" |
|第二引数 | configuration | "Debug" または "Release" |

#### 具体例 (全構成をビルドする場合)

```
build-all.bat Win32 Release
build-all.bat Win32 Debug
build-all.bat x64   Release
build-all.bat x64   Debug
```


### 個別にビルドする場合

以下のコマンドを実行する

```
build-sln.bat <Platform> <Configuration>
build-chm.bat
build-installer.bat <Platform> <Configuration>
```


#### 具体例 (x64 の Release)

```
build-sln.bat x64 Release
build-chm.bat
build-installer.bat x64 Release
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

- installer/
    - sakura/
        - sakura.exe
        - sakura_lang_en_US.dll
        - bregonig.dll
        - bsd_license.txt
        - sakura.exe.manifest.x
        - sakura.exe.manifest.v
        - sakura.chm
        - macro.chm
        - plugin.chm
        - sakura.exe.ini
        - keyword/
            - *.col
            - *.dic
            - *.hkn
            - *.khp
            - *.kwd
            - *.otl
            - *.rkw
            - *.rl
            - *.rule
            - *.txt

## インストーラのビルド

以下のコマンドでインストーラをビルドします。(build-installer.bat に含まれます。)

### Win32

"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" installer\sakura-Win32.iss

→ installer\Output-Win32\ にインストーラが生成されます。

### x64

"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" installer\sakura-x64.iss

→ installer\Output-x64\ にインストーラが生成されます。

## インストーラのテスト

### インストーラーのデバッグ

インストーラーのデバッグについては、Inno Script Studioを利用してステップ実行できます。ただしデバッカーから実行した場合のインストーラーの挙動が不安定の為実際の動作確認については、生成されたインストーラーそのもののexeを実行し動作確認したほうが確実です。
ですのでInno Script Studioを使うのは主にPascalのスクリプトの挙動を確認する場合に用います。

Inno Script Studioは、Inno Setupのサイトより、innosetup-qsp-5.6.1-unicode.exeのインストーラーにてインストールするか、Inno Script Studioのサイトより直接ダウンロードしてインストールしてください

https://www.kymoto.org/products/inno-script-studio/downloads

(有志で[日本語化のファイル](https://www42.atwiki.jp/jfactory/pages/75.html)も公開されています)

ただし現在、issファイルをインクルードしているとうまくブレイクポイントが有効にならないようなので、サクラエディタのissファイルのうち、sakura-x64.issまたは、sakura-Win32.issの作成したいインストーラのissファイル上２行をsakura-common.issの最初に挿入してからsakura-common.issにブレイクポイントを設定して実行してください。

### 英語版インストーラーの動作確認について

英語版のインストーラーの挙動を確認する場合には、実行環境を英語モードにする必要があります。
お使いのPCの言語設定を英語に変更するか([こちら](https://www.google.co.jp/search?q=%E8%A8%80%E8%AA%9E+%E6%97%A5%E6%9C%AC%E8%AA%9E+%E8%8B%B1%E8%AA%9E+Windows&oq=%E8%A8%80%E8%AA%9E%E3%80%80%E6%97%A5%E6%9C%AC%E8%AA%9E%E3%80%80%E8%8B%B1%E8%AA%9E%E3%80%80Windows&aqs=chrome..69i57j0l2.5435j0j4&sourceid=chrome&ie=UTF-8)参考)、お使いのPCのリソースに余裕があれば、[VirtualBOX](https://www.virtualbox.org/)等の仮想化ソフトウエアにて、[開発用Windowsマシン](https://developer.microsoft.com/en-us/microsoft-edge/tools/vms/)を利用する方法もあります。
（覚書：この仮想PCのユーザのパスワードは「Passw0rd!」です)