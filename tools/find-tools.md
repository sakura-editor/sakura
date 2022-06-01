- [ビルド関連ツールのパスを探す](#%E3%83%93%E3%83%AB%E3%83%89%E9%96%A2%E9%80%A3%E3%83%84%E3%83%BC%E3%83%AB%E3%81%AE%E3%83%91%E3%82%B9%E3%82%92%E6%8E%A2%E3%81%99)
  - [外部ツールの一覧](#%E5%A4%96%E9%83%A8%E3%83%84%E3%83%BC%E3%83%AB%E3%81%AE%E4%B8%80%E8%A6%A7)
  - [MSBuild以外の探索手順](#msbuild%E4%BB%A5%E5%A4%96%E3%81%AE%E6%8E%A2%E7%B4%A2%E6%89%8B%E9%A0%86)
  - [MSBuild](#msbuild)
    - [参照](#%E5%8F%82%E7%85%A7)
  - [zipの処理に7zではなくPowerShellを強制する](#zip%E3%81%AE%E5%87%A6%E7%90%86%E3%81%AB7z%E3%81%A7%E3%81%AF%E3%81%AA%E3%81%8Fpowershell%E3%82%92%E5%BC%B7%E5%88%B6%E3%81%99%E3%82%8B)

# ビルド関連ツールのパスを探す
外部ツールの実行ファイルの位置を探し、見つかれば環境変数にセットします。現在パスが通っている場所と、インストーラがデフォルトでインストールする場所が検索されます。詳細は[MSBuild以外の探索手順](#msbuild%E4%BB%A5%E5%A4%96%E3%81%AE%E6%8E%A2%E7%B4%A2%E6%89%8B%E9%A0%86)の項目を参照してください。

外部ツールのインストール時にデフォルトパスにインストールしていない場合や別の実行ファイルを使いたい場合、実行前にパスを通しておくかCMD_7Zなどの対応する環境変数に絶対パスをセットしておいてください。

## 外部ツールの一覧
|      ツール名      |   環境変数   |   デフォルトパス   |  ファイル名  |
| ------------------ | ------------ | ------------------ | ------------ |
| Git for Windows    | CMD_GIT      | Git\Cmd            | git.exe      |
| 7-Zip              | CMD_7Z       | 7-Zip              | 7z           |
| HTML Help Workshop | CMD_HHC      | HTML Help Workshop | hhc.exe      |
| Inno Setup 5       | CMD_ISCC     | Inno Setup 5       | ISCC.exe     |
| Cppcheck           | CMD_CPPCHECK | cppcheck           | cppcheck.exe |
| Doxygen            | CMD_DOXYGEN  | doxygen\bin        | doxygen.exe  |
| Visual Studio Locator | CMD_VSWHERE | Microsoft Visual Studio\Installer | vswhere.exe |
| MSBuild            | CMD_MSBUILD  | 特殊               | MSBuild.exe  |
| Locale Emulator    | CMD_LEPROC   | なし               | LEProc.exe   |
| Python             | CMD_PYTHON   | なし               | py.exe (python.exe) |
| CMake              | CMD_CMAKE    | CMake\bin          | cmake        |
| Ninja              | CMD_NINJA    | なし               | ninja        |

## MSBuild以外の探索手順
MSBuild以外の探索手順は同一であり、7-Zipを例に説明する。

1. CMD_7Zがセットされていればそれを使う
2. パスが通っていればそれを使う
3. 以下のディレクトリ内の「デフォルトパス」を順に探索し、実行ファイルが見つかればそれを使う
    * %ProgramFiles%
    * %ProgramFiles(x86)%
    * %ProgramW6432%
4. 1～3で見つからなければCMD_7Zには何もセットしない

## MSBuild

### ユーザーがビルドに使用する Visual Studio のバージョンを切り替える方法

環境変数 `NUM_VSVERSION` の値でビルドに使用するバージョンを切り替えられる。

| NUM_VSVERSION  | 使用される Visual Studio のバージョン  |
| -------------- | -------------------------------------- |
| （未定義）     | インストールされている最新のバージョン |
| 15             | Visual Studio 2017                     |
| 16             | Visual Studio 2019                     |
| 17             | Visual Studio 2022                     |

### 検索ロジック

1. バッチファイルの引数をチェックする。
   1. 引数が指定されていない場合、`NUM_VSVERSION`が定義されていればその値を、そうでなければ`latest`を指定したものとみなす。
   2. 引数にプロダクトバージョン（例：`2019`）が指定されている場合は値をメジャーバージョンに変換する。
      `latest`を指定した場合は、実行環境にインストールされている最新のメジャーバージョンを取得する。
   3. 指定したバージョンがインストールされているか確認し、見つからなければ引数の指定はなかったものとみなしてチェックをやり直す。
   4. `NUM_VSVERSION`が指定されている場合に限り、設定されている値とここまでのチェックで見つかったバージョンが同じであるか確認する。
      もし異なっている場合は環境変数を初期化した上で引数チェックをやり直す。
2. 引数チェックで決定したバージョンの MSBuild を探す。
   - Visual Studio 2017 が選択された場合
      - VS2017 のインストールパスを取得し、配下の所定位置に`MSBuild.exe`が存在する場合、そのパスを`MSBuild`のパスとして利用する。
      - CMakeのジェネレータ名を示す`CMAKE_G_PARAM`に`Visual Studio 15 2017`を設定する。
   - Visual Studio 2019 以降が選択された場合
      - Visual Studio Locator の`-find`オプションを利用して`MSBuild.exe`を検索し、見つかったパスを`MSBuild`のパスとして利用する。
      - `NUM_VSVERSION`と別途取得したプロダクトバージョンからCMakeのジェネレータ名を生成し、`CMAKE_G_PARAM`に設定する。

### 参照
* https://github.com/Microsoft/vswhere
* https://github.com/Microsoft/vswhere/wiki/Find-MSBuild

## python

ビルドバッチで利用する Python インタープリタの存在確認をします。
適切な Python インタープリタが見つかると、環境変数 `CMD_PYTHON` が定義されます。
適切な Python インタープリタが見つからない場合、 `CMD_PYTHON` は定義されません。
Python インタープリタはビルド要件ではないので、 Python を利用するバッチには `CMD_PYTHON` チェックを挟む必要があります。

1. Python Launcher (py.exe) が存在し、 `py.exe --version` でバージョンが取れたら、それを使う。
1. パスが通っているpython.exeで`python.exe --version`してバージョンが取れたら、それを使う。


## zipの処理に7zではなくPowerShellを強制する
事前に環境変数の`FORCE_POWERSHELL_ZIP`を1にセットすることで、7zの検索をスキップできます。
[PowerShellによるzipの処理](zip/readme.md)が正しく行われるかを7zがインストール済みの環境で検証する際に活用できます。[build.md](../build.md#powershell-によるzipファイルの圧縮解凍内容確認の強制)も参照してください。
