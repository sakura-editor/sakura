/*!	@file
	@brief Entry Point

	@author Norio Nakatani
	@date	1998/03/13 作成
	@date	2001/06/26 genta ワード単位のGrepのためのコマンドライン処理追加
	@date	2002/01/08 aroka 処理の流れを整理、未使用コードを削除
	@date	2002/01/18 aroka 虫取り＆リリース
	@date	2009/01/07 ryoji WinMainにOleInitialize/OleUninitializeを追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, aroka
	Copyright (C) 2007, kobake
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <Ole2.h>
#include <locale.h>
#include "CProcessFactory.h"
#include "CProcess.h"
#include "util/os.h"
#include "util/module.h"
#include "debug/CRunningTimer.h"
#include "version.h"

// アプリ名。2007.09.21 kobake 整理
#define _APP_NAME_(TYPE) TYPE("sakura")

#ifdef _DEBUG
#define _APP_NAME_2_(TYPE) TYPE("(デバッグ版)")
#else
#define _APP_NAME_2_(TYPE) TYPE("")
#endif

#ifdef ALPHA_VERSION
#define _APP_NAME_3_(TYPE) TYPE("(Alpha Version)")
#else
#define _APP_NAME_3_(TYPE) TYPE("")
#endif

#ifdef APPVEYOR_DEV_VERSION
#define _APP_NAME_DEV_(TYPE) TYPE("(dev Version)")
#else
#define _APP_NAME_DEV_(TYPE) TYPE("")
#endif

#define _GSTR_APPNAME_(TYPE)  _APP_NAME_(TYPE) _APP_NAME_2_(TYPE) _APP_NAME_DEV_(TYPE) _APP_NAME_3_(TYPE)

const WCHAR g_szGStrAppName[]  = (_GSTR_APPNAME_(_T)   ); // この変数を直接参照せずに GSTR_APPNAME を使うこと
const CHAR  g_szGStrAppNameA[] = (_GSTR_APPNAME_(ATEXT)); // この変数を直接参照せずに GSTR_APPNAME_A を使うこと
const WCHAR g_szGStrAppNameW[] = (_GSTR_APPNAME_(LTEXT)); // この変数を直接参照せずに GSTR_APPNAME_W を使うこと

//! メモリ枯渇テスト用しきい値
constexpr size_t memorySizeLimit = 2048;

/*!
 * @brief メモリ枯渇テスト用アロケーションフック関数
 *
 * @see https://docs.microsoft.com/en-us/visualstudio/debugger/allocation-hook-functions
 * @note 大きなメモリの確保を失敗させるフック関数
 */
int DenyHugeAllocHook( int allocType, void *userData, size_t size,
                   int blockType, long requestNumber,
                   const unsigned char *filename, int lineNumber)
{
	// Cランタイムが確保するメモリには干渉しない
	// see https://docs.microsoft.com/en-us/visualstudio/debugger/allocation-hooks-and-c-run-time-memory-allocations
	if (blockType == _CRT_BLOCK)
		return TRUE;

	// ちいさなメモリの確保は許容する
	if (size <= memorySizeLimit)
		return TRUE;

	return FALSE;
}

/*!
	Windows Entry point

	コマンドラインオプションで -NOWIN を指定するかどうかでプロセスのタイプが変わる。
		+----------+---------------------------+---------------------------+
		|-NOWIN指定|どうなる？                 |作成するProcessインスタンス|
		+----------+---------------------------+---------------------------+
		|有        |コントロールプロセスとなる |CControlProcessクラス      |
		+----------+---------------------------+---------------------------+
		|無        |エディタプロセスとなる     |CNormalProcessクラス       |
		+----------+---------------------------+---------------------------+
*/
int WINAPI wWinMain(
	HINSTANCE	hInstance,		//!< handle to current instance
	HINSTANCE	hPrevInstance,	//!< handle to previous instance
	LPWSTR		lpCmdLine,		//!< pointer to command line
	int			nCmdShow		//!< show state of window
)
{
#ifdef USE_LEAK_CHECK_WITH_CRTDBG
	// 2009.9.10 syat メモリリークチェックを追加
	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

	MY_RUNNINGTIMER(cRunningTimer, "WinMain" );
	{
		// 2014.04.24 DLLの検索パスからカレントディレクトリを削除する
		::SetDllDirectory( L"" );
		::SetSearchPathMode( BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT );

		setlocale( LC_ALL, "Japanese" ); //2007.08.16 kobake 追加
		::OleInitialize( NULL );	// 2009.01.07 ryoji 追加
	}
	
	//開発情報
	DEBUG_TRACE(L"-- -- WinMain -- --\n");
	DEBUG_TRACE(L"sizeof(DLLSHAREDATA) = %d\n",sizeof(DLLSHAREDATA));

	//メモリ枯渇テスト
	//1. アロケーションフックを仕掛ける
	::_CrtSetAllocHook(DenyHugeAllocHook);

	//2. CMemoryを構築する
	CMemory mem;
	assert(mem.GetRawPtr() == nullptr);
	assert(mem.GetRawLength() == 0);
	assert(mem.capacity() == 0);

	//3. 大きなメモリの確保を試みる
	mem.AllocBuffer(memorySizeLimit + 1);
	assert(mem.GetRawPtr() == nullptr);
	assert(mem.GetRawLength() == 0);
	//assert(mem.capacity() == 0);

	//4. 大きなメモリの設定を試みる
	char hugeBuf[memorySizeLimit + 1] = { 0 };
	mem.SetRawData(hugeBuf, sizeof(hugeBuf));
	assert(mem.GetRawPtr() == nullptr);
	assert(mem.GetRawLength() == 0);
	//assert(mem.capacity() == 0);

	//5. 小さなメモリの設定を試みる
	char smallBuf[] = "test";
	mem.SetRawData(smallBuf, sizeof(smallBuf));
	assert(mem.GetRawPtr());
	assert(mem.GetRawLength());
	assert(mem.capacity());
	assert(strcmp((char*)mem.GetRawPtr(), smallBuf) == 0);

	//6. 小さなメモリに大きなメモリをくっ付けてみる
	mem.SetRawData(smallBuf, sizeof(smallBuf));
	mem.AppendRawData(hugeBuf, sizeof(hugeBuf));
	assert(mem.GetRawPtr() == nullptr);
	assert(mem.GetRawLength() == 0);
	//assert(mem.capacity() == 0);

	//プロセスの生成とメッセージループ
	CProcessFactory aFactory;
	CProcess *process = 0;
	try{
		process = aFactory.Create( hInstance, lpCmdLine );
		MY_TRACETIME( cRunningTimer, "ProcessObject Created" );
	}
	catch(...){
	}
	if( 0 != process ){
		process->Run();
		delete process;
	}

	::OleUninitialize();	// 2009.01.07 ryoji 追加
	return 0;
}

