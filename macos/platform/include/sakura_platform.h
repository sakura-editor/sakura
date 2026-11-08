/*! @file
	@brief 非 Windows 平台用的 Win32 兼容定义（平台抽象层，阶段 1 第 13 步）

	只提供核心代码（sakura_core 中的纯逻辑部分）编译所需的最小定义。
	- 基本类型、常量、结构体：与 Windows SDK 的定义保持相同的大小和含义。
	- 窗口、绘图等句柄：只声明为不透明指针，让头文件能通过编译。
	  核心代码不应真正使用它们，后续步骤会逐步把这类依赖移出核心库。
	- MSVC 运行库扩展函数：用标准 C/C++ 或 POSIX 函数实现。

	注意：WCHAR 暂时定义为 wchar_t（macOS 上是 4 字节）。
	第 14、15 步会改成基于 char16_t 的 UTF-16 类型。

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
typedef wchar_t        WCHAR;   // TODO(第 14 步)：改为 char16_t 系的 UTF-16 类型
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

inline int _wcsicmp(const wchar_t* a, const wchar_t* b) { return ::wcscasecmp(a, b); }
inline int _wcsnicmp(const wchar_t* a, const wchar_t* b, size_t n) { return ::wcsncasecmp(a, b, n); }
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
inline long _wtol(const wchar_t* s) { return std::wcstol(s, nullptr, 10); }
inline wchar_t* _wcslwr(wchar_t* s) { for (auto p = s; *p; ++p) *p = static_cast<wchar_t>(std::towlower(*p)); return s; }
inline unsigned short _byteswap_ushort(unsigned short v) { return __builtin_bswap16(v); }
inline unsigned long _byteswap_ulong(unsigned long v) { return __builtin_bswap32(static_cast<std::uint32_t>(v)); }
inline int __iscsym(int c) { return std::isalnum(c) || c == '_'; }

// 注意：Windows 的宽字符 printf 中 %s 表示宽字符串，POSIX 中表示窄字符串（宽字符串要用 %ls）。
// 下列函数只负责签名兼容，格式字符串的差异由第 14、15 步统一处理。
inline int _vsnwprintf_s(wchar_t* buf, size_t size, size_t count, const wchar_t* fmt, va_list ap) {
	if (!buf || size == 0) return -1;
	const size_t limit = (count == _TRUNCATE || count >= size) ? size : count + 1;
	const int n = std::vswprintf(buf, limit, fmt, ap);
	if (n < 0) { buf[limit - 1] = L'\0'; return (count == _TRUNCATE) ? -1 : -1; }
	return n;
}
inline int _snwprintf_s(wchar_t* buf, size_t size, size_t count, const wchar_t* fmt, ...) {
	va_list ap; va_start(ap, fmt); const int n = _vsnwprintf_s(buf, size, count, fmt, ap); va_end(ap); return n;
}
inline int _vsnprintf_s(char* buf, size_t size, size_t count, const char* fmt, va_list ap) {
	if (!buf || size == 0) return -1;
	const size_t limit = (count == _TRUNCATE || count >= size) ? size : count + 1;
	const int n = std::vsnprintf(buf, limit, fmt, ap);
	if (n < 0 || static_cast<size_t>(n) >= limit) { buf[limit - 1] = '\0'; return -1; }
	return n;
}
inline int _snprintf_s(char* buf, size_t size, size_t count, const char* fmt, ...) {
	va_list ap; va_start(ap, fmt); const int n = _vsnprintf_s(buf, size, count, fmt, ap); va_end(ap); return n;
}
inline int _scprintf(const char* fmt, ...) {
	va_list ap; va_start(ap, fmt); const int n = std::vsnprintf(nullptr, 0, fmt, ap); va_end(ap); return n;
}
inline int _scwprintf(const wchar_t* fmt, ...) {
	// vswprintf 不支持只计数，用逐步扩大的缓冲区求长度。
	for (size_t size = 256; size <= (size_t(1) << 24); size *= 2) {
		auto buf = static_cast<wchar_t*>(std::malloc(size * sizeof(wchar_t)));
		if (!buf) return -1;
		va_list ap; va_start(ap, fmt);
		const int n = std::vswprintf(buf, size, fmt, ap);
		va_end(ap);
		std::free(buf);
		if (n >= 0) return n;
	}
	return -1;
}
inline int _swprintf(wchar_t* buf, const wchar_t* fmt, ...) {
	va_list ap; va_start(ap, fmt); const int n = std::vswprintf(buf, static_cast<size_t>(-1) / sizeof(wchar_t), fmt, ap); va_end(ap); return n;
}

#endif /* SAKURA_PLATFORM_H_ */
