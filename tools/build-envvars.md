# CI でのビルドにて使用される環境変数

## GitHub Actions が定義する環境変数

|環境変数名|説明|値の例|
|--|--|--|
|GITHUB_ACTIONS|GitHub Actionsで実行中かどうか|`true`|
|GITHUB_EVENT_NAME|ワークフローをトリガしたイベント名|`pull_request`, `push`|
|GITHUB_REF_TYPE|リファレンスの種類|`branch`, `tag`|
|GITHUB_SERVER_URL|GitHubサーバーのURL|`https://github.com`|
|GITHUB_REPOSITORY|リポジトリのフルパス|`sakura-editor/sakura`|
|GITHUB_SHA|コミットハッシュ (PRではマージコミット)|40桁のハッシュ値|

## CI が定義する環境変数

|変数名|定義条件|定義仕様|
|--|--|--|
|CI_ACCOUNT_NAME|常に|`github.actor`|
|CI_REPO_NAME|常に|`github.repository`|
|CI_BUILD_VERSION|常に|`github.run_id`|
|CI_BUILD_NUMBER|常に|`github.run_number`|
|CI_BUILD_URL|常に|`github.server_url`/`github.repository`/actions/runs/`github.run_id`|
|GITHUB_COMMIT_URL|常に|`github.server_url`/`github.repository`/commit/`github.sha`|
|GITHUB_PR_NUMBER|pull_request のみ|`github.event.pull_request.number`|
|GITHUB_PR_HEAD_SHORT_COMMIT|pull_request のみ|`github.event.pull_request.head.sha` の先頭8文字|
|GITHUB_PR_HEAD_COMMIT|pull_request のみ|`github.event.pull_request.head.sha`|
|GITHUB_PR_HEAD_URL|pull_request のみ|`github.server_url`/`github.repository`/pull/`github.event.pull_request.number`/commits/`github.event.pull_request.head.sha`|

## version.cmake が CI ビルドで生成するマクロ

|変数名|定義条件|定義仕様|
|--|--|--|
|BUILD_ENV_NAME|常に|"GHA"|
|GIT_REMOTE_ORIGIN_URL|常に|`GITHUB_SERVER_URL`/`GITHUB_REPOSITORY`|
|GIT_COMMIT_HASH|常に|``|
|BUILD_VERSION|.git がある|カレントブランチの累積コミット数|
|DEV_VERSION|32bit以外|(値なし)|
|CI_BUILD_NUMBER_INT|pull_request のみ|CI_BUILD_NUMBERを数値化。|
|CI_BUILD_NUMBER_LABEL|pull_request のみ|"Build %CI_BUILD_NUMBER%"|
|GITHUB_PR_NUMBER_INT|pull_request のみ|GITHUB_PR_NUMBERを数値化。|
|GITHUB_PR_NUMBER_LABEL|pull_request のみ|"PR %GITHUB_PR_NUMBER%"|
|GITHUB_TAG_NAME|タグビルドのみ|`GITHUB_REF_NAME`|

[CI が定義した環境変数](#ci-が定義する環境変数)はそのまま定義されます。

## version.cmake がローカルビルドで生成するマクロ

|変数名|定義条件|定義仕様|
|--|--|--|
|BUILD_ENV_NAME|常に|"Local"|
|BUILD_VERSION|.git がある|カレントブランチの累積コミット数|
|GIT_COMMIT_HASH|.git がある|`git show -s --format=%H`|
|GIT_SHORT_COMMIT_HASH|.git がある|GIT_COMMIT_HASH の先頭8文字|
|GIT_REMOTE_ORIGIN_URL|.git がある|`git config --get remote.origin.url`|

## zipArtifacts.bat で設定する環境変数

### 生成する環境変数

|生成する環境変数|説明|有効性|
----|----|----
|ALPHA|alphaバージョンの場合1|x64ビルドの場合|
|BUILD_ACCOUNT|CIのビルドアカウント名|sakuraeditor用のアカウントの場合空|
|TAG_NAME|"tag_"+tag名|tagが有効な場合|
|BUILD_NUMBER|"build"+ビルド番号|CIビルド以外の場合"buildLocal"|
|PR_NAME|"PR"+PR番号|CIでのPRのビルドのみ有効|
|SHORTHASH|commithashの先頭8文字|実体はGIT_SHORT_COMMIT_HASH|
|RELEASE_PHASE|"alpha"または空|x64ビルドの場合のみ有効|
|BASENAME|成果物のzipファイル名(拡張子含まない部分)|常に有効|
|WORKDIR|作業用フォルダー|常に有効|
|WORKDIR_LOG|ログファイル用の作業用フォルダー|常に有効|
|WORKDIR_EXE|実行ファイル(一般向け)用の作業用フォルダー|常に有効|
|WORKDIR_DEV|開発者向け成果物用の作業用フォルダー|常に有効|
|WORKDIR_INST|インストーラ用の作業用フォルダー|常に有効|
|WORKDIR_ASM|アセンブラ出力用の作業用フォルダー|常に有効|
|OUTFILE|成果物のzipファイル名|常に有効|
|OUTFILE_LOG|ログファイルの成果物のzipファイル名|常に有効|
|OUTFILE_EXE|実行ファイル(一般向け)の成果物のzipファイル名|常に有効|
|OUTFILE_DEV|開発者向け成果物のzipファイル名|常に有効|
|OUTFILE_INST|インストーラの成果物のzipファイル名|常に有効|
|OUTFILE_ASM|アセンブラ出力の成果物のzipファイル名|常に有効|
