# CI でのビルド

## githash.bat で設定する環境変数

| 生成する環境変数 | 説明 | 有効性 |
----|----|----
| GIT_SHORT_COMMIT_HASH         | git の commit Hash の先頭8文字                      | git 環境が有効な場合              |
| GIT_COMMIT_HASH               | git の commit Hash                                  | git 環境が有効な場合              |
| GIT_REMOTE_ORIGIN_URL         | git remote origin URL                               | git 環境が有効な場合              |
| GIT_TAG_NAME                  | git の tag                                        | git 環境が有効な場合              |
| GITHUB_COMMIT_URL             | gitHub で対応する commit に対する URL               | appveyor でのビルドのみ有効       |
| GITHUB_COMMIT_URL_PR_HEAD     | gitHub の Pull Request の commit に対応する URL     | appveyor での PR のビルドのみ有効 |
| APPVEYOR_SHORTHASH_PR_HEAD    | Pull Request の commit hash の先頭8文字             | appveyor での PR のビルドのみ有効 |
| APPVEYOR_BUILD_URL            | appveyor でのビルドに対応する URL                   | appveyor でのビルドのみ有効       |
| TEMP_GIT_SHORT_COMMIT_HASH    | git の commit Hash の先頭8文字                      | appveyor でのビルドのみ有効 (暫定的な変数。将来整理予定) |
| TEMP_GIT_COMMIT_HASH          | git の commit Hash                                  | appveyor でのビルドのみ有効 (暫定的な変数。将来整理予定) |

## 入力として使用する環境変数

| 環境変数 | 説明 |
----|---- 
|APPVEYOR                           | バッチが appveyor で実行されているかどうか  |
|APPVEYOR_ACCOUNT_NAME              | appveyor のアカウント名 (sakura editor の場合 "sakuraeditor") |
|APPVEYOR_BUILD_NUMBER              | ビルド番号 |
|APPVEYOR_URL                       | https://ci.appveyor.com |
|APPVEYOR_BUILD_VERSION             | appveyor.yml の version フィールドの値 |
|APPVEYOR_BUILD_ID                  | ビルドID (ビルド結果URLに含まれる数値です。`build-chm.bat`が実行中のビルドを識別するために使います。) |
|APPVEYOR_PROJECT_SLUG              | project slug (appveyor の URL 名) |
|APPVEYOR_PULL_REQUEST_NUMBER       | Pull Request 番号 |
|APPVEYOR_PULL_REQUEST_HEAD_COMMIT  | Pull Request の Head commit Hash |
|APPVEYOR_REPO_NAME                 | リポジトリ名 (owner-name/repo-name) |
|APPVEYOR_REPO_PROVIDER             | appveyor の参照するリポジトリ種別 (GitHub の場合 "gitHub") |
|~~READONLY_TOKEN~~                 | デバッグ用です。 appveyor の REST API に渡す [Bearer Token](https://www.appveyor.com/docs/api/#Authentication) をスクリプト外から渡せるように定義しています。 appveyor では使いません。(未定義なので値は''になります。) |

APPVEYOR_REPO_TAG_NAME は利用をやめて 代わりに GIT_TAG_NAME を使うようにしました。[#876](https://github.com/sakura-editor/sakura/pull/876)

* 上記環境変数をローカル環境で set コマンドで設定することにより appveyor でビルドしなくてもローカルでテストできます。
* 上記の環境変数がどんな値になるのかは、過去の appveyor ビルドでのログを見ることによって確認できます。
* `build-chm.bat`をローカルでテストするには完了済みのビルドIDが必要です。ビルドIDは[history](https://ci.appveyor.com/project/sakuraeditor/sakura/history)から各ビルド結果を表示するとURL末尾に付いている数字です。

## githash.h で生成するマクロ

| 生成するマクロ名                | 元にする環境変数                | 型       |
| ----                            | ----                            | ----     |
| GIT_SHORT_COMMIT_HASH           | GIT_SHORT_COMMIT_HASH           | 文字列   |
| GIT_COMMIT_HASH                 | GIT_COMMIT_HASH                 | 文字列   |
| GIT_REMOTE_ORIGIN_URL           | GIT_REMOTE_ORIGIN_URL           | 文字列   |
| GIT_TAG_NAME                    | GIT_TAG_NAME                    | 文字列   |
| APPVEYOR_URL                    | APPVEYOR_URL                    | 文字列   |
| APPVEYOR_REPO_NAME              | APPVEYOR_REPO_NAME              | 文字列   |
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
| APPVEYOR_SHORTHASH_PR_HEAD      | APPVEYOR_SHORTHASH_PR_HEAD      | 文字列   |
| APPVEYOR_BUILD_URL              | APPVEYOR_BUILD_URL              | 文字列   |
| TEMP_GIT_SHORT_COMMIT_HASH      | TEMP_GIT_SHORT_COMMIT_HASH      | 文字列   |
| TEMP_GIT_COMMIT_HASH            | TEMP_GIT_COMMIT_HASH            | 文字列   |

## zipArtifacts.bat で設定する環境変数

### 生成する環境変数

| 生成する環境変数 | 説明 | 有効性 |
----|----|----
| ALPHA         | alpha バージョンの場合 1                           | x64 ビルドの場合                       |
| BUILD_ACCOUNT | appveyor のビルドアカウント名                      | sakura editor 用のアカウントの場合空   |
| TAG_NAME      | "tag_" + tag 名                                    | tag が有効な場合                       |
| BUILD_NUMBER  | "build" + appveyor のビルド番号                    | appveyor ビルド以外の場合 "buildLocal" |
| PR_NAME       | "PR" + PR番号                                      | appveyor での PR のビルドのみ有効      |
| SHORTHASH     | commit hash の先頭8文字                            | 実体は TEMP_GIT_SHORT_COMMIT_HASH      |
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
