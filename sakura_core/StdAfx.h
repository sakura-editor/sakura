/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

// stdafx.h : 標準のシステム インクルード ファイル、
//				または参照回数が多く、かつあまり変更されない
//				プロジェクト専用のインクルード ファイルを記述します。
//

// #pragma onceについての蘊蓄
//   Visual C++は、C++標準規格(1998年制定)より前から存在しており、
//   初期バージョンでは「#pragma once」をサポートしていなかった。
//   
//   _MSC_VER = 1000 : VC++ 4.0 (Windows 95時代)
//   _MSC_VER > 1000 : VC++ 5.0以降で#pragma onceをサポート
//   
//   サクラエディタは1998年にVC++ 6.0で開発が始まったため、
//   以下の条件分岐は本来不要だが、歴史的経緯として残している。
#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// この位置にヘッダーを挿入してください
// #define WIN32_LEAN_AND_MEAN		// Windows ヘッダーから殆ど使用されないスタッフを除外します

// STRICTシンボルについての蘊蓄
//   Windows SDKには、HWND(ウィンドウハンドル)やHDC(デバイスコンテキストハンドル)など、
//   多数のハンドル型が定義されている。これらはOSが管理するデータへのポインタだが、
//   内部構造は非公開のため、通常は void* 型で定義される。
//
//   しかし、すべてのハンドルを void* 型にすると、異なる種類のハンドルを
//   誤って混在させてもコンパイラが検出できない問題が発生する。
//   例: HWND が必要な関数に HDC を渡してもエラーにならない
//
//   この問題を解決するために導入されたのが STRICT モードである。
//   STRICTを定義すると、各ハンドル型が個別の型として認識され、
//   型の不一致をコンパイル時に検出できるようになる。
//
//   現在のWindows SDKでは、NO_STRICTを明示的に定義しない限り、
//   STRICTがデフォルトで有効になっているため、以下のコードは実質的に不要。
//   参考: https://learn.microsoft.com/ja-jp/windows/win32/winprog/disabling-strict
//
// #ifndef STRICT
// #define STRICT 1
// #endif

// NOMINMAXシンボルについての蘊蓄
//   Windows SDKは、C++標準規格(1998年制定)より前から存在するため、
//   標準ライブラリの std::min / std::max と同名のマクロ min/max を定義している。
//
//   これらのマクロは、テンプレート関数 std::min<T> / std::max<T> の
//   呼び出しを妨げ、コンパイルエラーを引き起こす問題がある。
//   例: std::min(a, b) がマクロ展開されてしまい、構文エラーになる
//
//   この問題を回避するには、Windows.h をインクルードする前に
//   NOMINMAX を定義してマクロを無効化する必要がある。
//
//   サクラエディタでは、以前はこのファイルで #define NOMINMAX していたが、
//   MinGW GCCでコンパイル時に大量の警告が発生する問題があったため、
//   現在はビルド設定(CMake/MSBuild)でグローバルに定義する方式に変更した。
//
// #define NOMINMAX

// _CRT_SECURE_NO_WARNINGSシンボルについての蘊蓄
//   Visual C++ 2005(VS2005)以降、Microsoftは標準Cランタイム関数の一部を
//   「セキュリティ上の問題がある」として非推奨にした。
//   例: strcpy → strcpy_s、sprintf → sprintf_s など
//
//   これらの非推奨関数を使用すると、コンパイル時に警告(C4996)が発生する。
//   _CRT_SECURE_NO_WARNINGS を定義すると、この警告を抑制できる。
//
//   サクラエディタでは、以前はこのシンボルを定義して警告を全面的に抑制していたが、
//   現在はビルド設定で警告レベルを調整する方式に変更した。(/w44996)
//
// #define _CRT_SECURE_NO_WARNINGS

// 標準C++ヘッダー（追加するときは昇順で。）
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <climits>
#include <clocale>
#include <cmath>
#include <concepts>
#include <condition_variable>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <cwctype>
#include <deque>
#include <exception>
#include <filesystem>
#include <format>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <numeric>
#include <optional>
#include <ranges>
#include <regex>
#include <set>
#include <source_location>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef _MSC_VER

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

// Workaround for PROPSHEETHEADER_V2_SIZE
#ifdef __MINGW32__
#include <_mingw.h>
#ifndef DUMMYUNION5_MEMBER
#ifndef NONAMELESSUNION
#define DUMMYUNION5_MEMBER(x) x
#else /* NONAMELESSUNION */
#define DUMMYUNION5_MEMBER(x) DUMMYUNIONNAME5.x
#endif
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

// Windowsヘッダー(他のSDKヘッダーとは別格。)
#include <Windows.h>

// windowsx.h (他のSDKヘッダーとは別格。)
#include <windowsx.h>

// その他のWindows SDK ヘッダー（ファイル名は最新に合わせる。追加するときは昇順で。）
#include <comdef.h>
#include <CommCtrl.h>
#include <HtmlHelp.h>
#include <imm.h>
#include <oaidl.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <Uxtheme.h>
#include <vsstyle.h>
#include <wrl.h>
#include <wrl/client.h>

/*!
 * NORETURNマクロ
 *
 * 関数の制御が戻らないことを示す。
 */
#if defined(_MSC_VER)
#  define NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
#  define NORETURN __attribute__((noreturn))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#  define NORETURN _Noreturn
#else
#  define NORETURN
#endif

#ifdef __MINGW32__
#ifdef UNREFERENCED_PARAMETER
#undef UNREFERENCED_PARAMETER
#endif
#define UNREFERENCED_PARAMETER(P) (void)(P)
#endif

// プロジェクト内のファイルだがプリコンパイル対象とする。
// プリコンパイルの有無がビルドパフォーマンスに大きく影響するため。
#include "env/DLLSHAREDATA.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。
