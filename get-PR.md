# PR(Pull Request) を簡単にローカルに取得する方法

<!-- TOC -->

- [PR(Pull Request) を簡単にローカルに取得する方法](#prpull-request-を簡単にローカルに取得する方法)
    - [コマンドライン引数](#コマンドライン引数)
    - [remote name](#remote-name)
    - [動作](#動作)
    - [使用例](#使用例)
        - [例1: PR36 をローカルに取得したい場合 (SAKURA_EDITOR_REMOTE_NAME を設定していない場合)](#例1-pr36-をローカルに取得したい場合-sakura_editor_remote_name-を設定していない場合)
        - [例2: Fork したリポジトリで PR40 を取得したい場合](#例2-fork-したリポジトリで-pr40-を取得したい場合)

<!-- /TOC -->

```
get-PR.bat <PR-Number> [<remote name>]
```


## コマンドライン引数

|引数名|意味|必須|例|
|:--|:--|:--|:--|
|PR-Number|PR 番号|○|100|
|remote name|git remote の名前|×|origin|

## remote name

PR を取得するための remote name の取得先

|優先順位|取得先|
|:--|:--|
|1|バッチファイルの第2引数|
|2|環境変数 SAKURA_EDITOR_REMOTE_NAME|
|3|origin|


## 動作


`PR-xx/yyyymmdd-HHMMSS` というローカルブランチに取得してチェックアウトします。 
実行した後に PR が更新された場合は単純に再実行します。

|フィールド名|意味|
|--|--|
|xx|PR番号|
|yyyy|西暦|
|mm|月|
|dd|日|
|HH|時間|
|MM|分|
|SS|秒|


## 使用例

### 例1: PR36 をローカルに取得したい場合 (SAKURA_EDITOR_REMOTE_NAME を設定していない場合)

```
git clone https://github.com/sakura-editor/sakura.git
cd sakura
get-PR.bat 36
```

### 例2: Fork したリポジトリで PR40 を取得したい場合

`<forkname>` は適宜 GitHub 上での自分のユーザー名等に読み替えます。


```
git clone https://github.com/<forkname>/sakura.git
cd sakura
git remote add  upstream  https://github.com/sakura-editor/sakura.git
git fetch upstream
get-PR.bat 40 upstream

set SAKURA_EDITOR_REMOTE_NAME=upstream
get-PR.bat 40
```
