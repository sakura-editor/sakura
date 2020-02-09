# appveyor でのビルド

<!-- TOC -->

- [appveyor でのビルド](#appveyor-でのビルド)
    - [CI でのビルドをスキップする方法](#ci-でのビルドをスキップする方法)
        - [参考サイト](#参考サイト)
    - [CI Buildおよびローカルビルドの環境変数](#CI-Buildおよびローカルビルドの環境変数)
    - [ビルドに使用するバッチファイル](#ビルドに使用するバッチファイル)
    - [呼び出し構造](#呼び出し構造)
    - [ビルドに使用するバッチファイルの引数](#ビルドに使用するバッチファイルの引数)
    - [バッチファイルの仕組み](#バッチファイルの仕組み)
        - [preBuild.bat の構造](#prebuildbat-の構造)
            - [生成する環境変数](#生成する環境変数)
            - [処理の流れ](#処理の流れ)
        - [postBuild.bat の構造](#postbuildbat-の構造)
            - [処理の流れ](#処理の流れ-1)
        - [mingw32-del.bat の構造](#mingw32-delbat-の構造)
            - [処理の流れ](#処理の流れ-2)
        - [zipArtifacts.bat の構造](#zipartifactsbat-の構造)
            - [処理の流れ](#処理の流れ-3)

<!-- /TOC -->

## CI でのビルドをスキップする方法

ビルドに関係ない修正 (ドキュメントの修正など)を行った場合に、コミットメッセージの中に
`[ci skip]` を含めることで、Appveyor での CI ビルドを行わないようにすることができます。
ただし PR をマージするときには、CI のビルドは走ります。

### 参考サイト

- https://srz-zumix.blogspot.com/2018/03/ci-ci-20183.html
- https://www.appveyor.com/docs/how-to/filtering-commits/#skip-commits


## CI Buildおよびローカルビルドの環境変数

[CI でのビルド](ci-build.md) を参照

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
|[help\make-artifacts.bat](help\make-artifacts.bat) | compiled HTML ファイルを zip に固めるバッチファイル |
|[help\extract-chm-from-artifact.ps1](help\extract-chm-from-artifact.ps1) | ビルド済み compiled HTML ファイルをダウンロードして解凍するバッチファイル |
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
        - [remove-comment.py](help/remove-comment.py) : UTF-8 でのコンパイルエラーの回避のために日本語を削除するために [sakura.hh](sakura_core/sakura.hh) から一行コメントを削除する
        - hhc.exe (Visual Studio 2017 に同梱) : compiled HTML をビルドするコンパイラ。かなり古いツールであり、日本語HTMLをビルドするためにはWindowsのシステムロケールを日本語に変更する必要がある。
        - [help\extract-chm-from-artifact.ps1](help\extract-chm-from-artifact.ps1) : compiled HTML が必要なタイミングで、あらかじめ作成したビルド済みCHMをダウンロード＆解凍してその場でビルドを行ったのと同じ状態にする。
    - [help\make-artifacts.bat](help\make-artifacts.bat) : HTML Help を zip に固める
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
|help\make-artifacts.bat | なし | なし |
|help\extract-chm-from-artifact.ps1 | なし | なし |
|build-installer.bat | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|run-cppcheck.bat                        | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|run-doxygen.bat                         | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|zipArtifacts.bat    | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|calc-hash.bat       | sha256 のハッシュ値の出力先ファイル | ハッシュ値を計算するフォルダパス |

## バッチファイルの仕組み

### preBuild.bat の構造

#### 処理の流れ

* HeaderMake.exe で [Funccode_x.hsrc](sakura_core/Funccode_x.hsrc) を入力に Funccode_define.h と Funccode_enum.h を生成する
* MakefileMake.exe で MinGW 用の [Makefile](sakura_core/Makefile) を更新する
* Git や appveyor の環境変数を元に githash.h を生成する (output_githash のサブルーチン)
 
### postBuild.bat の構造

#### 処理の流れ

* リポジトリに登録している bregonig の zipファイルを解凍して bregonig.dll を sakura.exe のビルドの出力先にコピーする

### mingw32-del.bat の構造

#### 処理の流れ

* 引数で渡されたパスに含まれるスラッシュ(`/`)をバックスラッシュ(`\`)に置換してdelコマンドに渡し、ファイルを削除する

### zipArtifacts.bat の構造

#### 処理の流れ

* if 文の条件判定を元に、成果物のファイル名、フォルダ名を構築して環境変数に設定する
* 作業用フォルダに必要なファイルをコピーする
* [calc-hash.bat](calc-hash.bat) で sha256 のハッシュを計算して、作業用フォルダにコピーする
* 7z コマンドで作業用フォルダの中身を zip に固める
