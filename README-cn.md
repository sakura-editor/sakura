# Sakura Editor
[![Build status](https://ci.appveyor.com/api/projects/status/xlsp22h1q91mh96j/branch/master?svg=true)](https://ci.appveyor.com/project/sakuraeditor/sakura/branch/master)
[![Github Releases All](https://img.shields.io/github/downloads/sakura-editor/sakura/total.svg)](https://github.com/sakura-editor/sakura/releases "All Releases")
[![License: Zlib](https://img.shields.io/badge/License-Zlib-lightgrey.svg)](https://opensource.org/licenses/Zlib)
[![CodeFactor](https://www.codefactor.io/repository/github/sakura-editor/sakura/badge)](https://www.codefactor.io/repository/github/sakura-editor/sakura)
[![Build Status](https://dev.azure.com/sakuraeditor/sakura/_apis/build/status/sakura-editor.sakura?branchName=master)](https://dev.azure.com/sakuraeditor/sakura/_build/latest?definitionId=3&branchName=master)
[![build sakura](https://github.com/sakura-editor/sakura/workflows/build%20sakura/badge.svg)](https://github.com/sakura-editor/sakura/actions?query=workflow%3A%22build+sakura%22)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=sakura-editor_sakura&metric=alert_status)](https://sonarcloud.io/dashboard?id=sakura-editor_sakura)
[![Star History](https://img.shields.io/badge/star-histroy-yellow.svg)](https://star-history.t9t.io/#sakura-editor/sakura)

<!-- 以下は Markdownの参照形式によるリンク の定義です。 -->
<!-- 参照 https://hail2u.net/blog/coding/markdown-reference-style-links.html -->

[Visual Studio 以前のバージョン]: https://visualstudio.microsoft.com/ja/vs/older-downloads/ "Visual Studio 以前のバージョン"
[Visual Studio 最新版]: https://visualstudio.microsoft.com/ja/downloads/ "Visual Studio 最新版"
[ライセンスの OSI のページ]: https://opensource.org/license/zlib
[Visual Studio Community ライセンス]: https://visualstudio.microsoft.com/ja/license-terms/vs2022-ga-community/
[Markdown をローカルで確認する方法]: https://github.com/sakura-editor/sakura/wiki/markdown-%E3%82%92%E3%83%AD%E3%83%BC%E3%82%AB%E3%83%AB%E3%81%A7%E7%A2%BA%E8%AA%8D%E3%81%99%E3%82%8B%E6%96%B9%E6%B3%95
[How to extract currently installed Visual Studio component IDs?]: https://stackoverflow.com/questions/52946333/how-to-extract-currently-installed-visual-studio-component-ids
[Configure Visual Studio across your organization with .vsconfig]: https://devblogs.microsoft.com/setup/configure-visual-studio-across-your-organization-with-vsconfig/
[インストール構成をインポートまたはエクスポートする]: https://docs.microsoft.com/ja-jp/visualstudio/install/import-export-installation-configurations?view=vs-2019
[コマンド ライン パラメーターを使用して Visual Studio をインストールする]: https://docs.microsoft.com/ja-jp/visualstudio/install/use-command-line-parameters-to-install-visual-studio?view=vs-2019
[不足しているコンポーネントを自動的にインストールする]: https://docs.microsoft.com/ja-jp/visualstudio/install/import-export-installation-configurations?view=vs-2019#automatically-install-missing-components

<!-- TOC -->

- [Sakura Editor](#sakura-editor)
  - [最新动态](#hot-topic)
  - [官方网站](#web-site)
  - [开发参与政策](#開発参加ポリシー)
  - [构建要求](#build-requirements)
    - [Visual Studio 安装必要组件](#visual-studio-install-options-required)
    - [关于支持 Visual Studio 2019 及后续版本](#visual-studio-2019-以降の対応に関して)
    - [关于 .vsconfig 文件](#vsconfig-に関して)
      - [全版本通用配置](#visual-studio-2017-以降共通)
      - [仅限 Visual Studio 2019 及更新版本](#visual-studio-2019-以降のみ)
      - [参考链接](#参照)
  - [构建方法](#how-to-build)
    - [详细信息](#詳細情報)
  - [CI构建及本地构建的环境变量](#ci-buildおよびローカルビルドの環境変数)
  - [PR(Pull Request) 本地快速获取方法](#prpull-request-を簡単にローカルに取得する方法)
  - [CI构建 (Azure Pipelines)](#ci-build-azure-pipelines)
    - [构建机制说明 (Azure Pipelines)](#ビルドの仕組み-azure-pipelines)
  - [CI构建 (AppVeyor)](#ci-build-appveyor)
    - [构建机制说明 (AppVeyor)](#ビルドの仕組み-appveyor)
    - [使用构建产物的注意事项](#ビルド成果物を利用する上での注意事項)
    - [构建产物下载指南（二进制文件、安装程序等）](#ビルド成果物のダウンロードバイナリインストーラなど)
      - [master分支最新版本](#master-の-最新)
      - [master分支历史版本](#master-の-最新以外)
  - [开发信息](#開発情報)
    - [单元测试](#単体テスト)
    - [调试方法](#デバッグ方法)
  - [更新日志](#変更履歴)
  - [宏命令示例](#マクロのサンプル)
  - [静态代码分析](#静的コード解析)

<!-- /TOC -->

一款适用于 Windows 的免费日语文本编辑器

## 最新动态
开始使用项目管理看板：

- [Projects](https://github.com/orgs/sakura-editor/projects)
- [看板管理](https://github.com/sakura-editor/sakura/wiki/ProjectOperation)

## 官方网站
- [Sakura Editor 门户网站](https://sakura-editor.github.io/)

## 开发参与政策
开发政策总结如下。有意参与开发的请参考：  
https://github.com/sakura-editor/sakura/wiki

## 构建要求

编译 Sakura Editor 需要：
[最新版 Visual Studio][Visual Studio 最新版] 或 [旧版本 Visual Studio（2017 及更新版本，不含 Express 2017 for Windows Desktop）][Visual Studio 以前のバージョン]

Sakura Editor 基于 [OSI 认证的 zlib 许可证][ライセンスの OSI のページ][，符合开源协议](LICENSE)，[允许企业组织在特例情况下使用 Community 版进行开发和测试][Visual Studio Community ライセンス]。当然也支持 Professional/Enterprise 版。

正式版二进制文件使用 [Visual Studio Community 2017][Visual Studio 以前のバージョン] 构建。

### Visual Studio 安装必选组件
- Windows 10 SDK

### Visual Studio 2019 及后续版本支持情况

| Visual Studio 版本 | 已处理 Issue/PR                                           |
| ------------------ | -------------------------------------------------------- |
| 2022               | [#1872](https://github.com/sakura-editor/sakura/pull/1872)（[提交][2022対応コミット]） |
| 2017/2019 双版本   | [#866](https://github.com/sakura-editor/sakura/issues/866) |

[2022対応コミット]: https://github.com/sakura-editor/sakura/pull/1872/commits/93cf3f3eacfed6a4d0a2c30d5445b53b2599db3c

[构建机制说明](vcx-props/project-PlatformToolset.md)

### .vsconfig 配置说明

我们提供了 [.vsconfig](.vsconfig) 配置文件来帮助安装编译所需的 Visual Studio 组件。

#### 全版本通用配置

`vs_community__XXXXX.exe` でインストールする際に、--config オプションをつけてインストールする。
あるいは構成変更することにより、必要なコンポーネントを自動的にインストールします。

```
vs_community__XXXXX.exe --config <.vsconfig のファイルパス>
```

#### Visual Studio 2019 及更新版本
打开 sakura.sln 时会自动读取 [.vsconfig](.vsconfig) 文件，提示安装缺失组件。

#### 参考文档
- [如何提取已安装的 Visual Studio 组件 ID？][How to extract currently installed Visual Studio component IDs?]
- [使用 .vsconfig 统一配置 Visual Studio][Configure Visual Studio across your organization with .vsconfig]
- [导入/导出安装配置][インストール構成をインポートまたはエクスポートする]
- [Visual Studio 命令行安装参数][コマンド ライン パラメーターを使用して Visual Studio をインストールする]
- [自动安装缺失组件][不足しているコンポーネントを自動的にインストールする]

## 构建方法
1. 安装 [7-Zip](https://7-zip.opensource.jp/) 并配置环境变量
2. 使用 Visual Studio 打开 sakura.sln 进行构建

[详细构建说明](build.md)

## CI构建及本地构建环境变量
[环境变量说明](ci/build-envvars.md)

## PR本地快速获取方法
[获取 Pull Request 的快捷方式](get-PR.md)

## CI构建 (Azure Pipelines)
[构建机制详解](ci/azure-pipelines/azure-pipelines.md)

## CI构建 (AppVeyor)

### 构建机制
使用 [build-all.bat](build-all.bat) 进行构建，详见[批处理文件说明](ci/build-batchfiles.md)

### 构建成果物注意事项
使用前请仔细阅读：
- [通用警告](installer/warning.txt)
- [x64 版为 alpha 版本](installer/warning-alpha.txt)

### 构建成果物下载

#### master分支最新版
1. 访问 https://ci.appveyor.com/project/sakuraeditor/sakura/branch/master
2. 点击右侧 Jobs
3. 选择构建配置（如 Release/Win32）
4. 点击 ARTIFACTS 下载所需文件

#### master分支历史版本
访问构建历史记录下载：
https://ci.appveyor.com/project/sakuraeditor/sakura/history

## 开发信息

### 单元测试
[测试说明](tests/unittest.md)

### 调试方法
- [任务栏菜单调试指南](debug-tasktray-menu.md)
- [大文件生成方法](create-big-file.md)

## 更新日志
- [CHANGELOG.md](https://github.com/sakura-editor/sakura/blob/master/CHANGELOG.md)
- [最新日志下载](https://ci.appveyor.com/project/sakuraeditor/changelog-sakura/branch/master/artifacts)
- [本地查看 Markdown 的方法](https://github.com/sakura-editor/sakura/wiki/markdown-%E3%82%92%E3%83%AD%E3%83%BC%E3%82%AB%E3%83%AB%E3%81%A7%E7%A2%BA%E8%AA%8D%E3%81%99%E3%82%8B%E6%96%B9%E6%B3%95)

## 宏命令示例
[宏示例库](tools/macro) 欢迎提交 PR 贡献示例

## 静态代码分析
使用 CodeFactor 进行代码质量检查：
- [CPPLINT 规范](CPPLINT.md)

