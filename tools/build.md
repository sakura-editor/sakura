# ビルド方法

<!-- TOC -->

- [ビルド方法](#ビルド方法)
  - [必要なツール](#必要なツール)
    - [サクラエディタ本体のビルド](#サクラエディタ本体のビルド)
      - [PowerShell 7の準備](#powershell-7の準備)
      - [Pythonの準備](#pythonの準備)
      - [uvの準備](#uvの準備)
    - [インストーラのビルド](#インストーラのビルド)
  - [ビルド手順](#ビルド手順)
    - [ソースコードの取得とvcpkgの準備](#ソースコードの取得とvcpkgの準備)
    - [実行ファイルのみ](#実行ファイルのみ)
      - [GUI でビルド](#gui-でビルド)
      - [コマンドラインでビルド](#コマンドラインでビルド)
    - [すべてビルド](#すべてビルド)
  - [開発者向け情報](#開発者向け情報)
    - [テスト実行時の注意](#テスト実行時の注意)
    - [ビルドで使用する環境変数](#ビルドで使用する環境変数)
    - [ビルドに使用されるバッチファイル](#ビルドに使用されるバッチファイル)
    - [デバッグ方法](#デバッグ方法)
    - [githash.h の更新をスキップ](#githashh-の更新をスキップ)
    - [PowerShellによるZIPファイル処理の強制](#powershellによるzipファイル処理の強制)
    - [CIビルドのスキップ](#ciビルドのスキップ)
  - [MinGWビルド (実験的)](#mingwビルド-実験的)
    - [MinGWでのビルド方法](#mingwでのビルド方法)
  - [参考情報](#参考情報)
    - [Chocolatey関連](#chocolatey関連)
      - [Chocolateyのインストール](#chocolateyのインストール)
    - [Msys2関連](#msys2関連)
      - [Msys2のインストール](#msys2のインストール)
      - [Msys2コンソールを開く方法](#msys2コンソールを開く方法)
    - [MinGW w64関連](#mingw-w64関連)
      - [MinGW w64のインストール](#mingw-w64のインストール)

<!-- /TOC -->

## 必要なツール

### サクラエディタ本体のビルド

C++20をサポートするC++コンパイラーが必要です。

- [最新のVisual Studio][Visual Studio 最新版] (推奨)
- [以前のバージョンのVisual Studio][Visual Studio 以前のバージョン]
  - Visual Studio 2019 以降
- MinGW64 GCCコンパイラー

Visual Studio Installerで使用するVisual Studioの「構成のインポート」から、リポジトリ直下の.vsconfigを選択し、不足しているコンポーネントを追加してください。

**補助ツールとして以下が必須です。**
|ツール名|exe名|説明|
|--|--|--|
|HTML Help Workshop|hhc.exe|Visual Studio同梱のもの|
|CMake|cmake.exe|Visual Studio同梱のもので可|
|PowerShell 7|pwsh.exe|Windows標準とは別に用意するので[PowerShell 7の準備](#powershell-7の準備)を参照してください。|
|Python|python.exe|バージョンや開発用ファイルの条件は[Pythonの準備](#pythonの準備)を参照してください。|
|uv|uv.exe|ビルド時のCMake設定で必要です。[uvの準備](#uvの準備)を参照してください。|
|[7-Zip](https://7-zip.opensource.jp/)|7z.exe|外部依存ファイルの解凍に使用します。|
|Locale Emulator|LEProc.exe|日本語環境以外でHTMLヘルプをビルドする場合に利用します。|
|Auto HotKey|AutoHotKey.exe|日本語環境以外でHTMLヘルプをビルドする場合にソースに腹持ちしたLocale Emulatorを展開する際に利用します。|

#### PowerShell 7の準備

Windows標準のWindows PowerShell（`powershell.exe`）とは別に、PowerShell 7（`pwsh.exe`）が必要です。
例えば、次のコマンドで導入できます。

```powershell
winget install --id Microsoft.PowerShell --exact --source winget
```

導入後はVisual Studioとターミナルを起動し直し、新しいターミナルで確認してください。

```powershell
pwsh --version
```

#### Pythonの準備

Python 3.14以降が必要です。
ビルド対象に合わせて、x64には64bit版、Win32には32bit版を導入してください。
開発用ヘッダー（`Python.h`など）と`python3.dll`も必要です。
例えば、x64ビルド用には次のコマンドで導入できます。

```powershell
winget install --id Python.Python.3.14 --exact --architecture x64
```

Pythonマクロのテストを実行する場合は、`python3.dll`があるPythonのインストールフォルダーをPATHに追加してください。

PATHの変更後はVisual Studioとターミナルを起動し直してください。

#### uvの準備

uvはテストを実行しない場合でも、ビルド時のCMake設定で必要です。
例えば、次のコマンドで導入できます。

```powershell
winget install --id astral-sh.uv --exact --source winget
```

導入後はVisual Studioとターミナルを起動し直し、新しいターミナルで確認してください。

```powershell
uv --version
```

### インストーラのビルド

インストーラをビルドする場合のみ必要です。

- [Inno Setup](https://jrsoftware.org/isdl.php) (ISCC.exe)
  - 推奨バージョン: [innosetup-6.4.0.exe](https://files.jrsoftware.org/is/6/)

```pwsh
choco install InnoSetup -y
```

詳細は [インストーラビルドの仕組み](../installer/readme.md) を参照してください。

## ビルド手順

### ソースコードの取得とvcpkgの準備

このリポジトリでは、vcpkgや外部ライブラリをGitサブモジュールとして管理しています。
GitHubの「Source code (zip)」などを展開しただけではサブモジュールが揃わないため、Gitにより取得してください。
新しくcloneする場合は、作業用フォルダーで次のコマンドを実行します。

```powershell
git clone --recurse-submodules https://github.com/sakura-editor/sakura.git
```

既に通常の`git clone`で取得済みの場合は、`sakura.sln`があるフォルダーで次のコマンドを実行してください。

```powershell
git submodule update --init --recursive
```

サブモジュールを取得すると、Visual Studioまたは`build-sln.bat`による初回ビルド時にvcpkgが自動で初期化され、必要な外部ライブラリが取得・ビルドされます。
通常は`bootstrap-vcpkg.bat`を手動実行する必要はありません。
うまくいかない場合、`sakura.sln`があるフォルダーで以下を試してください。

```powershell
.\tools\vcpkg\bootstrap-vcpkg.bat
```

### 実行ファイルのみ

#### GUI でビルド

Visual Studio で `sakura.sln` を開いてビルドします。

#### コマンドラインでビルド

```cmd
build-sln.bat <Platform> <Configuration>
```

**例: x64 の Release ビルド**
```cmd
build-sln.bat x64 Release
```

**Visual Studio 2019を指定してビルド**
```cmd
set ARG_VSVERSION=16
build-sln.bat Win32 Release
```

参考: [MSBuildの検索について](./find-tools.md#MSBuild) で `ARG_VSVERSION` の詳細を説明しています。

### すべてビルド

実行ファイル、ヘルプファイル、インストーラをすべてビルドします。

```cmd
build-all.bat <Platform> <Configuration>
```

**例: Win32 の Release ビルド**
```cmd
build-all.bat Win32 Release
```

## 開発者向け情報

### テスト実行時の注意

- Pythonマクロのテストには、[Pythonの準備](#pythonの準備)に記載したPATHの設定も必要です。

- Documents配下では、パスの簡易表示によって`CSakuraEnvironmentTest.ExpandParameter_mixed`が失敗する場合があります。
期待値のフルパスに対し、実際の値が「マイドキュメント」で始まる場合は、Documents配下以外の作業フォルダーで確認してください。

### ビルドで使用する環境変数

[ビルドで使用する環境変数](./build-envvars.md) を参照してください。

### ビルドに使用されるバッチファイル

[ビルドに使用されるバッチファイル](./build-batchfiles.md) を参照してください。

### デバッグ方法

- [タスクトレイのメニュー項目をデバッグする方法](./debug-tasktray-menu.md)
- [大きなファイルの作成方法](./create-big-file.md)

### githash.h の更新をスキップ

ビルド時に git の commit hash を `githash.h` に出力します。これによりバイナリが commit hash から特定できますが、バイナリが変化しないリファクタリングでもバイナリが異なってしまいます。

検証を容易にするため、環境変数 `SKIP_CREATE_GITHASH` を `1` に設定することで commit hash の更新をスキップできます。

**注意:** `githash.h` が存在しない場合は、この環境変数に関係なく生成されます。

**実行例:**
```cmd
set SKIP_CREATE_GITHASH=1
build-sln.bat Win32 Release
build-sln.bat Win32 Debug
build-sln.bat x64 Release
build-sln.bat x64 Debug
```

### PowerShellによるZIPファイル処理の強制

通常、`7z.exe` が利用可能な場合は自動的に使用されます（高速）。デバッグ目的で [PowerShellスクリプト](./zip/readme.md) を強制的に使用する場合:

```cmd
set FORCE_POWERSHELL_ZIP=1
build-sln.bat Win32 Release
```

### CIビルドのスキップ

ドキュメント修正など、ビルドが不要な変更の場合、コミットメッセージに `[ci skip]` または `[skip ci]` を含めることでCIビルドをスキップできます。

**注意:** PRマージ時は実行されます。

**参考:**
- https://qiita.com/vmmhypervisor/items/f10c77a375c2a663b300
- https://github.blog/changelog/2021-02-08-github-actions-skip-pull-request-and-push-workflows-with-skip-ci/

## MinGWビルド (実験的)

**警告:** 生成されるバイナリは正しく動作しません。

### MinGWでのビルド方法

[MinGW w64のインストール](#mingw-w64のインストール) を完了後、以下の方法でビルドできます。

MINGW64コンソールで以下を実行。
```bash
cmake -S . -B build/MinGW -DCMAKE_BUILD_TYPE=Debug -DBUILD_PLATFORM=MinGW
cmake --build build/MinGW
ctest --test-dir build/MinGW --output-on-failure
```

または、コマンドプロンプトで以下を実行。
```cmd
build-gnu.bat MinGW Debug
build-gnu.bat MinGW Release
```

## 参考情報

### Chocolatey関連

#### Chocolateyのインストール

1. PowerShell管理者コンソールを開く:
   - Windowsタスクバーの検索窓に `powershell` と入力
   - `Windows PowerShell` を右クリックして「管理者として実行」

2. 以下のコマンドを実行:
   ```powershell
   Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
   ```

3. インストール確認:
   ```powershell
   choco
   ```

詳細: [Chocolatey公式サイト](https://chocolatey.org/install)

### Msys2関連

#### Msys2のインストール

[Chocolatey](#chocolateyのインストール) をインストール後、PowerShell管理者コンソールで実行:

```powershell
choco install msys2 --params "/InstallDir:C:\msys64"
```

#### Msys2コンソールを開く方法

`C:\msys64\msys2.exe` を実行します。

### MinGW w64関連

#### MinGW w64のインストール

1. [Chocolatey](#chocolateyのインストール) をインストール
2. [Msys2](#msys2のインストール) をインストール
3. [Msys2コンソール](#msys2コンソールを開く方法) を開く
4. pacmanパッケージを最新化:
   ```bash
   pacman -Syuu
   ```
5. MinGW-w64をインストール:
   ```bash
   pacman -S --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make mingw-w64-x86_64-7zip
   ```

<!-- リンク定義 -->
[Visual Studio 以前のバージョン]: https://visualstudio.microsoft.com/ja/vs/older-downloads/ "Visual Studio 以前のバージョン"
[Visual Studio 最新版]: https://visualstudio.microsoft.com/ja/downloads/ "Visual Studio 最新版"
