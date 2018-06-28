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
#include "CProcessFactory.h"
#include "CProcess.h"
#include "util/os.h"
#include "util/module.h"
#include "debug/CRunningTimer.h"


namespace _os {


/*!
 * @brief WinMainの引数「コマンドライン文字列」を取得する
 *
 *  入力: "\"C:\Program Files (x86)\\sakura\\sakura.exe\" -NOWIN"
 *  出力: "-NOWIN"
 *  ※ポインタ位置を進めているだけ。
 *
 * @param [in] lpCmdLine GetCommandLineW()の戻り値。
 * @retval lpCmdLine wWinMain形式のコマンドライン文字列。
 */
inline
_Ret_z_ LPWSTR SkipExeNameOfCommandLine(_In_z_ LPWSTR lpCmdLine) noexcept
{
	// 内部定数(空白文字)
	const WCHAR whiteSpace[] = L"\t\x20";

	// 文字列がダブルクォーテーションで始まっているかチェック
	if (L'\x22' == lpCmdLine[0]) {
		// 文字列ポインタを進める
		lpCmdLine++;
		// 閉じクォーテーションを探す(パス文字列なのでエスケープの考慮は不要)
		WCHAR *p = ::wcschr(lpCmdLine, L'\x22');
		if (p) {
			// 文字列ポインタを進める
			lpCmdLine = ++p;
		}
	}
	else {
		// 最初のトークンをスキップする
		// ※Windows 環境で実行する場合、この部分はデッドコードになる
		//   Wine等によるエミュレータ実行を考慮して実装だけはしておく
		size_t nPos = ::wcscspn(lpCmdLine, whiteSpace);
		lpCmdLine = &lpCmdLine[nPos];
	}

	// 次のトークンまで進める
	size_t nPos = ::wcsspn(lpCmdLine, whiteSpace);
	return &lpCmdLine[nPos];
}


}; // end of namespace _os


/*!
	Windows Entry point

	1つ目のエディタプロセスの場合は、このプロセスはコントロールプロセスと
	なり、新しいエディタプロセスを起動する。そうでないときはエディタプロセス
	となる。

	コントロールプロセスはCControlProcessクラスのインスタンスを作り、
	エディタプロセスはCNormalProcessクラスのインスタンスを作る。
*/
int WINAPI _tWinMain(
	_In_		HINSTANCE	hInstance,		//!< handle to current instance
	_In_opt_	HINSTANCE	hPrevInstance,	//!< handle to previous instance
	_In_		LPTSTR		lpCmdLine,		//!< pointer to command line
	_In_		int			nCmdShow		//!< show state of window
)
{
#ifdef USE_LEAK_CHECK_WITH_CRTDBG
	// 2009.9.10 syat メモリリークチェックを追加
	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

	MY_RUNNINGTIMER(cRunningTimer, "WinMain" );
	{
		// 2014.04.24 DLLの検索パスからカレントディレクトリを削除する
		HMODULE kernel32 = GetModuleHandleA( "KERNEL32" );
		if( kernel32 ){
			typedef BOOL (WINAPI* Proc_pfnSetDllDirectoryW)(LPCWSTR);
			Proc_pfnSetDllDirectoryW pfnSetDllDirectoryW = (Proc_pfnSetDllDirectoryW)GetProcAddress( kernel32, "SetDllDirectoryW" );
			if( pfnSetDllDirectoryW ){
				pfnSetDllDirectoryW( L"" );
			}
			typedef BOOL (WINAPI* Proc_pfnSetSearchPathMode)(DWORD);
			Proc_pfnSetSearchPathMode pfnSetSearchPathMode = (Proc_pfnSetSearchPathMode)GetProcAddress( kernel32, "SetSearchPathMode" );
			if( pfnSetSearchPathMode ){
				const DWORD dwBASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE = 1;
				const DWORD dwBASE_SEARCH_PATH_PERMANENT = 0x8000;
				pfnSetSearchPathMode( dwBASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | dwBASE_SEARCH_PATH_PERMANENT );
			}
		}
		// 2010.08.28 Moca DLLインジェクション対策
		CCurrentDirectoryBackupPoint dirBack;
		ChangeCurrentDirectoryToExeDir();

		setlocale( LC_ALL, "Japanese" ); //2007.08.16 kobake 追加
		::OleInitialize( NULL );	// 2009.01.07 ryoji 追加
	}
	
	//開発情報
	DEBUG_TRACE(_T("-- -- WinMain -- --\n"));
	DEBUG_TRACE(_T("sizeof(DLLSHAREDATA) = %d\n"),sizeof(DLLSHAREDATA));

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


#if defined( __MINGW32__ ) && defined( _UNICODE )
/*!
	Windows Entry point for MinGW Unicode Build.

	MinGW環境では wWinMain をエントリポイントとして起動できないため、
	自前でコマンドラインを取得して lpCmdLine を作成する
	wWinMainに入ったあとは普通の Windows アプリと同じ。
 */
int WINAPI WinMain(
	_In_		HINSTANCE	hInstance,		//!< handle to current instance
	_In_opt_	HINSTANCE	hPrevInstance,	//!< handle to previous instance
	_In_		LPSTR		lpCmdLineA,		//!< pointer to command line
	_In_		int			nCmdShow		//!< show state of window
)
{
	// コマンドラインを取得して実行ファイル名をスキップする
	LPTSTR pszCommandLine;
	pszCommandLine = ::GetCommandLine();
	pszCommandLine = _os::SkipExeNameOfCommandLine(pszCommandLine);
	return _tWinMain( hInstance, hPrevInstance, pszCommandLine, nCmdShow );
}
#endif /* defined( __MINGW32__ ) && defined( _UNICODE ) */
