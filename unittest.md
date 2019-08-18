# 単体テスト

<!-- TOC -->

- [単体テスト](#単体テスト)
    - [doctest](#doctest)
    - [単体テストのディレクトリ構造](#単体テストのディレクトリ構造)
    - [単体テスト関連のバッチファイル](#単体テスト関連のバッチファイル)
        - [一覧](#一覧)
        - [呼び出し構造](#呼び出し構造)
        - [使用するバッチファイルの引数](#使用するバッチファイルの引数)
    - [インクルードディレクトリ](#インクルードディレクトリ)

<!-- /TOC -->

## doctest

単体テストフレームワークとしてオープンソースでいくつか公開されていますが、
[doctest](https://github.com/onqtam/doctest) を使用します。

## 単体テストのディレクトリ構造

- tests
    - unittests (単体テストの実体を置く。中の構成は要検討)
    - build (ビルド時に生成されるフォルダ。git には登録しない)
        - Win32 (Win32 用のプロジェクトを格納するフォルダ)
        - x64   (x64 用のプロジェクトを格納するフォルダ)

## 単体テスト関連のバッチファイル

### 一覧

| バッチファイル | 説明 |
----|---- 
|[tests\create-project.bat](tests/create-project.bat)| 単体テスト用のプロジェクトファイル作成するバッチファイル |
|[tests\build-project.bat](tests/build-project.bat)  | 単体テスト用のプロジェクトをビルドするバッチファイル |
|[tests\run-tests.bat](tests/run-tests.bat)          | 単体テストを実行するバッチファイル |
|[tests\build-and-test.bat](tests/build-and-test.bat)| appveyor.yml から呼ばれて上記を呼び出すバッチファイル  |

### 呼び出し構造

- [tests\build-and-test.bat](tests/build-and-test.bat)
    - [tests\create-project.bat](tests/create-project.bat)
        - cmake
    - [tests\build-project.bat](tests/build-project.bat)
        - cmake --build
    - [tests\run-tests.bat](tests/run-tests.bat)
        - tests*.exe を実行

### 使用するバッチファイルの引数

| バッチファイル | 第一引数 | 第二引数 |
----|----|----
|[tests\create-project.bat](tests/create-project.bat)| platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|[tests\build-project.bat](tests/build-project.bat)  | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|[tests\run-tests.bat](tests/run-tests.bat)          | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|[tests\build-and-test.bat](tests/build-and-test.bat)| platform ("Win32" または "x64" または "MinGW") | configuration ("Debug" または "Release")  |

## インクルードディレクトリ

[単体テスト用のCMakeLists.txt](tests/unittests/CMakeLists.txt) で [サクラエディタ用のディレクトリ](sakura_core) に
インクルードディレクトリを指定するので、そこからの相対パスを指定すれば、サクラエディタのヘッダをインクルードできます。
