# 阶段 1 第 14 步：字符类型方案

## 结论

**WCHAR 在所有平台上都是 2 字节的 UTF-16 码元。** 原版的文本缓冲、编码转换、文件读写都以 UTF-16 为前提（比如直接把文件字节当成 `wchar_t` 数组处理、判断代理对），所以 Mac 版保持同样的内存表示，不改用 UTF-32。

| 项目 | Windows | macOS |
|---|---|---|
| `WCHAR` | `wchar_t`（Windows SDK，原样不变） | `char16_t`（`macos/platform/include/sakura_platform.h`） |
| 宽字符字面量 | `LTEXT("…")` / `LCHAR('x')` → `L"…"` | 同一个宏 → `u"…"` |
| 字符串类 | `sakura::wstring` / `sakura::wstring_view`（= `std::basic_string<WCHAR>`） | 同左 |
| `wcslen`、`_wcsicmp`、`_swprintf` 等 | MSVC 运行库 | `sakura_core/util/wchar16.h` 提供同名、同语义的 `char16_t` 重载 |

`LTEXT` / `LCHAR` 是原版 `util/std_macro.h` 里本来就有的宏，只是在非 Windows 平台上改为展开成 `u""`。Windows 上的展开结果和原来完全一样。

## 为什么这样做

- **调用字符串函数的代码不需要改写。** 原版大约调用了 1,700 次 `wcscpy`、`wcslen`、`wcscmp`、`_wcsicmp`、`_wtoi` 等函数。Mac 上有同名的 `char16_t` 版本，这些代码可以原样编译。
- **Windows 版行为不变。** Windows 上 WCHAR 仍是 `wchar_t`，宏的展开也不变。
- **没有采用 `-fshort-wchar`。** 它会让 `wchar_t` 和 macOS 系统库的 ABI 不一致，`std::wstring` 等标准库组件也会出错。

## 新增文件

| 文件 | 内容 |
|---|---|
| `sakura_core/util/wchar16.h` | `sakura::wstring`；`sakura::u16` 命名空间里的模板实现（字符串比较、查找、复制、切分，数值转换，大小写转换，UTF-8 / UTF-32 互转，以及与 MSVC 宽字符 printf 书式相同的 `format` / `vsnprintf_s`）；非 Windows 平台上的同名重载 |
| `macos/platform/tests/test_wchar16.cpp` | 示例与测试。macOS（clang，arm64 + x86_64）和 Windows（MSVC x64）都会编译运行；在 Windows 上还会把 `sakura::u16` 的结果和 MSVC 运行库逐项对比 |

printf 书式按 MSVC 宽字符版的规则处理：`%s` 是宽字符串，`%hs` / `%S` 是窄字符串（Mac 上按 UTF-8 处理），另外支持 `%I64d` 等 MSVC 专有写法。

## 第 15 步要做的替换

| 原写法 | 改为 | 数量（sakura_core） |
|---|---|---:|
| `L"…"` / `L'x'` | `LTEXT("…")` / `LCHAR('x')` | 约 6,150 / 1,880 |
| `wchar_t` | `WCHAR` | 约 1,580 |
| `std::wstring` / `std::wstring_view` | `sakura::wstring` / `sakura::wstring_view` | 约 920 / 370 |
| `std::format(L"…")`（35 处）、`std::wregex`（5 处）、`std::filesystem::path` 的宽字符串接口 | 逐个处理。标准库不支持 `char16_t` 的 format 和 regex，需要改用本工程自己的函数 | — |

另外发现一个需要在第 16 步处理的问题：`basis/primitive.h` 里 `uchar32_t` / `wchar32_t` 定义成 `long` 系类型，在 macOS 上是 8 字节，应改为 `char32_t` / `std::int32_t`。

## 对编译检查的影响

WCHAR 改成 `char16_t` 以后，原版代码里的 `L"…"`、`wchar_t` 和 WCHAR 不再是同一种类型，所以第 12 步编译检查的错误数暂时从 6,845 增加到 8,722。第 15 步批量替换后，这部分错误会消失。
