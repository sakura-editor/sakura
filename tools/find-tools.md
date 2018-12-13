# ビルド関連ツールのパスを探す
外部ツールを利用する前にデフォルトのインストールパスにある実行ファイルを自動で検索します。
デフォルトパスにインストールしていない場合や別の実行ファイルを使いたい場合、実行前にパスを通しておくかCMD_7Zなどの対応する環境変数に絶対パスをセットしておいてください。

## 外部ツールの一覧
|      ツール名      |   環境変数   | ソフトのデフォルトパス |  ファイル名  |
| ------------------ | ------------ | ---------------------- | ------------ |
| 7-Zip              | CMD_7Z       | 7-Zip                  | 7z           |
| HTML Help Workshop | CMD_HHC      | HTML Help Workshop%    | hhc.exe      |
| Inno Setup 5       | CMD_ISCC     | Inno Setup 5           | ISCC.exe     |
| cppcheck           | CMD_CPPCHECK | cppcheck               | cppcheck.exe |
| doxygen            | CMD_DOXYGEN  | doxygen\bin            | doxygen.exe  |
| MSBuild            | CMD_MSBUILD  | 特殊                   |              |

## MSBuild以外の探索手順
MSBuild以外の探索手順は同一であり、7-Zipを例に説明する。

1. CMD_7Zがセットされていればそれを使う
2. パスが通っていればそれを使う
3. 以下のディレクトリ内の「ソフトのデフォルトパス」で見つかればそれを使う
    * %ProgramFiles%
    * %ProgramFiles(x86)
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
