/*! @file
	@brief 非 Windows 平台用的 Win32 兼容定义（平台抽象层，阶段 1 第 13 步）

	只提供核心代码（sakura_core 中的纯逻辑部分）编译所需的最小定义。
	- 基本类型、常量、结构体：与 Windows SDK 的定义保持相同的大小和含义。
	- 窗口、绘图等句柄：只声明为不透明指针，让头文件能通过编译。
	  核心代码不应真正使用它们，后续步骤会逐步把这类依赖移出核心库。
	- MSVC 运行库扩展函数：用标准 C/C++ 或 POSIX 函数实现。

	WCHAR 定义为 char16_t（2 字节 UTF-16，与 Windows 相同）。
	宽字符串函数（wcslen、_wcsicmp、_swprintf 等）的 char16_t 版本在 sakura_core/util/wchar16.h。

	Copyright (C) 2026, Sakura Editor Organization
	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_PLATFORM_H_
#define SAKURA_PLATFORM_H_
#pragma once

#if defined(_WIN32)
#error "sakura_platform.h is for non-Windows platforms only."
#endif

#include <cctype>
#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <iterator>
#include <strings.h>

// ---------------------------------------------------------------------------
// SAL 注解：非 Windows 平台上没有意义，全部定义为空。
// ---------------------------------------------------------------------------
#define _In_
#define _In_z_
#define _In_opt_
#define _In_opt_z_
#define _In_reads_(n)
#define _In_reads_opt_(n)
#define _Inout_
#define _Inout_opt_
#define _Inout_z_
#define _Out_
#define _Out_opt_
#define _Out_writes_(n)
#define _Out_writes_z_(n)
#define _Outptr_
#define _Outptr_opt_
#define _Ret_z_
#define _Ret_maybenull_
#define _Check_return_
#define _Success_(expr)
#define _Printf_format_string_
#define _Analysis_assume_(expr)
#define _Use_decl_annotations_

// ---------------------------------------------------------------------------
// 调用约定等 MSVC 关键字
// ---------------------------------------------------------------------------
#define WINAPI
#define CALLBACK
#define APIENTRY
#define STDMETHODCALLTYPE
#define __stdcall
#define __cdecl
#define FAR
#define NEAR
#define CONST const

// ---------------------------------------------------------------------------
// 基本类型（大小与 Windows 相同）
// ---------------------------------------------------------------------------
typedef std::uint8_t   BYTE;
typedef std::uint16_t  WORD;
typedef std::uint32_t  DWORD;
typedef std::int32_t   BOOL;
typedef std::int32_t   INT;
typedef std::uint32_t  UINT;
typedef std::int32_t   LONG;
typedef std::uint32_t  ULONG;
typedef std::int64_t   LONGLONG;
typedef std::uint64_t  ULONGLONG;
typedef std::int32_t   INT32;
typedef std::uint32_t  UINT32;
typedef std::int64_t   INT64;
typedef std::uint64_t  UINT64;
typedef std::intptr_t  INT_PTR;
typedef std::uintptr_t UINT_PTR;
typedef std::intptr_t  LONG_PTR;
typedef std::uintptr_t ULONG_PTR;
typedef std::uintptr_t DWORD_PTR;
typedef std::size_t    SIZE_T;
typedef char           CHAR;
typedef char16_t       WCHAR;   // UTF-16 码元（第 14 步，见 sakura_core/util/wchar16.h）
typedef WCHAR          OLECHAR;
typedef void*          PVOID;
typedef void*          LPVOID;
typedef const void*    LPCVOID;
typedef BYTE*          LPBYTE;
typedef DWORD*         LPDWORD;
typedef CHAR*          LPSTR;
typedef const CHAR*    LPCSTR;
typedef WCHAR*         LPWSTR;
typedef const WCHAR*   LPCWSTR;
typedef UINT_PTR       WPARAM;
typedef LONG_PTR       LPARAM;
typedef LONG_PTR       LRESULT;
typedef DWORD          COLORREF;
typedef WORD           ATOM;
typedef DWORD          LCID;
typedef LONG           HRESULT;
typedef int            errno_t;

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// ---------------------------------------------------------------------------
// 句柄：不透明指针。核心代码只应传递，不应使用。
// ---------------------------------------------------------------------------
#define SAKURA_DECLARE_HANDLE(name) struct name##__; typedef struct name##__* name
typedef void* HANDLE;
SAKURA_DECLARE_HANDLE(HWND);
SAKURA_DECLARE_HANDLE(HINSTANCE);
SAKURA_DECLARE_HANDLE(HDC);
SAKURA_DECLARE_HANDLE(HFONT);
SAKURA_DECLARE_HANDLE(HBITMAP);
SAKURA_DECLARE_HANDLE(HBRUSH);
SAKURA_DECLARE_HANDLE(HPEN);
SAKURA_DECLARE_HANDLE(HRGN);
SAKURA_DECLARE_HANDLE(HICON);
SAKURA_DECLARE_HANDLE(HMENU);
SAKURA_DECLARE_HANDLE(HACCEL);
SAKURA_DECLARE_HANDLE(HKEY);
SAKURA_DECLARE_HANDLE(HMONITOR);
SAKURA_DECLARE_HANDLE(HIMC);
SAKURA_DECLARE_HANDLE(HDROP);
SAKURA_DECLARE_HANDLE(HIMAGELIST);
SAKURA_DECLARE_HANDLE(HTREEITEM);
typedef HICON   HCURSOR;
typedef HINSTANCE HMODULE;
typedef HANDLE  HGLOBAL;
typedef HANDLE  HGDIOBJ;
#define INVALID_HANDLE_VALUE (reinterpret_cast<HANDLE>(static_cast<LONG_PTR>(-1)))

// ---------------------------------------------------------------------------
// 结构体
// ---------------------------------------------------------------------------
typedef struct tagPOINT { LONG x; LONG y; } POINT, *PPOINT, *LPPOINT;
typedef struct tagSIZE  { LONG cx; LONG cy; } SIZE, *PSIZE, *LPSIZE;
typedef struct tagRECT  { LONG left; LONG top; LONG right; LONG bottom; } RECT, *PRECT, *LPRECT;
typedef const RECT* LPCRECT;
typedef struct tagPOINTS { short x; short y; } POINTS;
typedef struct _POINTL { LONG x; LONG y; } POINTL;
typedef struct _SYSTEMTIME {
	WORD wYear; WORD wMonth; WORD wDayOfWeek; WORD wDay;
	WORD wHour; WORD wMinute; WORD wSecond; WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
typedef struct _FILETIME { DWORD dwLowDateTime; DWORD dwHighDateTime; } FILETIME, *PFILETIME, *LPFILETIME;
typedef union _ULARGE_INTEGER {
	struct { DWORD LowPart; DWORD HighPart; } u;
	ULONGLONG QuadPart;
} ULARGE_INTEGER;
typedef struct _GUID {
	DWORD Data1; WORD Data2; WORD Data3; BYTE Data4[8];
} GUID, IID, CLSID;

#define LF_FACESIZE 32
#define DEFAULT_PITCH 0
#define FIXED_PITCH   1
#define VARIABLE_PITCH 2
typedef struct tagLOGFONTW {
	LONG lfHeight; LONG lfWidth; LONG lfEscapement; LONG lfOrientation; LONG lfWeight;
	BYTE lfItalic; BYTE lfUnderline; BYTE lfStrikeOut; BYTE lfCharSet;
	BYTE lfOutPrecision; BYTE lfClipPrecision; BYTE lfQuality; BYTE lfPitchAndFamily;
	WCHAR lfFaceName[LF_FACESIZE];
} LOGFONTW, LOGFONT, *LPLOGFONT;

// ---------------------------------------------------------------------------
// 常量
// ---------------------------------------------------------------------------
#define MAX_PATH  260
#define _MAX_PATH 260
#define _MAX_DIR  256
#define _MAX_FNAME 256
#define _MAX_EXT  256

#define CP_ACP   0
#define CP_UTF7  65000
#define CP_UTF8  65001

#define MB_OK                0x00000000L
#define MB_OKCANCEL          0x00000001L
#define MB_YESNOCANCEL       0x00000003L
#define MB_YESNO             0x00000004L
#define MB_ICONSTOP          0x00000010L
#define MB_ICONQUESTION      0x00000020L
#define MB_ICONEXCLAMATION   0x00000030L
#define MB_ICONINFORMATION   0x00000040L
#define MB_TOPMOST           0x00040000L
#define IDOK     1
#define IDCANCEL 2
#define IDYES    6
#define IDNO     7

#define CW_USEDEFAULT (static_cast<int>(0x80000000))
#define NO_ERROR 0L
#define S_OK     static_cast<HRESULT>(0L)
#define S_FALSE  static_cast<HRESULT>(1L)

#define RGB(r, g, b) (static_cast<COLORREF>( \
	static_cast<BYTE>(r) | (static_cast<WORD>(static_cast<BYTE>(g)) << 8) | (static_cast<DWORD>(static_cast<BYTE>(b)) << 16)))

#define MAKEWORD(a, b) (static_cast<WORD>((static_cast<BYTE>(a)) | (static_cast<WORD>(static_cast<BYTE>(b)) << 8)))
#define MAKELONG(a, b) (static_cast<LONG>((static_cast<WORD>(a)) | (static_cast<DWORD>(static_cast<WORD>(b)) << 16)))
#define LOWORD(l) (static_cast<WORD>(static_cast<DWORD_PTR>(l) & 0xffff))
#define HIWORD(l) (static_cast<WORD>((static_cast<DWORD_PTR>(l) >> 16) & 0xffff))
#define LOBYTE(w) (static_cast<BYTE>(static_cast<DWORD_PTR>(w) & 0xff))
#define HIBYTE(w) (static_cast<BYTE>((static_cast<DWORD_PTR>(w) >> 8) & 0xff))

// ---------------------------------------------------------------------------
// MSVC 运行库扩展
// ---------------------------------------------------------------------------
#define _TRUNCATE  (static_cast<size_t>(-1))
#define STRUNCATE  80
#define _countof(a) (std::size(a))
#define _HEAP_MAXREQ (static_cast<size_t>(-1) & ~static_cast<size_t>(0x1F))

inline int _stricmp(const char* a, const char* b) { return ::strcasecmp(a, b); }
inline int _strnicmp(const char* a, const char* b, size_t n) { return ::strncasecmp(a, b, n); }
inline int _memicmp(const void* a, const void* b, size_t n) {
	auto p = static_cast<const unsigned char*>(a);
	auto q = static_cast<const unsigned char*>(b);
	for (size_t i = 0; i < n; ++i) {
		const int d = std::tolower(p[i]) - std::tolower(q[i]);
		if (d != 0) return d;
	}
	return 0;
}
inline unsigned short _byteswap_ushort(unsigned short v) { return __builtin_bswap16(v); }
inline unsigned long _byteswap_ulong(unsigned long v) { return __builtin_bswap32(static_cast<std::uint32_t>(v)); }
inline int __iscsym(int c) { return std::isalnum(c) || c == '_'; }

// char 版の _snprintf_s / _scprintf は sakura_core/util/wchar16.h（%S で WCHAR 文字列を扱うため）

// ---------------------------------------------------------------------------
// Win32 API 的替代实现（macos/platform/src/win32_api.cpp）
// 只提供核心代码需要的少数函数，行为与 Windows 相同。
// ---------------------------------------------------------------------------
//! 代码页是否可用（第 17 步改为按 iconv 的实际支持情况判断）
BOOL IsValidCodePage(UINT codePage);
//! FILETIME（1601-01-01 起的 100 纳秒数，UTC）→ SYSTEMTIME（UTC）
BOOL FileTimeToSystemTime(const FILETIME* lpFileTime, LPSYSTEMTIME lpSystemTime);
//! SYSTEMTIME → FILETIME
BOOL SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime, LPFILETIME lpFileTime);
//! UTC 的 SYSTEMTIME → 本地时间。lpTimeZone 必须是 nullptr（使用当前时区）
BOOL SystemTimeToTzSpecificLocalTime(const void* lpTimeZone, const SYSTEMTIME* lpUniversalTime, LPSYSTEMTIME lpLocalTime);
//! 当前本地时间
void GetLocalTime(LPSYSTEMTIME lpSystemTime);
//! 当前 UTC 时间
void GetSystemTime(LPSYSTEMTIME lpSystemTime);
//! 系统启动后的毫秒数
DWORD GetTickCount();
ULONGLONG GetTickCount64();
//! 暂停指定毫秒
void Sleep(DWORD dwMilliseconds);
//! (a * b) / c，四舍五入。c 为 0 时返回 -1
int MulDiv(int nNumber, int nNumerator, int nDenominator);
//! 向调试输出写字符串（macOS 上写到 stderr 和系统日志）
void OutputDebugStringW(const WCHAR* lpOutputString);
void OutputDebugStringA(const char* lpOutputString);
//! 中断到调试器
void DebugBreak();
//! 释放动态库（dlclose）。语言资源 DLL 在 Mac 版不使用，这里只为让持有句柄的类能编译
BOOL FreeLibrary(HMODULE hLibModule);
//! 最近一次错误码
DWORD GetLastError();
void SetLastError(DWORD dwErrCode);

/* ---- 文件 ---- */
#define ERROR_FILE_NOT_FOUND      2L
#define ERROR_ACCESS_DENIED       5L
#define ERROR_INVALID_PARAMETER   87L
#define FILE_ATTRIBUTE_READONLY   0x00000001
#define FILE_ATTRIBUTE_HIDDEN     0x00000002
#define FILE_ATTRIBUTE_SYSTEM     0x00000004
#define FILE_ATTRIBUTE_DIRECTORY  0x00000010
#define FILE_ATTRIBUTE_ARCHIVE    0x00000020
#define FILE_ATTRIBUTE_NORMAL     0x00000080
#define INVALID_FILE_ATTRIBUTES   ((DWORD)-1)
//! 文件属性。macOS 上只返回只读、目录、隐藏（文件名以 . 开头）；文件不存在时返回 INVALID_FILE_ATTRIBUTES
DWORD GetFileAttributesW(const WCHAR* lpFileName);
//! 设置文件属性。macOS 上只处理只读属性（写权限），其余属性忽略
BOOL SetFileAttributesW(const WCHAR* lpFileName, DWORD dwFileAttributes);
#define GetFileAttributes GetFileAttributesW
#define SetFileAttributes SetFileAttributesW
//! 用 UTF-16 的路径和模式打开文件（路径转换为 UTF-8 后调用 fopen）
FILE* _wfopen(const WCHAR* filename, const WCHAR* mode);

#endif /* SAKURA_PLATFORM_H_ */
