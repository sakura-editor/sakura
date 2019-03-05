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
1. CMD_MSBUILDがセットされていればそれを使う
2. パスが通っていればそれを使う
3. Visual Studio 2017以降にあるMicrosoft Visual Studio\Installer\vswhere.exeを利用し、msbuild.exeを探す。
4. 1～3で見つからなければCMD_MSBUILDには何もセットしない

### 参照
* https://github.com/Microsoft/vswhere
* https://github.com/Microsoft/vswhere/wiki/Find-MSBuild

## zipの処理に7zではなくPowerShellを強制する
事前に環境変数の`FORCE_POWERSHELL_ZIP`を1にセットすることで、7zの検索をスキップできます。
[PowerShellによるzipの処理](zip/readme.md)が正しく行われるかを7zがインストール済みの環境で検証する際に活用できます。[build.md](../build.md#powershell-によるzipファイルの圧縮解凍内容確認の強制)も参照してください。
