# 阶段 1 第 13 步：平台抽象层

## 做了什么

新增 `macos/platform/include/`，Mac 版编译时放在 include 路径最前面：

| 文件 | 作用 |
|---|---|
| `StdAfx.h` | Mac 版预编译头。保留原版的标准库头文件，但不再包含 `Windows.h` 等 SDK 头文件，也不再包含 `env/DLLSHAREDATA.h` |
| `sakura_platform.h` | Win32 兼容定义：基本类型（BYTE/WORD/DWORD/BOOL…）、字符串指针类型、POINT/SIZE/RECT/SYSTEMTIME/FILETIME/LOGFONT 等结构体、不透明句柄、常用常量和宏（MAX_PATH、CP_*、MB_*、RGB、LOWORD…）、空的 SAL 注解，以及 MSVC 运行库扩展函数（`_wcsicmp`、`_snwprintf_s`、`_countof` 等）的标准实现 |

`macos/tools/check_compile.sh` 增加了开关：`SAKURA_PLATFORM=1` 时使用这一层。

## 效果（12 个目录、121 个 .cpp，错误按文件累计，包括头文件里的错误）

| 目录 | 文件数 | 之前错误数 | 现在错误数 | 现在零错误的文件 |
|---|---:|---:|---:|---:|
| basis | 15 | 14,624 | 99 | 8 |
| convert | 21 | 21,067 | 521 | 1 |
| mem | 5 | 4,895 | 15 | 3 |
| io | 7 | 11,737 | 692 | 1 |
| grep | 1 | 1,110 | 38 | 0 |
| parse | 2 | 2,041 | 38 | 0 |
| config | 1 | 961 | 0 | 1 |
| docplus | 4 | 6,494 | 431 | 0 |
| debug | 3 | 2,901 | 2 | 1 |
| charset | 19 | 21,817 | 773 | 0 |
| doc | 21 | 37,593 | 2,775 | 0 |
| types | 22 | 26,453 | 1,461 | 0 |
| **合计** | **121** | **151,693** | **6,845** | **15** |

找不到头文件导致的中断（fatal error）：0。

## 剩下的错误是什么

1. **原来靠 DLLSHAREDATA.h 顺带引入的类型**：最多的是 `EFunctionCode`（992 处），其次是 `CMyPoint`、`SelectionRange` 等。需要在各文件里补上真正需要的 `#include`（第 16 步处理）。
2. **GDI 绘图**：`FontHolder`、`MemDcHolder`、`SelectObject`、`DeleteObject` 等。这些属于界面层，会从核心库移出，由第 27 步的 Core Text 绘制替代。
3. **COM/OLE**：`BSTR`、`VARIANT`、`IDataObject` 等，来自宏和拖放代码，也不进核心库。
4. **编码 API**：`MultiByteToWideChar`、`IsValidCodePage` 等，第 17 步换成 ICU / iconv 实现。
5. **时间 API**：`FileTimeToSystemTime` 等，第 16 步补实现。

注意：`WCHAR` 暂时是 `wchar_t`（macOS 上 4 字节），第 14、15 步改为 UTF-16。
