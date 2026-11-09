# 阶段 1 第 16 步：编译纯逻辑模块

## 结果

- 新建 Mac 版核心库的构建 `macos/core/CMakeLists.txt`。macOS（arm64 + x86_64）和 Linux 都能构建。
- 核心库现在包含 `basis`、`config`、`debug`、`mem`、`parse` 五个目录，另外还有 `charset/charcode.cpp`、`util/format.cpp`、`util/string_ex.cpp` 和 `util/string_ex2.cpp`。COM 专用的 `basis/CErrorInfo.cpp` 不在 Mac 版里编译。
- 原版的单元测试（`src/test/cpp/tests1`）里，下列测试在 Mac 版上原样运行，**232 项全部通过**：
  `test-clayoutint` `test-cmemory` `test-cnative` `test-crunningtimer` `test-cwordparse` `test-editinfo` `test-format` `test-int2dec` `test-is_mailaddress` `test-statictype` `test-string_ex`
- 第 12 步那 12 个目录的编译检查：错误从 6,876 降到 4,238。没有错误的文件从 14 个增加到 72 个，其中 `convert`、`mem`、`parse`、`config`、`debug` 目录已经全部没有错误。

```
cmake -S macos/core -B build/mac-core -DCMAKE_BUILD_TYPE=Debug
cmake --build build/mac-core -j
ctest --test-dir build/mac-core --output-on-failure
```

## 新增文件

| 文件 | 内容 |
|---|---|
| `macos/core/CMakeLists.txt` | 核心库和测试的构建。Windows SDK 头文件的空替身在配置时生成；GoogleTest 1.17.0 下载时校验 SHA-256 |
| `macos/core/tests/tests.cmake` | 在 Mac 上运行的测试文件清单 |
| `macos/core/tests/pch.h` | 测试用的预编译头。让 `EXPECT_STREQ`、`StrEq` 等也能比较 `char16_t` 字符串 |
| `macos/platform/src/win32_api.cpp` | 少数 Win32 API 的替代实现：文件时间转换、`GetTickCount`、`MulDiv`、`Sleep`、`OutputDebugStringW`、`DebugBreak` 等 |
| `macos/platform/include/sakura_platform_hooks.h`、`macos/platform/src/ui_bridge.cpp` | 核心库调用界面功能的接口：测量文字宽度、显示消息框、读取界面字符串。App 启动时注册真正的实现（第 24、27、47 步）；没有注册时使用简单的默认实现 |

## 对原版代码的修改（Windows 版行为不变）

| 修改 | 原因 |
|---|---|
| `std::format(LTEXT(…))` → `sakura::format(LTEXT(…))`（39 处） | 标准库不支持 `char16_t` 的 format。Windows 上 `sakura::format` 直接转发给 `std::format` |
| `_T("…")` → `LTEXT("…")`（55 处） | `_T` 来自 Windows 的 `tchar.h` |
| `src/main/cpp` 也用第 15 步的工具做了替换 | 这里的代码同样编译进核心库 |
| `charset/charcode.{h,cpp}`：GDI 部分用 `#if defined(_WIN32)` 隔开，Mac 版通过测量接口计算文字宽度 | 字符宽度在排版中要用到 |
| `basis/CStrictInteger.h`：`LONG` 版运算符只在 Windows 上定义 | Mac 版的 `LONG` 就是 `int`，会重复定义 |
| `types/CType.h`：`CTypeConfig` 加上 `constexpr` | clang 要求 constexpr 构造函数的参数是字面量类型 |
| `basis/CMyString.h`：`path` → 字符串改用 `sakura::path_to_wstring` | Mac 上 `std::filesystem::path` 内部是 UTF-8 |
| 测试：错误码改为用 `EILSEQ` 生成预期消息；只适用于 Windows 的 `path` 测试用 `_WIN32` 隔开 | `EILSEQ` 在 Windows 上是 42，在 macOS 上是 92 |

## 留到后续步骤

- `charset`（第 17 步）、`doc` / `types` / `docplus`（第 19 步）、`env`（第 20 步）的剩余错误，以及 `io`（文件读写）。
- `test-cfileext` 依赖 `env` 里的 `CDocTypeManager`，到第 20 步再加入。
