﻿/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
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

#ifdef _MSC_VER

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
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
// MinGW-w64-gcc にない関数をマクロ定義する
#define _wcstok wcstok
#endif

//グローバル
#include "_main/global.h"

//ビルドオプション的なヘッダ
#include "config/build_config.h"
#include "config/maxdata.h"

//定数(プリコンパイル日付に依存)
#include "config/system_constants.h"	//システム定数
#include "config/app_constants.h"		//アプリケーション定数

//高頻度API等
// #include <CommDlg.h> // WIN32_LEAN_AND_MEANでは必要。OpenFileDialg系
#include <CommCtrl.h> // コモンコントロール
#include <stdlib.h>  // _MAX_PATH
#include <wchar.h>

#ifndef SAKURA_PCH_MODE_MIN
// 2010.04.19 重そうなので追加
#include <HtmlHelp.h>
#include <ObjIdl.h>
#include <ShlObj.h>
#include <ShellAPI.h>
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <time.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#endif // ifndef SAKURA_PCH_MODE_MIN

//デバッグ
#include "debug/Debug1.h"
#include "debug/Debug2.h"
#include "debug/Debug3.h"

//シンプルでよく使うもの
#include "basis/primitive.h"
#include "util/std_macro.h"

//MFC互換
#include "basis/CMyString.h"
#include "basis/CMyRect.h"
#include "basis/CMyPoint.h"
#include "basis/CMySize.h"

//サクラエディタ固有型
#include "basis/SakuraBasis.h"

//よく使うヘッダ
#include "mem/CNativeW.h"
#include "mem/CNativeA.h"

#include "util/string_ex.h"
#include "util/MessageBoxF.h"
#include "CSelectLang.h"
#include "String_define.h"

//その他
#define malloc_char (char*)malloc
#define GlobalLockChar  (char*)::GlobalLock
#define GlobalLockUChar (unsigned char*)::GlobalLock
#define GlobalLockWChar (wchar_t*)::GlobalLock
#define GlobalLockBYTE  (BYTE*)::GlobalLock

//APIラップ
#include "apiwrap/StdControl.h"
#include "apiwrap/CommonControl.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。
