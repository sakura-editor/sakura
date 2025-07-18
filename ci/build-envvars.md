﻿# CI でのビルドにて使用される環境変数

## githash.bat で設定する環境変数

|||ローカルビルド|gitbash.h への出力|
|--|--|--|--|
|GIT_COMMIT_HASH|git の commit Hash|◎|◎|
|GIT_SHORT_COMMIT_HASH|git の commit Hash 短縮形|◎|◎|
|GIT_REMOTE_ORIGIN_URL|git remote origin URL|◎|◎|
|GIT_TAG_NAME|git の tag|◎|◎|
|CI_ACCOUNT_NAME|CI のアカウント名|×|◎|
|CI_REPO_NAME|リポジトリ名|×|◎|
|CI_BUILD_VERSION|CI のビルドバージョン (文字列)|×|◎|
|CI_BUILD_NUMBER|CI のビルド番号 (数値)|×|◎|
|CI_BUILD_URL|CI のビルドURL|×|◎|
|GITHUB_COMMIT_URL|GitHub の Commit URL|×|◎|
|GITHUB_PR_NUMBER|GitHub の PR 番号|×|○ (PRのみ)|
|GITHUB_PR_HEAD_COMMIT | GitHub の PR の Head commit Hash | × |○ (PRのみ) |
|GITHUB_PR_HEAD_SHORT_COMMIT | GitHub の PR の Head commit Hash 短縮形 | × |○ (PRのみ) |
|GITHUB_PR_HEAD_URL | GitHub の PR の Head commit の URL | × |○ (PRのみ) |

## 入力として使用する環境変数

## githash.h で生成するマクロ

|生成するマクロ名|元にする環境変数|型|
|--|--|--|
|GIT_SHORT_COMMIT_HASH|GIT_SHORT_COMMIT_HASH|文字列|
|GIT_COMMIT_HASH|GIT_COMMIT_HASH|文字列|
|GIT_REMOTE_ORIGIN_URL|GIT_REMOTE_ORIGIN_URL|文字列|
|GIT_TAG_NAME|GIT_TAG_NAME|文字列|
|CI_REPO_NAME|CI_REPO_NAME|文字列|
|DEV_VERSION|-|-|
|CI_ACCOUNT_NAME|CI_ACCOUNT_NAME|文字列|
|CI_BUILD_VERSION|CI_BUILD_VERSION|文字列|
|CI_BUILD_NUMBER|CI_BUILD_NUMBER|文字列|
|CI_BUILD_NUMBER_INT|CI_BUILD_NUMBER|数値|
|CI_BUILD_NUMBER_LABEL|CI_BUILD_NUMBER|文字列|
|GITHUB_PR_NUMBER|GITHUB_PR_NUMBER|文字列|
|GITHUB_PR_NUMBER_INT|GITHUB_PR_NUMBER|数値|
|GITHUB_PR_NUMBER_LABEL|GITHUB_PR_NUMBER|文字列|
|GITHUB_COMMIT_URL|GITHUB_COMMIT_URL|文字列|
|GITHUB_PR_HEAD_URL|GITHUB_PR_HEAD_URL|文字列|
|GITHUB_PR_HEAD_COMMIT|GITHUB_PR_HEAD_COMMIT|文字列|
|GITHUB_PR_HEAD_SHORT_COMMIT|GITHUB_PR_HEAD_SHORT_COMMIT|文字列|
|CI_BUILD_URL|CI_BUILD_URL|文字列|

## zipArtifacts.bat で設定する環境変数

### 生成する環境変数

| 生成する環境変数 | 説明 | 有効性 |
----|----|----
| ALPHA         | alpha バージョンの場合 1                           | x64 ビルドの場合                       |
| BUILD_ACCOUNT | CI のビルドアカウント名                            | sakura editor 用のアカウントの場合空   |
| TAG_NAME      | "tag_" + tag 名                                    | tag が有効な場合                       |
| BUILD_NUMBER  | "build" + ビルド番号                               | CI ビルド以外の場合 "buildLocal"       |
| PR_NAME       | "PR" + PR番号                                      | CI での PR のビルドのみ有効            |
| SHORTHASH     | commit hash の先頭8文字                            | 実体は GIT_SHORT_COMMIT_HASH      |
| RELEASE_PHASE | "alpha" または 空                                  | x64 ビルドの場合のみ有効               |
| BASENAME      | 成果物の zip ファイル名(拡張子含まない部分)        | 常に有効                               |
| WORKDIR       | 作業用フォルダー                                     | 常に有効                               |
| WORKDIR_LOG   | ログファイル用の作業用フォルダー                     | 常に有効                               |
| WORKDIR_EXE   | 実行ファイル(一般向け)用の作業用フォルダー           | 常に有効                               |
| WORKDIR_DEV   | 開発者向け成果物用の作業用フォルダー                 | 常に有効                               |
| WORKDIR_INST  | インストーラ用の作業用フォルダー                     | 常に有効                               |
| WORKDIR_ASM   | アセンブラ出力用の作業用フォルダー                   | 常に有効                               |
| OUTFILE       | 成果物の zip ファイル名                            | 常に有効                               |
| OUTFILE_LOG   | ログファイルの成果物の zip ファイル名              | 常に有効                               |
| OUTFILE_EXE   | 実行ファイル(一般向け)の成果物の zip ファイル名    | 常に有効                               |
| OUTFILE_DEV   | 開発者向け成果物の zip ファイル名                  | 常に有効                               |
| OUTFILE_INST  | インストーラの成果物の zip ファイル名              | 常に有効                               |
| OUTFILE_ASM   | アセンブラ出力の成果物の zip ファイル名            | 常に有効                               |
| HASHFILE      | sha256 のハッシュ値のファイル名                    | 常に有効                               |
