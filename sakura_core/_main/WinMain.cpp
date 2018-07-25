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

/*!
	Windows Entry point

	1つ目のエディタプロセスの場合は、このプロセスはコントロールプロセスと
	なり、新しいエディタプロセスを起動する。そうでないときはエディタプロセス
	となる。

	コントロールプロセスはCControlProcessクラスのインスタンスを作り、
	エディタプロセスはCNormalProcessクラスのインスタンスを作る。
*/
#ifdef __MINGW32__
int WINAPI WinMain(
	HINSTANCE	hInstance,		//!< handle to current instance
	HINSTANCE	hPrevInstance,	//!< handle to previous instance
	LPSTR		lpCmdLineA,		//!< pointer to command line
	int			nCmdShow		//!< show state of window
)
#else
int WINAPI _tWinMain(
	HINSTANCE	hInstance,		//!< handle to current instance
	HINSTANCE	hPrevInstance,	//!< handle to previous instance
	LPTSTR		lpCmdLine,		//!< pointer to command line
	int			nCmdShow		//!< show state of window
)
#endif
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
#ifdef __MINGW32__
		LPTSTR pszCommandLine;
		pszCommandLine = ::GetCommandLine();
		// 実行ファイル名をスキップする
		if( _T('\"') == *pszCommandLine ){
			pszCommandLine++;
			while( _T('\"') != *pszCommandLine && _T('\0') != *pszCommandLine ){
				pszCommandLine++;
			}
			if( _T('\"') == *pszCommandLine ){
				pszCommandLine++;
			}
		}else{
			while( _T(' ') != *pszCommandLine && _T('\t') != *pszCommandLine
				&& _T('\0') != *pszCommandLine ){
				pszCommandLine++;
			}
		}
		// 次のトークンまで進める
		while( _T(' ') == *pszCommandLine || _T('\t') == *pszCommandLine ){
			pszCommandLine++;
		}
		process = aFactory.Create( hInstance, pszCommandLine );
#else
		process = aFactory.Create( hInstance, lpCmdLine );
#endif
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


