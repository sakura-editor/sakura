# appveyor でのビルド

<!-- TOC -->

- [appveyor でのビルド](#appveyor-でのビルド)
    - [CI でのビルドをスキップする方法](#ci-でのビルドをスキップする方法)
        - [参考サイト](#参考サイト)
    - [入力として使用する環境変数](#入力として使用する環境変数)
    - [ビルドに使用するバッチファイル](#ビルドに使用するバッチファイル)
    - [呼び出し構造](#呼び出し構造)
    - [ビルドに使用するバッチファイルの引数](#ビルドに使用するバッチファイルの引数)
    - [バッチファイルの仕組み](#バッチファイルの仕組み)
        - [preBuild.bat の構造](#prebuildbat-の構造)
            - [生成する環境変数](#生成する環境変数)
            - [処理の流れ](#処理の流れ)
        - [postBuild.bat の構造](#postbuildbat-の構造)
            - [処理の流れ](#処理の流れ-1)
        - [zipArtifacts.bat の構造](#zipartifactsbat-の構造)
            - [生成する環境変数](#生成する環境変数-1)
            - [処理の流れ](#処理の流れ-2)

<!-- /TOC -->

## CI でのビルドをスキップする方法

ビルドに関係ない修正 (ドキュメントの修正など)を行った場合に、コミットメッセージの中に
`[ci skip]` を含めることで、Appveyor での CI ビルドを行わないようにすることができます。
ただし PR をマージするときには、CI のビルドは走ります。

### 参考サイト

- https://srz-zumix.blogspot.com/2018/03/ci-ci-20183.html
- https://www.appveyor.com/docs/how-to/filtering-commits/#skip-commits


## 入力として使用する環境変数

| 環境変数 | 説明 |
----|---- 
|APPVEYOR_ACCOUNT_NAME              | appveyor のアカウント名 (sakura editor の場合 "sakuraeditor") |
|APPVEYOR_BUILD_NUMBER              | ビルド番号 |
|APPVEYOR_REPO_COMMIT               | commit Hash |
|APPVEYOR_REPO_TAG_NAME             | tag 名 |
|APPVEYOR_URL                       | https://ci.appveyor.com |
|APPVEYOR_BUILD_VERSION             | appveyor.yml の version フィールドの値 |
|APPVEYOR_PROJECT_SLUG              | project slug (appveyor の URL 名) |
|APPVEYOR_PULL_REQUEST_NUMBER       | Pull Request 番号 |
|APPVEYOR_PULL_REQUEST_HEAD_COMMIT  | Pull Request の Head commit Hash |
|APPVEYOR_REPO_NAME                 | リポジトリ名 (owner-name/repo-name) |
|APPVEYOR_REPO_PROVIDER             | appveyor の参照するリポジトリ種別 (GitHub の場合 "gitHub") |

* 上記環境変数をローカル環境で set コマンドで設定することにより appveyor でビルドしなくてもローカルでテストできます。
* 上記の環境変数がどんな値になるのかは、過去の appveyor ビルドでのログを見ることによって確認できます。

## ビルドに使用するバッチファイル

| バッチファイル | 説明 |
----|---- 
|[build-all.bat](build-all.bat)| appveyor.yml から呼ばれるバッチファイル  |
|[build-sln.bat](build-sln.bat)| solution をビルドするバッチファイル |
|[sakura\preBuild.bat](sakura/preBuild.bat) | solution のビルド前に Visual Studio から呼ばれるバッチファイル |
|[sakura\postBuild.bat](sakura/postBuild.bat)| 特に何もしない |
|[parse-buildlog.bat](parse-buildlog.bat)    | ビルドログを解析するバッチファイル |
|[build-chm.bat](build-chm.bat)       | compiled HTML ファイルをビルドするバッチファイル |
|[build-installer.bat](build-installer.bat) | インストーラをビルドするバッチファイル |
|[externals\cppcheck\install-cppcheck.bat](externals/cppcheck/install-cppcheck.bat) | cppcheck をインストールするバッチファイル |
|[run-cppcheck.bat](run-cppcheck.bat)       | cppcheck を実行するバッチファイル |
|[zipArtifacts.bat](zipArtifacts.bat)       | 成果物を zip に固めるバッチファイル |
|[calc-hash.bat](calc-hash.bat)             | 成果物の sha256 を計算するバッチファイル |

## 呼び出し構造

- [build-all.bat](build-all.bat)
    - [build-sln.bat](build-sln.bat) : ソリューションファイルのビルド
        - msbuild sakura.sln
            - [sakura\preBuild.bat](sakura/preBuild.bat) : ビルド前の準備。
                - HeaderMake.exe : Funccode_define.h, Funccode_enum.h を生成する
                - MakefileMake.exe : Makefile を更新する (MinGW 用)
                - git.exe : git や appveyor 関連の情報を githash.h に出力する
            - [sakura\postBuild.bat](sakura/postBuild.bat) : bregonig.dll のコピー
                - [unzip.bat](tools/zip/unzip.bat) : 外部依存ファイルを展開する
                    - [7z.exe](https://sevenzip.osdn.jp/) : zip ファイルの展開に使用
                    - [unzip.ps1](tools/zip/unzip.ps1) : powershell 版の ZIP ファイルの展開スクリプト
        - [parse-buildlog.bat](parse-buildlog.bat) : ビルドログを解析する
            - [appveyor_env.py](appveyor_env.py)
            - [parse-buildlog.py](parse-buildlog.py)
                - [appveyor_env.py](appveyor_env.py)
    - [build-chm.bat](build-chm.bat) : HTML Help をビルドする
        - hhc.exe (Visual Studio 2017 に同梱)
    - [externals\cppcheck\install-cppcheck.bat](externals/cppcheck/install-cppcheck.bat) : cppcheck をインストールする
        - msiexec.exe
    - [run-cppcheck.bat](run-cppcheck.bat) : cppcheck を実行する
        - cppcheck.exe
    - [build-installer.bat](build-installer.bat) : Installer をビルドする
        - [ISCC.exe](http://www.jrsoftware.org/isinfo.php) : InnoSetup でインストーラをビルドする
    - [zipArtifacts.bat](zipArtifacts.bat) : 成果物を zip で固める
        - [calc-hash.bat](calc-hash.bat) : 各成果物の sha256 を計算する
            - [calc-hash.py](calc-hash.py)
        - [zip.bat](tools/zip/zip.bat) : 成果物を zip に固める
            - [7z.exe](https://sevenzip.osdn.jp/)
            - [zip.ps1](tools/zip/zip.ps1) : powershell 版の ZIP ファイルの圧縮スクリプト
        - [listzip.bat](tools/zip/listzip.bat) : 成果物の zip の中身を確認する
            - [7z.exe](https://sevenzip.osdn.jp/)
            - [listzip.ps1](tools/zip/listzip.ps1) : powershell 版の ZIP ファイルの内容確認スクリプト

## ビルドに使用するバッチファイルの引数

| バッチファイル | 第一引数 | 第二引数 |
----|----|----
|build-all.bat       | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|build-sln.bat       | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|sakura\preBuild.bat | HeaderMake.exe または MakefileMake.exe の実行ファイルのフォルダパス | なし |
|sakura\postBuild.bat| platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|parse-buildlog.bat  | msbuild のビルドログパス | なし |
|build-chm.bat       | なし | なし |
|build-installer.bat | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|externals\cppcheck\install-cppcheck.bat | なし | なし |
|run-cppcheck.bat                        | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|zipArtifacts.bat    | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|calc-hash.bat       | sha256 のハッシュ値の出力先ファイル | ハッシュ値を計算するフォルダパス |

## バッチファイルの仕組み

### preBuild.bat の構造

#### 生成する環境変数

| 生成する環境変数 | 説明 | 有効性 |
----|----|----
| COMMITID                      | git の commit Hash                                  | git 環境が有効な場合              |
| SHORT_COMMITID                | git の commit Hash の先頭8文字                      | git 環境が有効な場合              |
| GIT_URL                       | git remote URL (origin)                             | git 環境が有効な場合              |
| GITHUB_COMMIT_URL             | gitHub で対応する commit に対する URL               | appveyor でのビルドのみ有効       |
| GITHUB_COMMIT_URL_PR_HEAD     | gitHub の Pull Request の commit に対応する URL     | appveyor での PR のビルドのみ有効 |
| APPVEYOR_SHORTHASH            | commit hash の先頭8文字                             | appveyor でのビルドのみ有効       |
| APPVEYOR_SHORTHASH_PR_HEAD    | Pull Request の commit hash の先頭8文字             | appveyor での PR のビルドのみ有効 |
| APPVEYOR_BUILD_URL            | appveyor でのビルドに対応する URL                   | appveyor でのビルドのみ有効       |

#### 処理の流れ

* HeaderMake.exe で [Funccode_x.hsrc](sakura_core/Funccode_x.hsrc) を入力に Funccode_define.h と Funccode_enum.h を生成する
* MakefileMake.exe で MinGW 用の [Makefile](sakura_core/Makefile) を更新する
* Git や appveyor の環境変数を元に githash.h を生成する (output_githash のサブルーチン)
 
 #### githash.h で生成するマクロ

 生成するマクロ名 | 元にする環境変数 | 型 |
----|----|----
| GIT_COMMIT_HASH                   | COMMITID                    |文字列   |
| SHORT_COMMITID                    | SHORT_COMMITID              |文字列   |
| GIT_URL                           | GIT_URL                     |文字列   |
| APPVEYOR_URL                      | APPVEYOR_URL                |文字列   |
| APPVEYOR_REPO_NAME                | APPVEYOR_REPO_NAME          |文字列   |
| APPVEYOR_ACCOUNT_NAME             | APPVEYOR_ACCOUNT_NAME       |文字列   |
| APPVEYOR_PROJECT_SLUG             | APPVEYOR_PROJECT_SLUG       |文字列   |
| APPVEYOR_BUILD_VERSION            | APPVEYOR_BUILD_VERSION      |文字列   |
| APPVEYOR_BUILD_NUMBER             | APPVEYOR_BUILD_NUMBER       |文字列   |
| APPVEYOR_BUILD_NUMBER_INT         | APPVEYOR_BUILD_NUMBER       |数値     |
| APPVEYOR_PULL_REQUEST_NUMBER      |APPVEYOR_PULL_REQUEST_NUMBER |文字列   |
| APPVEYOR_PULL_REQUEST_NUMBER_INT  |APPVEYOR_PULL_REQUEST_NUMBER |数値     |
| GITHUB_COMMIT_URL                 |GITHUB_COMMIT_URL            |文字列   |
| GITHUB_COMMIT_URL_PR_HEAD         |GITHUB_COMMIT_URL_PR_HEAD    |文字列   |
| APPVEYOR_SHORTHASH                |APPVEYOR_SHORTHASH           |文字列   |
| APPVEYOR_SHORTHASH_PR_HEAD        |APPVEYOR_SHORTHASH_PR_HEAD   |文字列   |
| APPVEYOR_BUILD_URL                |APPVEYOR_BUILD_URL           |文字列   |


### postBuild.bat の構造

#### 処理の流れ

* リポジトリに登録している bregonig の zipファイルを解凍して bregonig.dll を sakura.exe のビルドの出力先にコピーする

### zipArtifacts.bat の構造

#### 生成する環境変数

| 生成する環境変数 | 説明 | 有効性 |
----|----|----
| ALPHA                         | alpha バージョンの場合 1                            | x64 ビルドの場合                       |
| BUILD_ACCOUNT                 | appveyor のビルドアカウント名                       | sakura editor 用のアカウントの場合空   |
| TAG_NAME                      | "tag_" + tag 名                                     | tag が有効な場合                       |
| BUILD_NUMBER                  | "build" + appveyor のビルド番号                     | appveyor ビルド以外の場合 "buildLocal" |
| PR_NAME                       | "PR" + PR番号                                       | appveyor での PR のビルドのみ有効      |
| SHORTHASH                     | commit hash の先頭8文字                             | appveyor ビルドのみ有効                |
| RELEASE_PHASE                 | "alpha" または 空                                   | x64 ビルドの場合のみ有効               |
| BASENAME                      | 成果物の zip ファイル名(拡張子含まない部分)、zip 内部のフォルダ名       | 常に有効                               |
| WORKDIR                       | 作業用フォルダ、およびzip 内部のフォルダ名          | 常に有効                               |
| WORKDIR_LOG                   | ログファイル用の作業用フォルダ                      | 常に有効                               |
| WORKDIR_EXE                   | 実行ファイル用の作業用フォルダ                      | 常に有効                               |
| WORKDIR_INST                  | インストーラ用の作業用フォルダ                      | 常に有効                               |
| OUTFILE                       | 成果物の zip ファイル名                             | 常に有効                               |
| OUTFILE_LOG                   | ログファイルの成果物の zip ファイル名               | 常に有効                               |
| HASHFILE                      | sha256 のハッシュ値のファイル名                     | 常に有効                               |

#### 処理の流れ

* if 文の条件判定を元に、成果物のファイル名、フォルダ名を構築して環境変数に設定する
* 作業用フォルダに必要なファイルをコピーする
* [calc-hash.bat](calc-hash.bat) で sha256 のハッシュを計算して、作業用フォルダにコピーする
* 7z コマンドで作業用フォルダの中身を zip に固める
