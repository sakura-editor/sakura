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

1. `ARG_VSVERSION` から`VC++`のバージョン指定(`NUM_VSVERSION`)を判断する。
	1. `ARG_VSVERSION` 未指定の場合、`15`が指定されたものとみなす。
	1. `ARG_VSVERSION` が`2017`の場合、`15`が指定されたものとみなす。
	1. `ARG_VSVERSION` が`2019`の場合、`16`が指定されたものとみなす。
	1. `ARG_VSVERSION` が`latest`の場合、最新バージョンを取得する。
	1. `ARG_VSVERSION` が上記以外の場合、`%ARG_VSVERSION%`が指定されたものとみなす。
1. 指定されたバージョンのVC++がインストールされているかチェックする。  
	1. `vswhere -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath -version [%NUM_VSVERSION%, %NUM_VSVERSION% + 1)` を実行する。
	1. 取得したパスが存在していたら、バージョン指定(`NUM_VSVERSION`)は正しいとみなす。
	1. 取得したパスが存在していなかったら、最新バージョンを取得する。
1. インストール済み`VC++`の最新バージョンを取得する。
	1.  `vswhere -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion -latest` を実行する。
	1.  取得したバージョンが指定されたものとみなす。(`NUM_VSVERSION`に代入する。)
1. 指定されたバージョンの`MsBuild.exe`を検索する。
	1. `-find`オプションを付けて`MsBuild.exe`を検索する。(`vswhere -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe -version [%NUM_VSVERSION%, %NUM_VSVERSION% + 1)`)
	1. `vswhere` が VS2019 以降ver の場合、`MSBuild.exe` が見つかるので検索終了。
	1. `vswhere` が VS2017 以前ver の場合、`MSBuild.exe` が見つからない(エラーになる)ので検索続行。
1. VS2017 の `MsBuild.exe` を検索する。
	1. VS2017 のインストールパスを検索する。(`vswhere -requires Microsoft.Component.MSBuild -property installationPath -version [15^,16^)`)
	1. VS2017 のインストールパス配下の所定位置(`%Vs2017InstallRoot%\MSBuild\15.0\Bin`)に`MSBuild.exe`が存在する場合、そのパスを `MSBuild.exe` のパスとみなす。
		この場合、バージョン指定(`NUM_VSVERSION`)に`15`が指定されたものとみなす。

### 参照
* https://github.com/Microsoft/vswhere
* https://github.com/Microsoft/vswhere/wiki/Find-MSBuild

## zipの処理に7zではなくPowerShellを強制する
事前に環境変数の`FORCE_POWERSHELL_ZIP`を1にセットすることで、7zの検索をスキップできます。
[PowerShellによるzipの処理](zip/readme.md)が正しく行われるかを7zがインストール済みの環境で検証する際に活用できます。[build.md](../build.md#powershell-によるzipファイルの圧縮解凍内容確認の強制)も参照してください。
