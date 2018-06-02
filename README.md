# Sakura Editor
A free Japanese text editor for Windows

## Web Site
- [Sakura Editor Portal](http://sakura-editor.sourceforge.net/)
- [SourceForge Project Home](https://sourceforge.net/projects/sakura-editor/)

## Status
SourceForge から当 GitHub への移行検証中。

## Build Requirements
### Visual Studio Community 2017
- [Visual Studio Community 2017](https://www.visualstudio.com/downloads/)

#### Visual Studio Install options required
- Windows SDK
- Windows XP Support for C++
- C++に関するWindows XP サポート

More information: https://github.com/sakura-editor/sakura/issues/6

## How to build
Visual Studio Community 2017 で `sakura/sakura_vc2017.sln` を開いてビルド。

## PR を簡単にローカルに取得する方法

```
get-PR.bat PR-Number
```

### 例: PR #36 をローカルに取得したい場合

```
git clone https://github.com/sakura-editor/sakura.git
cd sakura
get-PR.bat 36
```

pull-request/36 というローカルブランチに取得してチェックアウトします。


## CI Build (AppVeyor)
本リポジトリの最新 master は以下の AppVeyor プロジェクト上で自動ビルドされます。  
https://ci.appveyor.com/project/kobake/sakura

最新のビルド結果（バイナリ）はここから取得できます。  
https://ci.appveyor.com/project/kobake/sakura/build/artifacts

最新以前のビルド結果は以下から参照できます。  
https://ci.appveyor.com/project/kobake/sakura/history
