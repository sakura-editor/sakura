# 単体テスト

<!-- TOC -->

- [単体テスト](#単体テスト)
    - [googletest](#googletest)
    - [Test Adapter for Google Test](#Test%20Adapter%20for%20Google%20Test)
    - [NuGet](#NuGet)
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

~~[googletest](https://github.com/google/googletest) は github でソースコードが公開されているので
git submodule の機能を使用してソースコードを参照してテストと同時にビルドします。~~
ビルド済みパッケージを簡単に取得する方法があるので、今後はNuGetパッケージを使用します。

## Test Adapter for Google Test

Visual Studio用の[Google Test プラグイン](https://marketplace.visualstudio.com/items?itemName=VisualCPPTeam.TestAdapterforGoogleTest)です。
このプラグインを入れるとvs2017 IDEをgoogletestと連携させる機能とNuGetパッケージがインストールされます。
vs2017 ver15.5以降では、C++デスクトップ開発を選ぶとチェックされる推奨オプションになりました。
vs2017 ver15.9以降では、[vs2017 IDEを日本語で起動していても変なエラーは出なくなりました](http://nasatame.hatenablog.com/entry/2018/03/16/092507)。

## NuGet

[マイクロソフト](https://www.microsoft.com/)が開発した .NET Framework 向けのパッケージ管理システムです。
[NuGet](https://www.nuget.org/)は Visual Studio に組み込まれています。vs2017 IDEでビルドを行うにあたり、追加のツールは必要ありません。バッチファイルから(Msbuildで)ビルドする場合には、別途 NuGet.exe を用意する必要があるのでリポジトリ内に登録された NuGet.exe を利用してください。

NuGetでパッケージを取得するには、[packages.config](https://docs.microsoft.com/en-us/nuget/reference/packages-config)という名前のファイルを使います。
プロジェクトフォルダに packages.config を置いておくと、ソリューションフォルダに packages というフォルダが掘られてパッケージがダウンロードされる仕組みです。たまにsolutionを開いてすぐにダウンロードされないことがありますが、そういう時は「ソリューションエクスプローラー」でソリューションを右クリックし、「NuGetパッケージの復元」をします。エクスプローラーでソリューションフォルダを見てみるとpackagesフォルダが掘られてパッケージファイルが展開されているはずです。

packages.configを使わない方法もありますが、必要なvs2017インストールオプションが増えてややこしくなるので、あえてpackages.configを使う方法を採用しています。


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
