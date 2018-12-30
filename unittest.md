# 単体テスト

<!-- TOC -->

- [単体テスト](#単体テスト)
    - [googletest](#googletest)
    - [cmake](#cmake)
    - [単体テストのディレクトリ構造](#単体テストのディレクトリ構造)
    - [単体テスト関連のバッチファイル](#単体テスト関連のバッチファイル)
        - [一覧](#一覧)
        - [呼び出し構造](#呼び出し構造)
        - [使用するバッチファイルの引数](#使用するバッチファイルの引数)
    - [インクルードディレクトリ](#インクルードディレクトリ)
    - [appveyor への結果の反映](#appveyor-への結果の反映)
        - [googletest 側の対応](#googletest-側の対応)
        - [appveyor 側の仕様](#appveyor-側の仕様)
        - [実際の処理](#実際の処理)
        - [参考サイト](#参考サイト)

<!-- /TOC -->

## googletest

単体テストフレームワークとしてオープンソースでいくつか公開されていますが、
google が開発している [googletest](https://github.com/google/googletest) を使用します。

[googletest](https://github.com/google/googletest) は github でソースコードが公開されているので
git submodule の機能を使用してソースコードを参照してテストと同時にビルドします。

## cmake

googletest はビルドシステムに [cmake](https://cmake.org/) を使用します。
cmake は CMakeLists.txt というテキストファイルにビルド設定を記載して
cmake を実行することにより、各プラットフォームに固有のネイティブの
ビルドシステム用のファイルを生成してビルドします。

googletest を使用して作成する単体テストも cmake でビルド設定を行います。

cmake は Visual Studio 2017 用のプロジェクト生成に対応しているため
Visual Studio 2017 用のプロジェクトを生成してビルドすれば 
GUI でステップ実行することができます。

## 単体テストのディレクトリ構造

- tests
    - googletest (googletest 用のフォルダ。git submodule)
    - unittests (単体テストの実体を置く。中の構成は要検討)
    - build (ビルド時に生成されるフォルダ。git には登録しない)
        - Win32 (Win32 用のプロジェクトを格納するフォルダ)
        - x64   (x64 用のプロジェクトを格納するフォルダ)

## 単体テスト関連のバッチファイル

### 一覧

| バッチファイル | 説明 |
|--|--|
|[tests\create-project.bat](tests/create-project.bat)|単体テスト用のプロジェクトファイル作成するバッチファイル |
|[tests\build-project.bat](tests/build-project.bat)  |単体テスト用のプロジェクトをビルドするバッチファイル |
|[tests\run-tests.bat](tests/run-tests.bat)          |単体テストを実行するバッチファイル |
|[tests\upload.ps1](tests/upload.ps1)          |単体テストの結果の XML ファイルを appveyor にアップロードするバッチファイル|
|[tests\upload.bat](tests/upload.bat)          |[tests\upload.ps1](tests/upload.ps1) を呼び出すラッパー|
|[tests\build-and-test.bat](tests/build-and-test.bat)|appveyor.yml から呼ばれて上記を呼び出すバッチファイル  |

### 呼び出し構造

- [tests\build-and-test.bat](tests/build-and-test.bat)
    - [tests\create-project.bat](tests/create-project.bat)
        - cmake
    - [tests\build-project.bat](tests/build-project.bat)
        - cmake --build
    - [tests\run-tests.bat](tests/run-tests.bat)
        - tests*.exe を実行
        - [tests\upload.bat](tests/upload.bat)
            - [tests\upload.ps1](tests/upload.ps1)

### 使用するバッチファイルの引数

| バッチファイル/powershell スクリプト| 第一引数 | 第二引数 |
|--|--|--|
|[tests\create-project.bat](tests/create-project.bat)|platform|configuration|
|[tests\build-project.bat](tests/build-project.bat)  |platform|configuration|
|[tests\run-tests.bat](tests/run-tests.bat)          |platform|configuration|
|[tests\build-and-test.bat](tests/build-and-test.bat)|platform|configuration|
|[tests\upload.bat](tests/upload.bat)          |xml ファイル名|なし|
|[tests\upload.ps1](tests/upload.ps1)          |xml ファイル名|なし|

|引数の値 (platform)|
|--|
|Win32|
|x64|
|MinGW|

|引数の値 (configuration)|
|--|
|Debug|
|Release|

## インクルードディレクトリ

[単体テスト用のCMakeLists.txt](tests/unittests/CMakeLists.txt) で [サクラエディタ用のディレクトリ](sakura_core) に
インクルードディレクトリを指定するので、そこからの相対パスを指定すれば、サクラエディタのヘッダをインクルードできます。

## appveyor への結果の反映

### googletest 側の対応

googletest の実行ファイルの引数に `-gtest_output=xml:tests.xml` というような引数を渡すと通常のコンソールへの
出力に加えて、`tests.xml` というファイルに XML 形式で出力を行います。

### appveyor 側の仕様

- https://www.appveyor.com/docs/running-tests/#pushing-real-time-test-results-to-build-console
- https://www.appveyor.com/docs/running-tests/#uploading-xml-test-results

以下の URL に googletest の出力の XML ファイルをアップロードすることにより、appveyor 側にテスト結果を
認識させることができます。


```
https://ci.appveyor.com/api/testresults/{resultsType}/{jobId}
```

|項目|値|参考|
|--|--|--|
|resultsType|`junit` (googletest が出力する XML の場合)|https://help.appveyor.com/discussions/questions/1843-uploading-test-results-from-google-test-lib-output|
|jobId|環境変数 `APPVEYOR_JOB_ID` の値|https://www.appveyor.com/docs/environment-variables/|

### 実際の処理

[upload.ps1](upload.ps1) の powershell スクリプトで System.Net.WebClient を使うことにより
XML ファイルのアップロードを行います。

### 参考サイト

- http://opencv.jp/googletestdocs/advancedguide.html#adv-generating-an-xml-report
- https://help.appveyor.com/discussions/questions/1843-uploading-test-results-from-google-test-lib-output
- https://www.appveyor.com/docs/running-tests/#pushing-real-time-test-results-to-build-console
   - https://www.appveyor.com/docs/running-tests/#uploading-xml-test-results