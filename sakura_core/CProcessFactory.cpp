//	$Id$
/*!	@file
	@brief プロセス生成クラス

	@author aroka
	@date 2002/01/03 Create
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, masami shoji
	Copyright (C) 2002, aroka WinMainより分離

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CProcessFactory.h"
#include "CControlProcess.h"
#include "CNormalProcess.h"
#include "CCommandLine.h"
#include "CEditApp.h"
#include "Debug.h"
#include "etc_uty.h"
#include <tchar.h>
#include "COsVersionInfo.h"
#include "CRunningTimer.h"

class CProcess;


/*!
	@brief プロセスクラスを生成する
	
	コマンドライン、コントロールプロセスの有無を判定し、
	適当なプロセスクラスを生成する。
	
	@author aroka
	@date 2002/01/08
*/
CProcess* CProcessFactory::Create( HINSTANCE hInstance, LPSTR lpCmdLine )
{
	CCommandLine::Instance(lpCmdLine);
	
	CProcess* process = 0;
	if( !IsValidVersion() ){
		return 0;
	}
	if( IsStartingControlProcess() ){
		if( !IsExistControlProcess() ){
			process = new CControlProcess( hInstance, lpCmdLine );
		}
	}else{
		if( !IsExistControlProcess() ){
			StartControlProcess();
		}
		if( IsExistControlProcess() ){
			process = new CNormalProcess( hInstance, lpCmdLine );
		}
	}
	return process;
}


/*!
	@brief Windowsバージョンのチェック
	
	@author aroka
	@date 2002/01/03
*/
bool CProcessFactory::IsValidVersion()
{
	/* Windowsバージョンのチェック */
	COsVersionInfo	cOsVer;
	if( cOsVer.GetVersion() ){
		if( !cOsVer.OsIsEnableVersion() ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
				"このアプリケーションを実行するには、\nWindows95以上 または WindowsNT4.0以上のOSが必要です。\nアプリケーションを終了します。"
			);
			return false;
		}
	}else{
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
			"OSのバージョンが取得できません。\nアプリケーションを終了します。"
		);
		return false;
	}

	/* システムリソースのチェック */
	// Jul. 5, 2001 shoji masami NTではリソースチェックを行わない
	if( !cOsVer.IsWin32NT() ){
		if( !CheckSystemResources( GSTR_APPNAME ) ){
			return false;
		}
	}
	return true;
}


/*!
	@brief コマンドラインに -NOWIN があるかを判定する。
	
	@author aroka
	@date 2002/01/03 作成 2002/01/18 変更
*/
bool CProcessFactory::IsStartingControlProcess()
{
	return CCommandLine::Instance()->IsNoWindow();
}

/*!
	ミューテックスを取得し、コントロールプロセスの有無を調べる。
	
	@author aroka
	@date 2002/01/03
*/
bool CProcessFactory::IsExistControlProcess()
{
	HANDLE hMutexCP;
	hMutexCP = ::CreateMutex( NULL, FALSE, GSTR_MUTEX_SAKURA_CP );
	if( NULL == hMutexCP ){
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			_T("CreateMutex()失敗。\n終了します。"));
		::ExitProcess(1);
	}
	if( ERROR_ALREADY_EXISTS == ::GetLastError() ){
		DWORD dwRet = ::WaitForSingleObject( hMutexCP, 0 );
		if( WAIT_TIMEOUT == dwRet ){// あるけどロックされてる
			::CloseHandle( hMutexCP );
			return true;
		}
		// ロックされていなかった
		::ReleaseMutex( hMutexCP );
		::CloseHandle( hMutexCP );
		return false;
	}
	// なかった
	::CloseHandle( hMutexCP );
	return false;
}

//	From Here Aug. 28, 2001 genta
/*!
	@brief コントロールプロセスを起動する
	
	自分自身に -NOWIN オプションを付けて起動する．
	共有メモリをチェックしてはいけないので，残念ながらCEditApp::OpenNewEditorは使えない．
	
	@author genta
	@date Aug. 28, 2001
*/
bool CProcessFactory::StartControlProcess()
{
	MY_RUNNINGTIMER(cRunningTimer,"StartControlProcess" );

	//	プロセスの起動
	PROCESS_INFORMATION p;
	STARTUPINFO s;

	s.cb = sizeof( s );
	s.lpReserved = NULL;
	s.lpDesktop = NULL;
	s.lpTitle = NULL;

	s.dwFlags = STARTF_USESHOWWINDOW;
	s.wShowWindow = SW_SHOWDEFAULT;
	s.cbReserved2 = 0;
	s.lpReserved2 = NULL;

	TCHAR szCmdLineBuf[1024];	//	コマンドライン
	TCHAR szEXE[MAX_PATH + 1];	//	アプリケーションパス名
	TCHAR szDir[MAX_PATH + 1];	//	ディレクトリパス名

	::GetModuleFileName( ::GetModuleHandle( NULL ), szEXE, sizeof( szEXE ));
	::wsprintf( szCmdLineBuf, _T("%s -NOWIN"), szEXE );
	::GetSystemDirectory( szDir, sizeof( szDir ));

	if( 0 == ::CreateProcess( szEXE, szCmdLineBuf, NULL, NULL, FALSE,
		CREATE_DEFAULT_ERROR_MODE, NULL, szDir, &s, &p ) ){
		//	失敗
		LPVOID pMsg;
		::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						::GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR) &pMsg,
						0,
						NULL
		);
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			_T("\'%s\'\nプロセスの起動に失敗しました。\n%s"), szEXE, (LPTSTR)pMsg );
		::LocalFree( (HLOCAL)pMsg );	//	エラーメッセージバッファを解放
		return false;
	}

	//	起動したプロセスが完全に立ち上がるまでちょっと待つ．
	int nResult = ::WaitForInputIdle( p.hProcess, 10000 );	//	最大10秒間待つ
	if( 0 != nResult ){
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			_T("\'%s\'\nコントロールプロセスの起動に失敗しました。"), szEXE );
		::CloseHandle( p.hThread );
		::CloseHandle( p.hProcess );
		return false;
	}

	::CloseHandle( p.hThread );
	::CloseHandle( p.hProcess );
	
	return true;
}
//	To Here Aug. 28, 2001 genta


/*[EOF]*/
