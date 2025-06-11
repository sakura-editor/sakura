/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

// stdafx.h : 標準のシステム インクルード ファイル、
//				または参照回数が多く、かつあまり変更されない
//				プロジェクト専用のインクルード ファイルを記述します。
//

#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// この位置にヘッダーを挿入してください
// #define WIN32_LEAN_AND_MEAN		// Windows ヘッダーから殆ど使用されないスタッフを除外します
#ifndef STRICT
#define STRICT 1
#endif

// Windows SDKのmin/maxマクロは使いません
#define NOMINMAX

// MS Cランタイムの非セキュア関数の使用を容認します
#define _CRT_SECURE_NO_WARNINGS

// 標準C++ヘッダー（追加するときは昇順で。）
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <climits>
#include <clocale>
#include <cmath>
#include <condition_variable>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cwchar>
#include <deque>
#include <exception>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <numeric>
#include <optional>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
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

#include <io.h>
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
#include <CommCtrl.h>
#include <HtmlHelp.h>
#include <imm.h>
#include <oaidl.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <Uxtheme.h>
#include <vsstyle.h>
#include <wrl.h>

// Windows SDKのマクロ定数「NULL」を訂正する。
// マクロ定数「NULL」は、省略可能なポインタ型パラメータに「省略」を指定するために使う。
// オリジナルでは「#define NULL 0」と定義されている。
// C++ではC++11からnullptrキーワードが導入されており、
// ポインタ型に0を渡すのは「不適切」になっている。
// 従来通りマクロ定数「NULL」を書けるようにするため、独自に上書き定義してしまう。
#ifdef __cplusplus
# pragma warning( push )
# pragma warning( disable : 4005 )
# define NULL nullptr
# pragma warning( pop )
#endif // end of #ifdef __cplusplus

// プロジェクト内のファイルだがプリコンパイル対象とする。
// プリコンパイルの有無がビルドパフォーマンスに大きく影響するため。
#include "env/DLLSHAREDATA.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。
