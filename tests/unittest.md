# 単体テスト

<!-- TOC -->

- [単体テスト](#単体テスト)
  - [GoogleTest](#googletest)
  - [CMake](#cmake)
  - [単体テストのディレクトリ構造](#単体テストのディレクトリ構造)
  - [単体テスト関連のバッチファイル](#単体テスト関連のバッチファイル)
    - [一覧](#一覧)
    - [呼び出し構造](#呼び出し構造)
    - [使用するバッチファイルの引数](#使用するバッチファイルの引数)
  - [インクルードディレクトリ](#インクルードディレクトリ)

<!-- /TOC -->

## GoogleTest

単体テストフレームワークとしてオープンソースでいくつか公開されていますが、
Google が開発している [GoogleTest](https://github.com/google/googletest) を使用します。

[GoogleTest](https://github.com/google/googletest) は GitHub でソースコードが公開されているので
git submodule の機能を使用してソースコードを参照してテストと同時にビルドします。

## CMake

GoogleTest はビルドシステムに [CMake](https://cmake.org/) を使用します。
CMake は CMakeLists.txt というテキストファイルにビルド設定を記載して
CMake を実行することにより、各プラットフォームに固有のネイティブの
ビルドシステム用のファイルを生成してビルドします。

GoogleTest を使用して作成する単体テストも CMake でビルド設定を行います。

CMake は Visual Studio 用のプロジェクト生成に対応しているため
Visual Studio 用のプロジェクトを生成してビルドすれば 
GUI でステップ実行することができます。

## 単体テストのディレクトリ構造

- tests
    - compiletests (コンパイルテスト用のファイルを置くディレクトリ)
    - googletest (googletest 用のフォルダー。git submodule)
    - unittests (単体テストの実体を置く。中の構成は要検討)
    - build (ビルド時に生成されるフォルダー。git には登録しない)
        - Win32 (Win32 用のプロジェクトを格納するフォルダー)
        - x64   (x64 用のプロジェクトを格納するフォルダー)

## 単体テスト関連のバッチファイル

### 一覧

| バッチファイル | 説明 |
----|---- 
|[tests\build-and-test.bat](build-and-test.bat) | ビルドと単体テストを順に行う |
|[tests\create-project.bat](create-project.bat) | 単体テスト用のプロジェクトファイルを作成する |
|[tests\build-project.bat](build-project.bat) | 単体テスト用のプロジェクトをビルドする |
|[tests\run-tests.bat](run-tests.bat) | 単体テストを実行する |

### 呼び出し構造

- [tests\build-and-test.bat](build-and-test.bat)
    - [tests\create-project.bat](create-project.bat)
        - [build-gnu.bat](../build-gnu.bat) : Makefile をビルドする (platform が MinGW の場合)
        - [build-sln.bat](../build-sln.bat) : ソリューションをビルドする (platform が Win32 または x64 の場合)
        - git.exe
        - cmake.exe
    - [tests\build-project.bat](build-project.bat)
        - git.exe
        - cmake.exe
    - [tests\run-tests.bat](run-tests.bat)
        - tests\tests*.exe : 単体テストを実行する
        - [tests\test_result_filter_tell_AppVeyor.bat](test_result_filter_tell_AppVeyor.bat) : AppVeyor のテストタブにテスト結果を出力する

### 使用するバッチファイルの引数

| バッチファイル | 第一引数 | 第二引数 |
----|----|----
|[tests\build-and-test.bat](build-and-test.bat) | platform ("Win32" または "x64" または "MinGW") | configuration ("Debug" または "Release")  |
|[tests\create-project.bat](create-project.bat) | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|[tests\build-project.bat](build-project.bat) | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|[tests\run-tests.bat](run-tests.bat) | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |

## インクルードディレクトリ

単体テスト用の [CMakeLists.txt](unittests/CMakeLists.txt) で [サクラエディタ用のディレクトリ](../sakura_core) を
インクルードディレクトリに指定しているので、そこからの相対パスを指定すれば、サクラエディタのヘッダーをインクルードできます。
