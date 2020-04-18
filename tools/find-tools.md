- [ビルド関連ツールのパスを探す](#%e3%83%93%e3%83%ab%e3%83%89%e9%96%a2%e9%80%a3%e3%83%84%e3%83%bc%e3%83%ab%e3%81%ae%e3%83%91%e3%82%b9%e3%82%92%e6%8e%a2%e3%81%99)
  - [外部ツールの一覧](#%e5%a4%96%e9%83%a8%e3%83%84%e3%83%bc%e3%83%ab%e3%81%ae%e4%b8%80%e8%a6%a7)
  - [MSBuild以外の探索手順](#msbuild%e4%bb%a5%e5%a4%96%e3%81%ae%e6%8e%a2%e7%b4%a2%e6%89%8b%e9%a0%86)
  - [MSBuild](#msbuild)
    - [ユーザーがビルドに使用する Visual Studio のバージョンを切り替える方法](#%e3%83%a6%e3%83%bc%e3%82%b6%e3%83%bc%e3%81%8c%e3%83%93%e3%83%ab%e3%83%89%e3%81%ab%e4%bd%bf%e7%94%a8%e3%81%99%e3%82%8b-visual-studio-%e3%81%ae%e3%83%90%e3%83%bc%e3%82%b8%e3%83%a7%e3%83%b3%e3%82%92%e5%88%87%e3%82%8a%e6%9b%bf%e3%81%88%e3%82%8b%e6%96%b9%e6%b3%95)
    - [検索ロジック](#%e6%a4%9c%e7%b4%a2%e3%83%ad%e3%82%b8%e3%83%83%e3%82%af)
    - [参照](#%e5%8f%82%e7%85%a7)
  - [zipの処理に7zではなくPowerShellを強制する](#zip%e3%81%ae%e5%87%a6%e7%90%86%e3%81%ab7z%e3%81%a7%e3%81%af%e3%81%aa%e3%81%8fpowershell%e3%82%92%e5%bc%b7%e5%88%b6%e3%81%99%e3%82%8b)

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
| cppcheck           | CMD_CPPCHECK | cppcheck           | cppcheck.exe |
| doxygen            | CMD_DOXYGEN  | doxygen\bin        | doxygen.exe  |
| vswhere            | CMD_VSWHERE  | Microsoft Visual Studio\Installer | vswhere.exe  |
| MSBuild            | CMD_MSBUILD  | 特殊               | MSBuild.exe  |

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

環境変数 ```ARG_VSVERSION``` の値でビルドに使用するバージョンを切り替えられる。

| ARG_VSVERSION  | 使用される Visual Studio のバージョン  |
| -------------- | ------------------------------------- |
| 空             | インストールされている Visual Studio の最新   |
| 15             | Visual Studio 2017                           |
| 16             | Visual Studio 2019                           |
| 2017           | Visual Studio 2017                           |
| 2019           | Visual Studio 2019                           |

### 検索ロジック

1. `ARG_VSVERSION` から`VC++`のバージョン指定を判断する。
  1.1.  `ARG_VSVERSION` 未指定の場合、`15`が指定されたものとみなす。
  1.2.  `ARG_VSVERSION` が`2017`の場合、`15`が指定されたものとみなす。
  1.3.  `ARG_VSVERSION` が`2019`の場合、`16`が指定されたものとみなす。
  1.4.  `ARG_VSVERSION` が`latest`の場合、最新バージョンを取得する。
  1.3.  `ARG_VSVERSION` が上記以外の場合、`ARG_VSVERSION` の値をバージョン指定とみなす。
2. 指定されたバージョンのVC++がインストールされているかチェックする。  
  2.1.  ```vswhere -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath -version [指定されたバージョン, 指定されたバージョン+1)``` を実行する。
  2.2.  取得したパスが存在していたら、バージョン指定は正しいとみなす。
  2.3.  取得したパスが存在していなかったら、最新バージョンを取得する。
3. インストール済みのVC++の最新バージョンを取得する。
  3.1.  ```vswhere -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion -latest``` を実行する。
  3.2.  取得したバージョンが指定されたものとみなす。
4. ```vswhere -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe -version [指定されたバージョン, 指定されたバージョン+1)``` で MSBuild.exe を検索する
  4.1. `vswhere` が VS2019 以降の `vswhere` の場合、`MSBuild.exe` が見つかるので検索終了
  4.2. `vswhere` が VS2017 以前の `vswhere` の場合、エラーになるので検索続行
5. VS2017 の vswhere の場合
  5.1 ```vswhere -version [15^,16^) -requires Microsoft.Component.MSBuild -property installationPath``` で VS2017 のインストールパスを検索する
  5.2 ```%Vs2017InstallRoot%\MSBuild\15.0\Bin\MSBuild.exe``` が存在する場合そのパスを MSBuild.exe のパスとする
  5.3 この場合、バージョンは`15`が指定されたものとみなす。

### 参照
* https://github.com/Microsoft/vswhere
* https://github.com/Microsoft/vswhere/wiki/Find-MSBuild

## zipの処理に7zではなくPowerShellを強制する
事前に環境変数の`FORCE_POWERSHELL_ZIP`を1にセットすることで、7zの検索をスキップできます。
[PowerShellによるzipの処理](zip/readme.md)が正しく行われるかを7zがインストール済みの環境で検証する際に活用できます。[build.md](../build.md#powershell-によるzipファイルの圧縮解凍内容確認の強制)も参照してください。
