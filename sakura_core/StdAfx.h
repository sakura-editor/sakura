/*! @file */
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
#pragma comment(                                                                                                       \
	linker,                                                                                                            \
	"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(                                                                                                       \
	linker,                                                                                                            \
	"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(                                                                                                       \
	linker,                                                                                                            \
	"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(                                                                                                       \
	linker,                                                                                                            \
	"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
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
#include "config/app_constants.h"	 //アプリケーション定数
#include "config/system_constants.h" //システム定数

//高頻度API等
// #include <CommDlg.h> // WIN32_LEAN_AND_MEANでは必要。OpenFileDialg系
#include <CommCtrl.h> // コモンコントロール
#include <stdlib.h>	  // _MAX_PATH
#include <wchar.h>

#ifndef SAKURA_PCH_MODE_MIN
// 2010.04.19 重そうなので追加
#include <HtmlHelp.h>
#include <ObjIdl.h>
#include <ShellAPI.h>
#include <ShlObj.h>
#include <algorithm>
#include <io.h>
#include <map>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>
#include <vector>
#endif // ifndef SAKURA_PCH_MODE_MIN

//デバッグ
#include "debug/Debug1.h"
#include "debug/Debug2.h"
#include "debug/Debug3.h"

//シンプルでよく使うもの
#include "basis/primitive.h"
#include "util/std_macro.h"

// MFC互換
#include "basis/CMyPoint.h"
#include "basis/CMyRect.h"
#include "basis/CMySize.h"
#include "basis/CMyString.h"

//サクラエディタ固有型
#include "basis/SakuraBasis.h"

//よく使うヘッダ
#include "mem/CNativeA.h"
#include "mem/CNativeW.h"

#include "CSelectLang.h"
#include "util/MessageBoxF.h"
#include "util/string_ex.h"
#include "String_define.h"

//その他
#define malloc_char		(char *)malloc
#define GlobalLockChar	(char *)::GlobalLock
#define GlobalLockUChar (unsigned char *)::GlobalLock
#define GlobalLockWChar (wchar_t *)::GlobalLock
#define GlobalLockBYTE	(BYTE *)::GlobalLock

// APIラップ
#include "apiwrap/CommonControl.h"
#include "apiwrap/StdApi.h"
#include "apiwrap/StdControl.h"

// TCHARユーティリティ
#include "charset/charcode.h"
#include "charset/codechecker.h"
#include "util/tchar_convert.h"

// 2010.04.19 Moca includeの大規模整理
#ifndef SAKURA_PCH_MODE_MIN
#define SAKURA_PCH_MODE_DLLSHARE 1
#endif

#if defined(SAKURA_PCH_MODE_MAX) || defined(SAKURA_PCH_MODE_DLLSHARE)
#include "env/DLLSHAREDATA.h"
#endif

#ifdef SAKURA_PCH_MODE_MAX
#include "CEditApp.h"
#include "CReadManager.h"
#include "CSearchAgent.h"
#include "CWriteManager.h"
#include "_main/CAppMode.h"
#include "_main/CNormalProcess.h"
#include "doc/CDocReader.h"
#include "docplus/CBookmarkManager.h"
#include "docplus/CDiffManager.h"
#include "docplus/CModifyManager.h"
#include "env/CShareData.h"
#include "window/CEditWnd.h"
//###########超仮
#include "uiparts/CGraphics.h"
#endif // SAKURA_PCH_MODE_MAX

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。
