# Sakura Editor for Mac

本目录存放 Sakura Editor 的 macOS 原生版本相关的代码和文档。目标是在功能上与 Windows 版保持一致，并上架 Mac App Store。

## 分支策略

- `master`：与上游 [sakura-editor/sakura](https://github.com/sakura-editor/sakura) 保持同步，不在这里直接开发。
- `macos`：Mac 版的开发主分支。定期把 `master` 合并进来，以跟进上游更新。
- 跨平台改动（例如字符类型、CMake 构建）尽量写成可以回馈上游的形式，减少分叉。

## 目录结构

| 路径 | 内容 |
| --- | --- |
| `macos/docs/` | 各步骤的说明、功能对照表、与上游同步的流程 |
| `macos/tools/` | 辅助脚本（功能对照表生成、字符类型替换、编译检查、界面字符串生成）和代码页表导出工具（`codepage_dump/`） |
| `macos/app/` | Xcode 工程定义（XcodeGen）和 Swift + AppKit 界面层 |
| `macos/core/` | Mac 版核心库（`sakura_core` 的跨平台部分）的 CMake 构建和 Mac 专用测试（阶段 1） |
| `macos/platform/` | 平台抽象层：Win32 类型和 API 的替代、字符类型、代码页转换表（`data/`）（阶段 1） |
| `macos/bridge/` | Objective-C++ 桥接层（阶段 2，尚未建立） |

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

核心库和单元测试可以不经 Xcode 单独构建：

```sh
cmake -S macos/core -B build/mac-core -DCMAKE_BUILD_TYPE=Debug
cmake --build build/mac-core -j 4
ctest --test-dir build/mac-core --output-on-failure
```

每次推送到 `macos` 分支，GitHub Actions 都会自动运行：`build-macos.yml`（Mac 版 App 的不签名构建、核心库的单元测试、在 Windows 上导出代码页表），以及从上游继承的 Windows 构建（MSVC、MinGW）、Cppcheck 和 Doxygen，确保对共用代码的修改不影响 Windows 版。

## 许可证

原版代码遵循 zlib 许可证（见仓库根目录的 [LICENSE](../LICENSE)）。本 Mac 版是基于 Sakura Editor 的修改版，不是官方版本。

## 文档

- [功能对照表](docs/feature-matrix.md)
- [与上游同步的流程](docs/upstream-sync.md)
- 阶段 1：[编译错误清单](docs/phase1-compile-errors.md) · [平台抽象层](docs/phase1-platform-layer.md) · [字符类型方案](docs/phase1-char-type.md) · [核心库构建](docs/phase1-core-build.md) · [编码模块](docs/phase1-charset.md)
