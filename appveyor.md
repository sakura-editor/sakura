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
    - [キャッシュを利用する方法](#キャッシュを利用する方法)

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
|[build-gnu.bat](build-gnu.bat)| Makefile をビルドするバッチファイル |
|[sakura\preBuild.bat](sakura/preBuild.bat) | solution のビルド前に Visual Studio から呼ばれるバッチファイル |
|[sakura\postBuild.bat](sakura/postBuild.bat)| 特に何もしない |
|[sakura\mingw32-del.bat](sakura/mingw32-del.bat)| MinGW の clean でファイルを削除するバッチファイル |
|[parse-buildlog.bat](parse-buildlog.bat)    | ビルドログを解析するバッチファイル |
|[build-chm.bat](build-chm.bat)       | compiled HTML ファイルをビルドするバッチファイル |
|[build-installer.bat](build-installer.bat) | インストーラをビルドするバッチファイル |
|[run-cppcheck.bat](run-cppcheck.bat)       | cppcheck を実行するバッチファイル |
|[run-doxygen.bat](run-doxygen.bat)         | doxygen を実行するバッチファイル |
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
            - [appveyor_env.py](appveyor_env.py) : 環境変数を再現できる `set_appveyor_env.bat` を生成する。(成果物に含まれる)
            - [parse-buildlog.py](parse-buildlog.py)
                - [appveyor_env.py](appveyor_env.py)
    - [build-gnu.bat](build-gnu.bat) : (Platform="MinGW"のみ) Makefileをビルドしてbuild-all.batの処理を終了する
    - [build-chm.bat](build-chm.bat) : HTML Help をビルドする
        - hhc.exe (Visual Studio 2017 に同梱)
    - [run-cppcheck.bat](run-cppcheck.bat) : cppcheck を実行する
        - cppcheck.exe
    - [run-doxygen.bat](run-doxygen.bat) : doxygen を実行する
        - doxygen.exe
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
|build-all.bat       | platform ("Win32" または "x64" または "MinGW") | configuration ("Debug" または "Release")  |
|build-sln.bat       | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|build-gnu.bat       | platform ("MinGW") | configuration ("Debug" または "Release")  |
|sakura\preBuild.bat | HeaderMake.exe または MakefileMake.exe の実行ファイルのフォルダパス | なし |
|sakura\postBuild.bat| platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|sakura\mingw32-del.bat| 削除するファイルパス1 | 削除するファイルパス2(2つ目以降は省略可能)  |
|parse-buildlog.bat  | msbuild のビルドログパス | なし |
|build-chm.bat       | なし | なし |
|build-installer.bat | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|run-cppcheck.bat                        | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|run-doxygen.bat                         | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|zipArtifacts.bat    | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|calc-hash.bat       | sha256 のハッシュ値の出力先ファイル | ハッシュ値を計算するフォルダパス |

## バッチファイルの仕組み

### preBuild.bat の構造

#### 生成する環境変数

| 生成する環境変数 | 説明 | 有効性 |
----|----|----
| GIT_SHORT_COMMIT_HASH         | git の commit Hash の先頭8文字                      | git 環境が有効な場合              |
| GIT_COMMIT_HASH               | git の commit Hash                                  | git 環境が有効な場合              |
| GIT_REMOTE_ORIGIN_URL         | git remote origin URL                               | git 環境が有効な場合              |
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

| 生成するマクロ名                | 元にする環境変数                | 型       |
| ----                            | ----                            | ----     |
| GIT_SHORT_COMMIT_HASH           | GIT_SHORT_COMMIT_HASH           | 文字列   |
| GIT_COMMIT_HASH                 | GIT_COMMIT_HASH                 | 文字列   |
| GIT_REMOTE_ORIGIN_URL           | GIT_REMOTE_ORIGIN_URL           | 文字列   |
| APPVEYOR_URL                    | APPVEYOR_URL                    | 文字列   |
| APPVEYOR_REPO_NAME              | APPVEYOR_REPO_NAME              | 文字列   |
| APPVEYOR_REPO_TAG_NAME          | APPVEYOR_REPO_TAG_NAME          | 文字列   |
| APPVEYOR_ACCOUNT_NAME           | APPVEYOR_ACCOUNT_NAME           | 文字列   |
| APPVEYOR_PROJECT_SLUG           | APPVEYOR_PROJECT_SLUG           | 文字列   |
| APPVEYOR_BUILD_VERSION          | APPVEYOR_BUILD_VERSION          | 文字列   |
| APPVEYOR_BUILD_NUMBER           | APPVEYOR_BUILD_NUMBER           | 文字列   |
| APPVEYOR_BUILD_NUMBER_INT       | APPVEYOR_BUILD_NUMBER           | 数値     |
| APPVEYOR_BUILD_NUMBER_LABEL     | APPVEYOR_BUILD_NUMBER           | 数値     |
| APPVEYOR_PULL_REQUEST_NUMBER    | APPVEYOR_PULL_REQUEST_NUMBER    | 文字列   |
| APPVEYOR_PULL_REQUEST_NUMBER_INT| APPVEYOR_PULL_REQUEST_NUMBER    | 数値     |
| APPVEYOR_PR_NUMBER_LABEL        | APPVEYOR_PULL_REQUEST_NUMBER    | 文字列   |
| GITHUB_COMMIT_URL               | GITHUB_COMMIT_URL               | 文字列   |
| GITHUB_COMMIT_URL_PR_HEAD       | GITHUB_COMMIT_URL_PR_HEAD       | 文字列   |
| APPVEYOR_SHORTHASH              | APPVEYOR_SHORTHASH              | 文字列   |
| APPVEYOR_SHORTHASH_PR_HEAD      | APPVEYOR_SHORTHASH_PR_HEAD      | 文字列   |
| APPVEYOR_BUILD_URL              | APPVEYOR_BUILD_URL              | 文字列   |


### postBuild.bat の構造

#### 処理の流れ

* リポジトリに登録している bregonig の zipファイルを解凍して bregonig.dll を sakura.exe のビルドの出力先にコピーする

### mingw32-del.bat の構造

#### 処理の流れ

* 引数で渡されたパスに含まれるスラッシュ(`/`)をバックスラッシュ(`\`)に置換してdelコマンドに渡し、ファイルを削除する

### zipArtifacts.bat の構造

#### 生成する環境変数

| 生成する環境変数 | 説明 | 有効性 |
----|----|----
| ALPHA         | alpha バージョンの場合 1                           | x64 ビルドの場合                       |
| BUILD_ACCOUNT | appveyor のビルドアカウント名                      | sakura editor 用のアカウントの場合空   |
| TAG_NAME      | "tag_" + tag 名                                    | tag が有効な場合                       |
| BUILD_NUMBER  | "build" + appveyor のビルド番号                    | appveyor ビルド以外の場合 "buildLocal" |
| PR_NAME       | "PR" + PR番号                                      | appveyor での PR のビルドのみ有効      |
| SHORTHASH     | commit hash の先頭8文字                            | appveyor ビルドのみ有効                |
| RELEASE_PHASE | "alpha" または 空                                  | x64 ビルドの場合のみ有効               |
| BASENAME      | 成果物の zip ファイル名(拡張子含まない部分)        | 常に有効                               |
| WORKDIR       | 作業用フォルダ                                     | 常に有効                               |
| WORKDIR_LOG   | ログファイル用の作業用フォルダ                     | 常に有効                               |
| WORKDIR_EXE   | 実行ファイル(一般向け)用の作業用フォルダ           | 常に有効                               |
| WORKDIR_DEV   | 開発者向け成果物用の作業用フォルダ                 | 常に有効                               |
| WORKDIR_INST  | インストーラ用の作業用フォルダ                     | 常に有効                               |
| WORKDIR_ASM   | アセンブラ出力用の作業用フォルダ                   | 常に有効                               |
| OUTFILE       | 成果物の zip ファイル名                            | 常に有効                               |
| OUTFILE_LOG   | ログファイルの成果物の zip ファイル名              | 常に有効                               |
| OUTFILE_EXE   | 実行ファイル(一般向け)の成果物の zip ファイル名    | 常に有効                               |
| OUTFILE_DEV   | 開発者向け成果物の zip ファイル名                  | 常に有効                               |
| OUTFILE_INST  | インストーラの成果物の zip ファイル名              | 常に有効                               |
| OUTFILE_ASM   | アセンブラ出力の成果物の zip ファイル名            | 常に有効                               |
| HASHFILE      | sha256 のハッシュ値のファイル名                    | 常に有効                               |

#### 処理の流れ

* if 文の条件判定を元に、成果物のファイル名、フォルダ名を構築して環境変数に設定する
* 作業用フォルダに必要なファイルをコピーする
* [calc-hash.bat](calc-hash.bat) で sha256 のハッシュを計算して、作業用フォルダにコピーする
* 7z コマンドで作業用フォルダの中身を zip に固める

## キャッシュを利用する方法

ビルドの中間生成物をキャッシュすることで不要な再コンパイルを省きビルド時間を短縮できます。

### 設定方法

環境変数 `cache` に `enabled` という値を設定することで有効になります。

#### Web ページで設定する場合

1. AppVeyor のアカウントページでサクラエディタのプロジェクト(※クローンを含む)を開き Settings を選びます。
2. Environment を選び Environment Variables の項目で Add variable ボタンをクリックします。
3. name に `cache` を、value に `enabled` を入力します。
4. ページ最下部で忘れずに Save ボタンを押します。

#### appveyor.yml で設定する場合

1. appveyor.yml に `cache: disabled` という行があるので `cache: enabled` に書き換えます。

### 確認方法

Console ログに `Restoring build cache`, `Updating build cache` という行があれば有効になっています。

### その他の詳細

導入経緯を参照してください。

* [Issue #637 「ビルドキャッシュを使用するとすごく早いです。」](https://github.com/sakura-editor/sakura/issues/637)
* [Pull request #650 「AppVeyor のビルドキャッシュを利用してビルド時間を削減(※およそ５分)できるようなオプションを用意します。」](https://github.com/sakura-editor/sakura/pull/650)
