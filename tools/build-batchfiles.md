# ビルドに使用するバッチファイル

<!-- TOC -->

- [ビルドに使用するバッチファイル](#ビルドに使用するバッチファイル)
  - [使用するバッチファイルの一覧](#使用するバッチファイルの一覧)
  - [呼び出し構造](#呼び出し構造)
  - [ビルドに使用するバッチファイルの引数](#ビルドに使用するバッチファイルの引数)
  - [バッチファイルの仕組み](#バッチファイルの仕組み)
    - [githash.bat の構造](#githashbat-の構造)
      - [処理の流れ](#処理の流れ)
    - [zipArtifacts.bat の構造](#zipartifactsbat-の構造)
      - [処理の流れ](#処理の流れ-1)

<!-- /TOC -->

## 使用するバッチファイルの一覧

| ファイル名 | 説明 |
|----|----|
|[tools\githash.bat](./githash.bat) | Git や CI の環境変数から githash.h を生成する |
|[tools\find-tools.bat](./find-tools.md) | ビルド関連ツールのパスを探す |
|[build-all.bat](../build-all.bat)| すべてをビルドできるバッチファイル  |
|[build-sln.bat](../build-sln.bat) | solution をビルドする |
|[build-gnu.bat](../build-gnu.bat) | Makefile をビルドする |
|[build-chm.bat](../build-chm.bat) | compiled HTML ファイルをビルドする |
|[build-installer.bat](../build-installer.bat) | インストーラをビルドする |
|[zipArtifacts.bat](../zipArtifacts.bat) | 成果物を zip ファイルにまとめる |

## 呼び出し構造

- [build-all.bat](../build-all.bat)
    - [build-sln.bat](../build-sln.bat)
        - MSBuild.exe sakura.sln
            - cmake.exe Gitリポジトリ情報を version.h に書き出す。
                - git.exe
            - HeaderMake.exe : Funccode_define.h, Funccode_enum.h を生成する
            - cmake.exe 外部ソースからツールをビルドする、または、配布zipから展開する
                - cmake.exe
                - 7z.exe
    - [build-gnu.bat](../build-gnu.bat)
        - cmake -S . -B build/MinGW -DCMAKE_BUILD_TYPE=Debug -DBUILD_PLATFORM=MinGW
        - cmake --build build/MinGW --config Debug --target sakura
        - cmake --build build/MinGW --config Debug --target sakura_lang_en_US
        - cmake --build build/MinGW --config Debug --target tests1
        - ctest --test-dir build/MinGW --build-config Debug --output-on-failure
    - [build-chm.bat](../build-chm.bat)
        - cmake.exe
          - ChmSourceConverter.exe : ヘルプファイルの文字コードを UTF-8 から Shift_JIS に変換する
          - pwsh.exe
            - [help\CompileChm.ps1](../help/CompileChm.ps1)
              - hhc.exe (Visual Studio に同梱) : compiled HTML をビルドするコンパイラ。かなり古いツールであり、日本語 HTML をビルドするためには Windows のシステムロケールを日本語に変更する必要がある。
    - [build-installer.bat](../build-installer.bat)
        - ISCC.exe : [InnoSetup](https://www.jrsoftware.org/isinfo.php) でインストーラをビルドする
    - [zipArtifacts.bat](../zipArtifacts.bat)
        - [tools\githash.bat](./githash.bat)
            - git.exe
        - [tools\zip\zip.bat](./zip/zip.bat) : 成果物を ZIP ファイルにまとめる
            - 7z.exe または [tools\zip\zip.ps1](./zip/zip.ps1)

## ビルドに使用するバッチファイルの引数

| バッチファイル | 第一引数 | 第二引数 |
|----|----|----|
|build-all.bat       | platform ("Win32" または "x64" または "MinGW") | configuration ("Debug" または "Release")  |
|build-sln.bat       | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|build-gnu.bat       | platform ("MinGW") | configuration ("Debug" または "Release")  |
|build-chm.bat       | なし | なし |
|build-installer.bat | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|zipArtifacts.bat    | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |

## バッチファイルの仕組み

### githash.bat の構造

#### 処理の流れ

- Git や CI の環境変数を元に githash.h を生成する
    - 設定される環境変数については [こちら](build-envvars.md) を参照してください。

### zipArtifacts.bat の構造

#### 処理の流れ

* if 文の条件判定を元に、成果物のファイル名、フォルダー名を構築して環境変数に設定する
    - 設定される環境変数については [こちら](build-envvars.md#zipartifactsbat-で設定する環境変数) を参照してください。
* 作業用フォルダーに必要なファイルをコピーする
* [tools\zip\zip.bat](./zip/zip.bat) を使用して作業用フォルダーの中身を zip ファイルにまとめる
    - [7-Zip](https://7-zip.opensource.jp/) が利用できる場合は 7z.exe を、利用できない場合は PowerShell を利用してファイルを作成します。
