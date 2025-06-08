# ビルド方法

<!-- TOC -->

- [ビルド方法](#ビルド方法)
  - [必要なもの](#必要なもの)
    - [実行ファイルのビルドに必要なもの](#実行ファイルのビルドに必要なもの)
    - [HTML ヘルプのビルドに必要なもの](#html-ヘルプのビルドに必要なもの)
    - [インストーラのビルドに必要なもの](#インストーラのビルドに必要なもの)
    - [(オプション) ログの解析等に必要なもの](#オプション-ログの解析等に必要なもの)
  - [ビルド方法](#ビルド方法-1)
    - [実行ファイルだけをビルドする場合](#実行ファイルだけをビルドする場合)
      - [方法1 (GUI)](#方法1-gui)
      - [方法2 (コマンドライン)](#方法2-コマンドライン)
        - [具体例 (x64 の Release)](#具体例-x64-の-release)
    - [すべてビルドする場合](#すべてビルドする場合)
        - [具体例 (Win32 の Release)](#具体例-win32-の-release)
      - [Visual Studio 2019 を使用してコマンドラインでビルド](#visual-studio-2019-を使用してコマンドラインでビルド)
  - [ビルドの仕組み](#ビルドの仕組み)
    - [appveyor でのビルドの仕組み](#appveyor-でのビルドの仕組み)
    - [Azure Pipelines でのビルドの仕組み](#azure-pipelines-でのビルドの仕組み)
    - [インストーラの仕組み](#インストーラの仕組み)
  - [開発者向けの情報](#開発者向けの情報)
    - [githash.h の更新のスキップ](#githashh-の更新のスキップ)
    - [Powershell によるZIPファイルの圧縮、解凍、内容確認の強制](#powershell-によるzipファイルの圧縮解凍内容確認の強制)
    - [CI でのビルドをスキップする方法](#ci-でのビルドをスキップする方法)
      - [参考情報](#参考情報)
    - [MinGW w64 ビルド](#mingw-w64-ビルド)
    - [MinGW w64 インストール方法](#mingw-w64-インストール方法)
    - [Msys2 コンソールを開く方法](#msys2-コンソールを開く方法)
    - [Msys2 インストール方法](#msys2-インストール方法)
    - [PowerShell の管理者コンソールを開く方法](#powershell-の管理者コンソールを開く方法)
    - [Chocolatey のインストール方法](#chocolatey-のインストール方法)

<!-- /TOC -->

## 必要なもの

### 実行ファイルのビルドに必要なもの

- (オプション) [7-Zip](https://7-zip.opensource.jp/) (外部依存ファイルの解凍に使用)
- Community または Professional エディション以上の Visual Studio 2017 または Visual Studio 2019
   - Windows 10 SDK のインストールも必要です。

### HTML ヘルプのビルドに必要なもの

- HTML ヘルプコンパイラ (hhc.exe)
   - Visual Studio のインストールにて以下のオプションを有効にすることにより導入されます。
      - 「C++ によるデスクトップ開発」を有効にする
      - 右のペインで 「C++ によるデスクトップ開発」を選ぶ
      - Visual Studio 2017 の場合は「x86用とx64用のVisual C++ MFC」を、Visual Studio 2019 の場合は「最新 v142 ビルド ツールの C++ MFC (x86 & x64)」をチェックする
      - 変更を確定する
   - VSインストール後でもVisual Studio Installerを起動して導入可能です。 
![vsi](https://user-images.githubusercontent.com/39618965/44622575-012dcc80-a8f6-11e8-906a-14d8cd6dfac9.PNG)
   - [.vsconfig](.vsconfig) を使用してインストールした場合は自動的にインストールされます。
   - ヘルプファイルを編集する場合 HTML Help Workshop 等の編集ソフトも別途必要になります。

### インストーラのビルドに必要なもの

インストーラビルドの仕組みは [こちら](installer/readme.md) を参照

- [Inno Setup](http://www.jrsoftware.org/isdl.php) の以下のいずれかのバージョンです。
    - [innosetup-5.5.9-unicode.exe](http://files.jrsoftware.org/is/5/) (appveyor でのビルドではこちらが使われます。)
    - [innosetup-5.6.1-unicode.exe](http://www.jrsoftware.org/isdl.php)

### (オプション) ログの解析等に必要なもの

- [Python](https://www.python.org/) の 2.7 系、3.x 系のどちらでも OK (のはず)

## ビルド方法

### 実行ファイルだけをビルドする場合

#### 方法1 (GUI)

Visual Studio で `sakura.sln` をダブルクリックして開いてビルドします。

#### 方法2 (コマンドライン)

```
build-sln.bat <Platform> <Configuration>
```

##### 具体例 (x64 の Release)

```
build-sln.bat x64 Release
```


### すべてビルドする場合

```
build-all.bat <Platform> <Configuration>
```

##### 具体例 (Win32 の Release)

```
build-all.bat Win32 Release
```

#### Visual Studio 2019 を使用してコマンドラインでビルド

```
set ARG_VSVERSION=16
build-all.bat Win32 Release
```

参考

[こちら](tools/find-tools.md#MSBuild) で ```ARG_VSVERSION``` に関して説明しています。


## ビルドの仕組み

### appveyor でのビルドの仕組み

AppVeyor では、 [build-all.bat](build-all.bat) を使用してビルドを行っています。
ビルドに使用されるバッチファイルについては [build-batchfiles.md](ci/build-batchfiles.md) を参照してください。

### Azure Pipelines でのビルドの仕組み

[こちら](ci/azure-pipelines/azure-pipelines.md) で [Azure Pipelines](https://azure.microsoft.com/ja-jp/services/devops/pipelines/) 上でのビルドの仕組みを説明しています。

### インストーラの仕組み

[こちら](installer/readme.md) でビルドの仕組みを説明しています。

## 開発者向けの情報

### githash.h の更新のスキップ

sakura editor ではビルド時に git の commit hash 等の情報を githash.h というファイルに出力します。
ビルド時に commit hash 等を生成することでビルド済みのバイナリがどの commit hash を元にビルドされたか
簡単に判断できて便利なのですが、

バイナリが変化しないリファクタリングをしたときでも、commit hash 等の変更が原因でバイナリ一致しなくなります。
これだと検証が面倒になるので、ローカルビルドで githash.h が変化しない手段を提供します。

コマンドラインで環境変数 ```SKIP_CREATE_GITHASH``` を 1 に設定することにより commit hash の
更新処理をスキップすることができます。githash.h が存在しない場合には、この環境変数が設定されていても
githash.h を生成します。

コマンド実行例

```
set SKIP_CREATE_GITHASH=1
build-sln.bat Win32 Release
build-sln.bat Win32 Debug
build-sln.bat x64   Release
build-sln.bat x64   Debug
```


### Powershell によるZIPファイルの圧縮、解凍、内容確認の強制

`7z.exe` へのパスが通っている場合または `C:\Program Files\7-Zip\7z.exe` が存在している場合は
`7z.exe` を、ZIP ファイルの解凍、圧縮、内容確認に使用します。

上記以外の場合は [powershell によるスクリプト](tools/zip/readme.md) により処理を行います。

`7z.exe` のほうがはるかに処理速度が速いので `7z.exe` が利用可能なら [powershell によるスクリプト](tools/zip/readme.md) を
使う理由は殆どないのですが、デバッグ目的で強制的に [powershell によるスクリプト](tools/zip/readme.md) を使用する手段を
提供します。

コマンドラインでビルドするときに事前に FORCE_POWERSHELL_ZIP を 1 に設定することにより
強制的に [powershell によるスクリプト](tools/zip/readme.md) を使用します。

コマンド実行例

```
set FORCE_POWERSHELL_ZIP=1
build-sln.bat Win32 Release
build-sln.bat Win32 Debug
build-sln.bat x64   Release
build-sln.bat x64   Debug
```

### CI でのビルドをスキップする方法

ビルドに関係ない修正 (ドキュメントの修正など) を行った場合に、
コミットメッセージの中に `[ci skip]` または `[skip ci]` というキーワードを含めることで、 CI ビルドを行わないようにすることができます。  
ただし PR をマージするときは実行されます。

#### 参考情報

- https://qiita.com/vmmhypervisor/items/f10c77a375c2a663b300
- https://www.appveyor.com/docs/how-to/filtering-commits/#skip-directive-in-commit-message
- https://docs.microsoft.com/ja-jp/azure/devops/pipelines/repos/azure-repos-git?view=azure-devops&tabs=yaml#skipping-ci-for-individual-commits
- https://github.blog/changelog/2021-02-08-github-actions-skip-pull-request-and-push-workflows-with-skip-ci/

### MinGW w64 ビルド

生成されるバイナリは正しく動作しないが、MinGWでのビルドも可能。

コマンド実行例（[MinGW w64](#mingw-w64-インストール方法) のインストールが必要。）

```cmd
build-gnu.bat MinGW Debug
build-gnu.bat MinGW Release
```

### MinGW w64 インストール方法

[Msys2 コンソール](#msys2-コンソールを開く方法) で以下のコマンドを入力し `pacman` パッケージ を最新化します。

```bash
pacman -Syuu
```

[Msys2 コンソール](#msys2-コンソールを開く方法) で以下のコマンドを入力し `MinGW-w64` をインストールします。  

```bash
pacman -S --noconfirm mingw-w64-x86_64-toolchain
```

### Msys2 コンソールを開く方法

`C:\msys64\msys2.exe` を実行します。（[Msys2](#msys2-インストール方法) のインストールが必要。）

### Msys2 インストール方法

[PowerShell の管理者コンソール](#powershell-の管理者コンソールを開く方法)で以下のコマンドを入力し `msys2` をインストールします。（[Chocolatey](#chocolatey-のインストール方法) のインストールが必要。）

```powershell
choco install msys2 --params "/InstallDir:C:\msys64"
```

### PowerShell の管理者コンソールを開く方法

Windowsタスクバーの検索窓に `powershell` と入力します。

検索結果に `Windows PowerShell (x86)` が表示されるので `管理者として実行` をクリックします。

### Chocolatey のインストール方法

[PowerShell の管理者コンソール](#powershell-の管理者コンソールを開く方法) で以下のコマンドを実行します。

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
```

詳細な手順は [Chocolateyの公式サイト](https://chocolatey.org/install) で確認してください。

[PowerShell の管理者コンソール](#powershell-の管理者コンソールを開く方法) で以下のコマンドを実行し `Chocolatey` のバージョンが表示されたらインストールできています。

```powershell
choco
```
