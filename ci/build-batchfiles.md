# ビルドに使用するバッチファイル

<!-- TOC -->

- [ビルドに使用するバッチファイル](#ビルドに使用するバッチファイル)
  - [使用するバッチファイルの一覧](#使用するバッチファイルの一覧)
    - [関連情報](#関連情報)
  - [呼び出し構造](#呼び出し構造)
  - [ビルドに使用するバッチファイルの引数](#ビルドに使用するバッチファイルの引数)
  - [バッチファイルの仕組み](#バッチファイルの仕組み)
    - [githash.bat の構造](#githashbat-の構造)
      - [処理の流れ](#処理の流れ)
    - [postBuild.bat の構造](#postbuildbat-の構造)
      - [処理の流れ](#処理の流れ-1)
    - [zipArtifacts.bat の構造](#zipartifactsbat-の構造)
      - [処理の流れ](#処理の流れ-2)

<!-- /TOC -->

## 使用するバッチファイルの一覧

| ファイル名 | 説明 |
----|---- 
|[build-all.bat](../build-all.bat)| appveyor.yml から呼ばれるバッチファイル  |
|[build-sln.bat](../build-sln.bat) | solution をビルドする |
|[build-gnu.bat](../build-gnu.bat) | Makefile をビルドする |
|[build-sonar-qube-start.bat](../build-sonar-qube-start.bat) | SonarQube の準備を行う |
|[build-sonar-qube-env.bat](../build-sonar-qube-env.bat) | SonarQube の実行に必要な環境変数の設定を行う |
|[sakura\preBuild.bat](../sakura/preBuild.bat) | 特に何もしない |
|[sakura\githash.bat](../sakura/githash.bat) | Git や CI の環境変数から githash.h を生成する |
|[sakura\postBuild.bat](../sakura/postBuild.bat) | bregonig.dll と ctags.exe を展開しコピーする |
|[tests\googletest.build.cmd](../tests/googletest.build.cmd) | Google Test をビルドする |
|[tests\compiletests.run.cmd](../tests/compiletests.run.cmd) | コンパイルテストを実行する |
|[build-sonar-qube-finish.bat](../build-sonar-qube-finish.bat) | SonarQube の解析結果をアップロードする |
|[parse-buildlog.bat](../parse-buildlog.bat) | ビルドログを解析する |
|[build-chm.bat](../build-chm.bat) | compiled HTML ファイルをビルドする |
|[build-installer.bat](../build-installer.bat) | インストーラをビルドする |
|[zipArtifacts.bat](../zipArtifacts.bat) | 成果物を zip ファイルにまとめる |
|[calc-hash.bat](../calc-hash.bat) | 成果物のハッシュ値を計算する |

### 関連情報

SonarQube に関しては [こちら](../SonarQube.md) も参照してください。

## 呼び出し構造

- [build-all.bat](../build-all.bat)
    - [build-sln.bat](../build-sln.bat)
        - [build-sonar-qube-start.bat](../build-sonar-qube-start.bat)
            - [build-sonar-qube-env.bat](../build-sonar-qube-env.bat)
            - [tools\zip\unzip.bat](../tools/zip/unzip.bat) : ZIP ファイルを展開する
                - 7z.exe または [tools\zip\unzip.ps1](../tools/zip/unzip.ps1)
        - MSBuild.exe sakura.sln
            - [sakura\preBuild.bat](../sakura/preBuild.bat)
            - HeaderMake.exe : Funccode_define.h, Funccode_enum.h を生成する
            - [sakura\githash.bat](../sakura/githash.bat)
                - git.exe
            - [sakura\postBuild.bat](../sakura/postBuild.bat)
                - [tools\zip\unzip.bat](../tools/zip/unzip.bat)
                    - 7z.exe または [tools\zip\unzip.ps1](../tools/zip/unzip.ps1)
            - [tests\googletest.build.cmd](../tests/googletest.build.cmd)
                - git.exe
                - cmake.exe
            - [tests\compiletests.run.cmd](../tests/compiletests.run.cmd)
                - cmake.exe
        - [build-sonar-qube-finish.bat](../build-sonar-qube-finish.bat)
            - [build-sonar-qube-env.bat](../build-sonar-qube-env.bat)
        - [parse-buildlog.bat](../parse-buildlog.bat)
            - [appveyor_env.py](../appveyor_env.py) : AppVeyor の環境変数を再現できる `set_appveyor_env.bat` を生成する。(成果物に含まれる)
            - [parse-buildlog.py](../parse-buildlog.py)
                - [appveyor_env.py](../appveyor_env.py)
    - [build-gnu.bat](../build-gnu.bat)
        - mingw32-make.exe sakura_core
            - [sakura\githash.bat](../sakura/githash.bat)
                - git.exe
            - HeaderMake.exe
    - [build-chm.bat](../build-chm.bat)
        - [help\remove-comment.py](../help/remove-comment.py) : [sakura_core\sakura.hh](../sakura_core/sakura.hh) に記述された日本語を含む行コメントを削除する
        - ChmSourceConverter.exe : ヘルプファイルの文字コードを UTF-8 から Shift_JIS に変換する
        - [help\CompileChm.ps1](../help/CompileChm.ps1)
            - hhc.exe (Visual Studio に同梱) : compiled HTML をビルドするコンパイラ。かなり古いツールであり、日本語 HTML をビルドするためには Windows のシステムロケールを日本語に変更する必要がある。
    - [build-installer.bat](../build-installer.bat)
        - ISCC.exe : [InnoSetup](https://www.jrsoftware.org/isinfo.php) でインストーラをビルドする
    - [zipArtifacts.bat](../zipArtifacts.bat)
        - [sakura\githash.bat](../sakura/githash.bat)
            - git.exe
        - [calc-hash.bat](../calc-hash.bat)
            - [calc-hash.py](../calc-hash.py)
        - [tools\zip\zip.bat](../tools/zip/zip.bat) : 成果物を ZIP ファイルにまとめる
            - 7z.exe または [tools\zip\zip.ps1](../tools/zip/zip.ps1)

## ビルドに使用するバッチファイルの引数

| バッチファイル | 第一引数 | 第二引数 |
----|----|----
|build-all.bat       | platform ("Win32" または "x64" または "MinGW") | configuration ("Debug" または "Release")  |
|build-sln.bat       | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|build-gnu.bat       | platform ("MinGW") | configuration ("Debug" または "Release")  |
|sakura\preBuild.bat | HeaderMake.exe の実行ファイルのフォルダーパス | なし |
|sakura\postBuild.bat| platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|parse-buildlog.bat  | msbuild のビルドログパス | なし |
|build-chm.bat       | なし | なし |
|build-installer.bat | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|zipArtifacts.bat    | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|calc-hash.bat       | sha256 のハッシュ値の出力先ファイル | ハッシュ値を計算するフォルダーパス |

## バッチファイルの仕組み

### githash.bat の構造

#### 処理の流れ

- Git や CI の環境変数を元に githash.h を生成する
    - 設定される環境変数については [こちら](build-envvars.md) を参照してください。

### postBuild.bat の構造

#### 処理の流れ

* リポジトリに登録している bregonig と ctags の zip ファイルを解凍して bregonig.dll と ctags.exe を sakura.exe のビルド出力先にコピーする

### zipArtifacts.bat の構造

#### 処理の流れ

* if 文の条件判定を元に、成果物のファイル名、フォルダー名を構築して環境変数に設定する
    - 設定される環境変数については [こちら](build-envvars.md#zipartifactsbat-で設定する環境変数) を参照してください。
* 作業用フォルダーに必要なファイルをコピーする
* [calc-hash.bat](../calc-hash.bat) で sha256 のハッシュを計算して、作業用フォルダーにコピーする
* [tools\zip\zip.bat](../tools/zip/zip.bat) を使用して作業用フォルダーの中身を zip ファイルにまとめる
    - [7-Zip](https://sevenzip.osdn.jp/) が利用できる場合は 7z.exe を、利用できない場合は PowerShell を利用してファイルを作成します。
