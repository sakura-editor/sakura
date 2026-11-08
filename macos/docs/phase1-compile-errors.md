# 阶段 1 第 12 步：编译错误摸底报告

日期：2026-11-08　对象：`sakura_core` 中计划先移植的 12 个目录，共 121 个 `.cpp` 文件

## 结论

- **121 个文件全部无法直接编译**，但错误大多来自共用的头文件，而不是文件本身。
- 文件**自身**的错误只有 950 处；**54 个文件（45%）自身没有任何错误**，只要头文件问题解决就能编译通过。
- 所有错误归结为 **303 个缺失的标识符**（另有 94 个是连带产生的），主要是 Win32 的类型、常量和 API。这份清单就是第 13 步“平台抽象层”要补齐的内容。
- 最大的障碍是结构性的：**每个 `.cpp` 都包含 `StdAfx.h`，而它会引入 `Windows.h` 和 `env/DLLSHAREDATA.h`**。后者又会层层引入打印、设置、窗口、视图等界面相关的头文件，导致“纯逻辑”模块也间接依赖整个界面层。

## 方法

用 clang 18（Linux，`wchar_t` 和 macOS 一样是 4 字节）对每个文件做 `-fsyntax-only` 语法检查。Windows SDK 头文件用空文件代替，这样所有 Win32 依赖都会以“未声明的标识符”报出来，得到一份完整清单。脚本见 [`macos/tools/check_compile.sh`](../tools/check_compile.sh)，可以重复执行。

在 macOS 上的确认放到第 16 步，由 GitHub Actions 完成；预计结果相同，因为两者用的是同一个编译器前端，`wchar_t` 的大小也一样。

## 按目录统计

| 目录 | 文件数 | 文件自身的错误 | 自身无错误的文件 |
| --- | ---: | ---: | ---: |
| basis | 15 | 47 | 11 |
| convert | 21 | 27 | 15 |
| mem | 5 | 5 | 3 |
| io | 7 | 141 | 1 |
| grep | 1 | 25 | 0 |
| parse | 2 | 21 | 0 |
| config | 1 | 0 | 1 |
| docplus | 4 | 23 | 2 |
| debug | 3 | 10 | 0 |
| charset | 19 | 181 | 6 |
| doc | 21 | 366 | 6 |
| types | 22 | 104 | 9 |
| **合计** | **121** | **950** | **54** |

自身错误最多的文件：`doc/CEditDoc.cpp`（156）、`doc/CDocTypeSetting.cpp`（115）、`charset/CharsetDetector.cpp`（112，几乎全是 `WORD` 类型）、`io/CZipFile.cpp`（57，用 Windows Shell 的 COM 接口解压 ZIP）、`charset/charcode.cpp`（35）、`io/CFile.cpp`（33）。

## 错误类型归类

| 类型 | 标识符数 | 例子 | 处理方式 |
| --- | ---: | --- | --- |
| Win32 类型与常量 | 174 | `WCHAR` `DWORD` `BOOL` `HWND` `RECT` `POINT` `MAX_PATH` `MB_OK` `CP_UTF8` | 第 13 步：平台抽象层提供跨平台定义；与界面有关的（`HWND`、`HDC` 等）只在界面层使用，应从核心代码中移除 |
| Win32 API 函数 | 68 | `MultiByteToWideChar` `WideCharToMultiByte` `GetTickCount` `CreateFile` `GetFileAttributes` `SelectObject` `ExtTextOut` | 编码转换在第 17 步替换；文件操作改用 `std::filesystem`；绘制类调用要从核心代码中移出 |
| COM / OLE / WIC | 32 | `VARIANT` `BSTR` `IDataObject` `IShellDispatch` `IWICImagingFactory` | 宏接口（`_os/OleTypes.h`）、拖放、ZIP 解压、图片加载要换成 Mac 的实现，不属于核心库 |
| MSVC 运行库扩展 | 23 | `_snwprintf_s` `_wcsicmp` `_wcsnicmp` `_countof` `errno_t` `_TRUNCATE` | 第 13 步：用标准 C++ 函数改写，或提供同名的兼容实现 |
| SAL 注解 | 6 | `_In_` `_In_z_` `_Out_` `_In_opt_` | 第 13 步：在非 Windows 平台定义为空宏 |
| 连带错误 | 94 | `params` `format` `x` `y` 等 | 前面的错误导致的，不需要单独处理 |

## 引入错误最多的头文件

这些头文件被几乎所有文件包含，修好它们收益最大：

1. `util/MessageBoxF.h`：消息框（Win32 UI）
2. `util/string_ex.h`：字符串函数，大量使用 MSVC 扩展函数
3. `env/CommonSetting.h`：共通设置结构体，包含字体、窗口位置等 Win32 类型
4. `util/format.h`：格式化函数，依赖 `_snwprintf_s` 和 SAL 注解
5. `util/file.h`：文件路径函数
6. `env/CAppNodeManager.h`：多进程窗口管理（Mac 版改为单进程后不需要）
7. `basis/CMyPoint.h`、`basis/CMyRect.h`：基于 Win32 的 `POINT`、`RECT`
8. `print/CPrint.h`：打印设置（经 `DLLSHAREDATA.h` 引入）

另外，“纯逻辑”目录的头文件还会间接引入 `view/CEditView.h`、`window/CTabWnd.h`、`dlg/CDialog.h`、`prop/CPropCommon.h` 等界面头文件，需要切断这些依赖。

## 其他发现

- 有 2 个文件因为找不到头文件而中断：`apiwrap/StdControl.h` 写的是 `Commctrl.h`（大小写和实际文件不同，Windows 不区分大小写，其他平台区分），以及 `macro/CWSH.h` 需要的 `ActivScp.h`（Windows 脚本宿主，Mac 版不支持）。
- 子模块（`externals/` 下的 Onigmo、cmigemo 等）在这次检查中没有检出，用空文件代替了，第 19 步会正式处理。

## 对后续步骤的建议

1. **第 13 步要多做一件事**：为 Mac 版准备单独的预编译头，不包含 `Windows.h` 和 `DLLSHAREDATA.h`，每个文件只包含自己真正需要的头文件。这是让“纯逻辑”模块真正独立的前提。
2. 第 13 步按上面“引入错误最多的头文件”的顺序处理，前 5 个修好后，预计大部分文件的错误会大幅减少。
3. `io/CZipFile.cpp`（ZIP 解压）和 `charset` 中用到 WIC 的部分不放进核心库，改到 Mac 的界面层实现。
