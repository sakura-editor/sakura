# 阶段 1 第 17 步：移植编码模块 charset

## 结果

- `sakura_core/charset` 的全部 19 个文件编入 Mac 核心库，另外加入 `util/tchar_convert.cpp`、`io/CTextStream.cpp` 和 `window/CMainStatusBar.cpp` 中的 `UnicodeToHex`。
- 原版的编码相关测试在 Mac 上原样运行并全部通过：
  `test-ccodebase` `test-charset` `test-codechecker` `test-charcode` `test-ceol` `test-cconvert` `test-cdecode`
- Mac 版专用测试：`macos/core/tests/test-mac-codepage.cpp`（代码页转换，以及原版没有直接测试的 `CCodePage` 类）、`test-mac-charcode.cpp`（文字宽度缓存）、`test-mac-io.cpp` 增加了 `CTextStream` 的读写。
- 本地（Linux + clang）合计 595 项测试全部通过；macOS CI 同样运行。

## 代码页转换的做法

原版的编码转换大量依赖 Windows 的 `MultiByteToWideChar` / `WideCharToMultiByte`（Shift_JIS、EUC、Latin-1，以及用户可以选的任意代码页）。为了让结果与 Windows 完全一致，Mac 版不用 ICU 或 iconv 的表，而是用在 Windows 上实测得到的表：

1. `macos/tools/codepage_dump/dump_codepages.cpp` 在 Windows 上对每个单字节/双字节代码页和 GB18030 逐字符调用真正的 API，记录：
   - 每个字节、每个双字节组合的转换结果，以及加 `MB_ERR_INVALID_CHARS` 时是否成功
   - 每个 UTF-16 字符的转换结果（默认和 `WC_NO_BEST_FIT_CHARS` 两种），从而包含 best fit 和替代字符
   - 代码页名称、替代字符、前导字节范围、代理项的处理
2. `build-macos.yml` 的 `codepage-dump-windows` 任务每次都在 Windows 上重新导出，并与仓库里的表比较，不一致时给出警告。
3. 导出结果 `macos/platform/data/codepages.bin`（格式第 2 版，119 个代码页，约 9.3 MB）用 `.incbin` 嵌入核心库，由 `macos/platform/src/codepage.cpp` 读取。

其他代码页（GB18030 和 UTF-7 起初用 macOS 的 iconv，但 macOS 自带的 libiconv 版本较旧，GB18030 无法转换表情符号等补充平面字符、UTF-7 的 `-` 处理也与 Windows 不同，所以改为自己实现）：

| 代码页 | 实现 | 与 Windows 的差异 |
|---|---|---|
| 65001 UTF-8 | 代码实现 | 无。不正确序列的处理已在 Windows 上实测核对（导出工具的 `utf8-probe`） |
| 54936 GB18030 | 转换表 + 代码 | 无。1、2 字节和 BMP 的 4 字节字符来自 Windows 导出的表，补充平面按公式计算；不正确序列、参数检查按 `gb18030-probe` / `param-probe` 核对 |
| 65000 UTF-7 | 代码实现 | 无。直接输出的字符、base64 块的开始和结束、不正确输入、参数检查都按 `utf7-probe` / `param-probe` 核对 |
| 50220/50221/50222 ISO-2022-JP、50225、50227、50229、52936 HZ | macOS 的 iconv | 不正确输入、无法转换字符的处理可能与 Windows 不同。原版的 JIS（50220 系）读写用自己的 `CJis`，不受影响 |
| 57002〜57011 ISCII | 不支持 | iconv 也不支持，代码页列表里不出现 |

导出工具在 Windows 上确认的参数规则（Mac 版照此实现）：

| 代码页 | `lpDefaultChar` | `lpUsedDefaultChar` | `WC_NO_BEST_FIT_CHARS` | `WC_ERR_INVALID_CHARS` | `MB_ERR_INVALID_CHARS` |
|---|---|---|---|---|---|
| 65001 UTF-8 | 错误 87 | 接受但不使用 | 错误 1004 | 可用 | 可用 |
| 54936 GB18030 | 错误 87 | 错误 87 | 错误 1004 | 可用 | 可用 |
| 65000 UTF-7 | 错误 87 | 错误 87 | 错误 1004 | 错误 1004 | 错误 1004 |
| 932 等转换表代码页 | 可用 | 可用 | 可用 | 错误 1004 | 可用 |

`CP_ACP`、`CP_OEMCP` 都是 65001（UTF-8），与 Windows 的“Beta: 使用 Unicode UTF-8 提供全球语言支持”设置相同。这与 Mac 版窄字符串字面量是 UTF-8 一致（第 14 步）。原版中明确指定 Shift_JIS（`CP_SJIS`）的地方，例如标签文件的读取，仍然按 Shift_JIS 处理。

## 界面字符串

`CEol::GetName` 等通过 `LS(STR_…)` 读取界面字符串。Mac 版构建时用 `macos/tools/rc_strings.py` 从 `sakura_core/sakura_rc.rc` 的 STRINGTABLE 生成日文字符串表（1,356 条），作为默认的读取结果，与 Windows 版 sakura.exe 内置的字符串相同。英文和中文在第 48 步（界面本地化）加入。

## 新增文件

| 文件 | 内容 |
|---|---|
| `macos/tools/codepage_dump/dump_codepages.cpp` | 在 Windows 上导出代码页转换表的工具 |
| `macos/platform/data/codepages.bin`、`codepages.tsv` | 导出的转换表和代码页一览（Windows 11 24H2，10.0.26100） |
| `macos/platform/src/codepage.cpp` | `MultiByteToWideChar`、`WideCharToMultiByte`、`GetCPInfoEx`、`IsValidCodePage`、`EnumSystemCodePages` 等 |
| `macos/platform/src/codepage_data.cpp` | 用 `.incbin` 嵌入转换表 |
| `macos/tools/rc_strings.py` | 从 sakura_rc.rc 生成字符串表 |
| `macos/core/tests/test-mac-codepage.cpp`、`test-mac-charcode.cpp` | Mac 版专用测试 |

## 对原版代码的修改（Windows 版行为不变）

| 修改 | 原因 |
|---|---|
| `charset/CCodePage.cpp`：代码页下拉框 `AddComboCodePages` 只在 Windows 上编译 | 界面部分，Mac 版用 AppKit 从 `GetCodePageList()` 生成 |
| `charset/CESI.cpp`：调试用的 `GetDebugInfo` 只在 Windows 上编译 | 它读取编辑中的文档，第 19 步移植文档模型后再加入 |
| `charset/codeutil.cpp`：Mac 版用与 MSVC CRT 相同的计算实现 SJIS↔JIS 转换 | `_mbcjmstojis_l` 等是 MSVC 专有函数 |
| `util/tchar_convert.cpp`：`CP_ACP` 按界面语言换成 Shift_JIS 的处理只在 Windows 上做 | Mac 版的 `CP_ACP` 就是 UTF-8 |
| `io/CTextStream.cpp`：`CTextInputStream_AbsIni` 只在 Windows 上编译 | 依赖设置文件位置（util/file），第 20 步加入 |
| `window/CMainStatusBar.cpp`：Mac 版只编译 `UnicodeToHex` | 状态栏本身是 Windows 界面 |
| `charset/CESI.h`：加上 `class CNativeW;` 前置声明 | 调试用函数的声明用到它；Windows 版靠预编译头间接引入，Mac 版的预编译头不含它。这样头文件可以单独包含 |
| 测试 `test-cconvert`：`to_achar` / `to_wchar` 的预期值写成 Shift_JIS 字节；Shift_JIS 专有的检查在 Mac 上明确指定 `CP_SJIS` | Windows 版的窄字符串字面量是 Shift_JIS，Mac 版是 UTF-8 |
| 测试 `test-charcode`：用 GDI 测宽度的部分只在 Windows 上运行 | Mac 版由 `test-mac-charcode.cpp` 用假的测量函数确认相同的计算规则 |

## 留到后续步骤

- `test-cconvert` 里的 `ConvTest`（原版也只在 MSVC Debug 下运行）要打开编辑器窗口执行转换命令，随编辑器界面（阶段 2）加入。
- `test-file`：测试对象主要是 `util/file`（设置文件位置等），移到第 20 步。
- `CESI::GetDebugInfo`：第 19 步。
- 代码页名称目前是英文（导出时的 Windows 语言），界面本地化时（第 48 步）换成各语言的名称。
