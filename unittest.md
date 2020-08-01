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

- / (リポジトリルート)
  - build (ビルド時に生成されるフォルダ。git には登録しない)
    - Win32 (Win32 用の中間生成物を格納するフォルダ)
    - x64 (x64 用の中間生成物を格納するフォルダ)
    - MinGW (MinGW-w64 用の中間生成物を格納するフォルダ)
  - tests
    - googletest (googletest 用のフォルダ。git submodule)
    - unittests (単体テストの実体を置くフォルダ)
  - Win32 (Win32 用の生成物を格納するフォルダ)
  - x64 (x64 用の生成物を格納するフォルダ)
  - MinGW (MinGW-w64 用の生成物を格納するフォルダ)

## 単体テスト関連のバッチファイル

### 一覧

| バッチファイル | 説明 |
----|---- 
|[tests\googletest.build.cmd](tests/googletest.build.cmd)  | 単体テスト用のプロジェクトをビルドするバッチファイル |
|[tests\run-tests.bat](tests/run-tests.bat)          | 単体テストを実行するバッチファイル |
|[tests\build-and-test.bat](tests/build-and-test.bat)| appveyor.yml から呼ばれて上記を呼び出すバッチファイル  |

### 呼び出し構造

- [tests\build-and-test.bat](tests/build-and-test.bat)
  - [build-sln.bat](build-sln.bat) : MsBuildを使ってソリューションをビルドする
    - msbuild sakura.sln
      - [tests\googletest.build.cmd](tests/googletest.build.cmd)
  - [build-gnu.bat](build-gnu.bat) : CMakeを使ってMinGW-w64プロジェクトを生成してビルドする
    - cmake(configure)
      - [tests\googletest.build.cmd](tests/googletest.build.cmd)
  - [tests\run-tests.bat](tests/run-tests.bat)
    - tests*.exe を実行

### 使用するバッチファイルの引数

| バッチファイル | 第一引数 | 第二引数 |
----|----|----
|[tests\googletest.build.cmd](tests/googletest.build.cmd)  | platform ("Win32" または "x64" または "MinGW") | configuration ("Debug" または "Release")  |
|[tests\run-tests.bat](tests/run-tests.bat)          | platform ("Win32" または "x64" または "MinGW") | configuration ("Debug" または "Release")  |
|[tests\build-and-test.bat](tests/build-and-test.bat)| platform ("Win32" または "x64" または "MinGW") | configuration ("Debug" または "Release")  |

## インクルードディレクトリ

[単体テスト用のCMakeLists.txt](tests/unittests/CMakeLists.txt) で [サクラエディタ用のディレクトリ](sakura_core) に
インクルードディレクトリを指定するので、そこからの相対パスを指定すれば、サクラエディタのヘッダをインクルードできます。
