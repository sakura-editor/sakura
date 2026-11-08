# Sakura Editor for Mac

本目录存放 Sakura Editor 的 macOS 原生版本相关的代码和文档。目标是在功能上与 Windows 版保持一致，并上架 Mac App Store。

## 分支策略

- `master`：与上游 [sakura-editor/sakura](https://github.com/sakura-editor/sakura) 保持同步，不在这里直接开发。
- `macos`：Mac 版的开发主分支。定期把 `master` 合并进来，以跟进上游更新。
- 跨平台改动（例如字符类型、CMake 构建）尽量写成可以回馈上游的形式，减少分叉。

## 目录结构（计划）

| 路径 | 内容 |
| --- | --- |
| `macos/docs/` | 计划、功能对照表等文档 |
| `macos/tools/` | 辅助脚本（例如生成功能对照表） |
| `macos/app/` | Xcode 工程定义（XcodeGen）和 Swift + AppKit 界面层 |
| `macos/bridge/` | Objective-C++ 桥接层（阶段 2） |
| `macos/platform/` | 平台抽象层：Win32 类型替代、字符类型（阶段 1） |

核心逻辑继续使用 `sakura_core/` 中的原版代码，不复制到本目录。

## 在 Mac 上构建

需要 Xcode 15 或更新版本，以及 [XcodeGen](https://github.com/yonaskolb/XcodeGen)。

```sh
brew install xcodegen
cd macos/app
xcodegen generate
open SakuraMac.xcodeproj
```

在 Xcode 里选好签名团队（Signing & Capabilities），按 Cmd+R 运行。
App 名称、Bundle ID、最低系统版本都集中在 `macos/app/Config/App.xcconfig`。

每次推送到 `macos` 分支，GitHub Actions（`.github/workflows/build-macos.yml`）都会在 macOS 上生成工程并做一次不签名的构建。

## 许可证

原版代码遵循 zlib 许可证（见仓库根目录的 [LICENSE](../LICENSE)）。本 Mac 版是基于 Sakura Editor 的修改版，不是官方版本。

## 文档

- [功能对照表](docs/feature-matrix.md)
