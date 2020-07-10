﻿/*!	@file
	@brief プロセス生成クラス

	@author aroka
	@date 2002/01/03 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, masami shoji
	Copyright (C) 2002, aroka WinMainより分離
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CProcessFactory.h"
#include "CControlProcess.h"
#include "CNormalProcess.h"
#include "CCommandLine.h"
#include "CControlTray.h"
#include "dlg/CDlgProfileMgr.h"
#include "debug/CRunningTimer.h"
#include "util/os.h"
#include <io.h>
#include <tchar.h>

class CProcess;

/*!
	@brief プロセスクラスを生成する
	
	コマンドライン、コントロールプロセスの有無を判定し、
	適当なプロセスクラスを生成する。
	
	@param[in] hInstance インスタンスハンドル
	@param[in] lpCmdLine コマンドライン文字列
	
	@author aroka
	@date 2002/01/08
	@date 2006/04/10 ryoji
*/
CProcess* CProcessFactory::Create( HINSTANCE hInstance, LPCWSTR lpCmdLine )
{
	if( !ProfileSelect( hInstance, lpCmdLine ) ){
		return 0;
	}

	CProcess* process = 0;
	if( !IsValidVersion() ){
		return 0;
	}

	// プロセスクラスを生成する
	//
	// Note: 以下の処理において使用される IsExistControlProcess() は、コントロールプロセスが
	// 存在しない場合だけでなく、コントロールプロセスが起動して ::CreateMutex() を実行するまで
	// の間も false（コントロールプロセス無し）を返す。
	// 従って、複数のノーマルプロセスが同時に起動した場合などは複数のコントロールプロセスが
	// 起動されることもある。
	// しかし、そのような場合でもミューテックスを最初に確保したコントロールプロセスが唯一生き残る。
	//
	if( IsStartingControlProcess() ){
		if( TestWriteQuit() ){	// 2007.09.04 ryoji「設定を保存して終了する」オプション処理（sakuext連携用）
			return 0;
		}
		if( !IsExistControlProcess() ){
			process = new CControlProcess( hInstance, lpCmdLine );
		}
	}
	else{
		if( !IsExistControlProcess() ){
			StartControlProcess();
		}
		if( WaitForInitializedControlProcess() ){	// 2006.04.10 ryoji コントロールプロセスの初期化完了待ち
			process = new CNormalProcess( hInstance, lpCmdLine );
		}
	}
	return process;
}

bool CProcessFactory::ProfileSelect( HINSTANCE hInstance, LPCWSTR lpCmdLine )
{
	CDlgProfileMgr dlgProf;
	SProfileSettings settings;

	CDlgProfileMgr::ReadProfSettings( settings );
	CSelectLang::InitializeLanguageEnvironment();
	CSelectLang::ChangeLang( settings.m_szDllLanguage );

	//	May 30, 2000 genta
	//	実行ファイル名をもとに漢字コードを固定する．
	WCHAR szExeFileName[MAX_PATH];
	const int cchExeFileName = ::GetModuleFileName(NULL, szExeFileName, _countof(szExeFileName));
	CCommandLine::getInstance()->ParseKanjiCodeFromFileName(szExeFileName, cchExeFileName);

	CCommandLine::getInstance()->ParseCommandLine(lpCmdLine);

	bool bDialog;
	if( CCommandLine::getInstance()->IsProfileMgr() ){
		bDialog = true;
	}else if( CCommandLine::getInstance()->IsSetProfile() ){
		bDialog = false;
	}else if( settings.m_nDefaultIndex == -1 ){
		bDialog = true;
	}else{
		assert( 0 <= settings.m_nDefaultIndex );
		if( 0 < settings.m_nDefaultIndex ){
			CCommandLine::getInstance()->SetProfileName( settings.m_vProfList[settings.m_nDefaultIndex - 1].c_str() );
		}else{
			CCommandLine::getInstance()->SetProfileName( L"" );
		}
		bDialog = false;
	}
	if( bDialog ){
		if( dlgProf.DoModal( hInstance, NULL, 0 ) ){
			CCommandLine::getInstance()->SetProfileName( dlgProf.m_strProfileName.c_str() );
		}else{
			return false; // プロファイルマネージャで「閉じる」を選んだ。プロセス終了
		}
	}
	return true;
}

/*!
	@brief Windowsバージョンのチェック
	
	Windows 95以上，Windows NT4.0以上であることを確認する．
	Windows 95系では残りリソースのチェックも行う．
	
	@author aroka
	@date 2002/01/03
*/
bool CProcessFactory::IsValidVersion()
{
	// Windowsバージョンは廃止。
	// 動作可能バージョン(=windows7以降)でなければ起動できない。
	return true;
}

/*!
	@brief コマンドラインに -NOWIN があるかを判定する。
	
	@author aroka
	@date 2002/01/03 作成 2002/01/18 変更
*/
bool CProcessFactory::IsStartingControlProcess()
{
	return CCommandLine::getInstance()->IsNoWindow();
}

/*!
	コントロールプロセスの有無を調べる。
	
	@author aroka
	@date 2002/01/03
	@date 2006/04/10 ryoji
*/
bool CProcessFactory::IsExistControlProcess()
{
	const auto pszProfileName = CCommandLine::getInstance()->GetProfileName();
	std::wstring strMutexSakuraCp = GSTR_MUTEX_SAKURA_CP;
	strMutexSakuraCp += pszProfileName;
 	HANDLE hMutexCP;
	hMutexCP = ::OpenMutex( MUTEX_ALL_ACCESS, FALSE, strMutexSakuraCp.c_str() );	// 2006.04.10 ryoji ::CreateMutex() を ::OpenMutex()に変更
	if( NULL != hMutexCP ){
		::CloseHandle( hMutexCP );
		return true;	// コントロールプロセスが見つかった
	}

	return false;	// コントロールプロセスは存在していないか、まだ CreateMutex() してない
}

//	From Here Aug. 28, 2001 genta
/*!
	@brief コントロールプロセスを起動する
	
	自分自身に -NOWIN オプションを付けて起動する．
	共有メモリをチェックしてはいけないので，残念ながらCControlTray::OpenNewEditorは使えない．
	
	@author genta
	@date Aug. 28, 2001
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
bool CProcessFactory::StartControlProcess()
{
	MY_RUNNINGTIMER(cRunningTimer,"StartControlProcess" );

	//	プロセスの起動
	PROCESS_INFORMATION p;
	STARTUPINFO s;

	s.cb          = sizeof( s );
	s.lpReserved  = NULL;
	s.lpDesktop   = NULL;
	s.lpTitle     = const_cast<WCHAR*>(L"sakura control process"); //2007.09.21 kobake デバッグしやすいように、名前を付ける
	s.dwFlags     = STARTF_USESHOWWINDOW;
	s.wShowWindow = SW_SHOWDEFAULT;
	s.cbReserved2 = 0;
	s.lpReserved2 = NULL;

	WCHAR szCmdLineBuf[1024];	//	コマンドライン
	WCHAR szEXE[MAX_PATH + 1];	//	アプリケーションパス名

	::GetModuleFileName( NULL, szEXE, _countof( szEXE ));
	if( CCommandLine::getInstance()->IsSetProfile() ){
		::auto_sprintf( szCmdLineBuf, L"\"%s\" -NOWIN -PROF=\"%ls\"",
			szEXE, CCommandLine::getInstance()->GetProfileName() );
	}else{
		::auto_sprintf( szCmdLineBuf, L"\"%s\" -NOWIN", szEXE ); // ""付加
	}

	//常駐プロセス起動
	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;
#ifdef _DEBUG
//	dwCreationFlag |= DEBUG_PROCESS; //2007.09.22 kobake デバッグ用フラグ
#endif
	BOOL bCreateResult = ::CreateProcess(
		szEXE,				// 実行可能モジュールの名前
		szCmdLineBuf,		// コマンドラインの文字列
		NULL,				// セキュリティ記述子
		NULL,				// セキュリティ記述子
		FALSE,				// ハンドルの継承オプション
		dwCreationFlag,		// 作成のフラグ
		NULL,				// 新しい環境ブロック
		NULL,				// カレントディレクトリの名前
		&s,					// スタートアップ情報
		&p					// プロセス情報
	);
	if( !bCreateResult ){
		//	失敗
		WCHAR* pMsg;
		::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						::GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPWSTR)&pMsg,
						0,
						NULL
		);
		ErrorMessage( NULL, L"\'%s\'\nプロセスの起動に失敗しました。\n%s", szEXE, pMsg );
		::LocalFree( (HLOCAL)pMsg );	//	エラーメッセージバッファを解放
		return false;
	}

	// 起動したプロセスが完全に立ち上がるまでちょっと待つ．
	//
	// Note: この待ちにより、ここで起動したコントロールプロセスが競争に生き残れなかった場合でも、
	// 唯一生き残ったコントロールプロセスが多重起動防止用ミューテックスを作成しているはず。
	//
	int nResult;
	nResult = ::WaitForInputIdle( p.hProcess, 10000 );	//	最大10秒間待つ
	if( 0 != nResult ){
		ErrorMessage( NULL, L"\'%ls\'\nコントロールプロセスの起動に失敗しました。", szEXE );
		::CloseHandle( p.hThread );
		::CloseHandle( p.hProcess );
		return false;
	}

	::CloseHandle( p.hThread );
	::CloseHandle( p.hProcess );
	
	return true;
}
//	To Here Aug. 28, 2001 genta

/*!
	@brief コントロールプロセスの初期化完了イベントを待つ。

	@author ryoji by assitance with karoto
	@date 2006/04/10
*/
bool CProcessFactory::WaitForInitializedControlProcess()
{
	// 初期化完了イベントを待つ
	//
	// Note: コントロールプロセス側は多重起動防止用ミューテックスを ::CreateMutex() で
	// 作成するよりも先に初期化完了イベントを ::CreateEvent() で作成する。
	//
	if( !IsExistControlProcess() ){
		// コントロールプロセスが多重起動防止用のミューテックス作成前に異常終了した場合など
		return false;
	}

	const auto pszProfileName = CCommandLine::getInstance()->GetProfileName();
	std::wstring strInitEvent = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	strInitEvent += pszProfileName;
	HANDLE hEvent;
	hEvent = ::OpenEvent( EVENT_ALL_ACCESS, FALSE, strInitEvent.c_str() );
	if( NULL == hEvent ){
		// 動作中のコントロールプロセスを旧バージョンとみなし、イベントを待たずに処理を進める
		//
		// Note: Ver1.5.9.91以前のバージョンは初期化完了イベントを作らない。
		// このため、コントロールプロセスが常駐していないときに複数ウィンドウをほぼ
		// 同時に起動すると、競争に生き残れなかったコントロールプロセスの親プロセスや、
		// 僅かに出遅れてコントロールプロセスを作成しなかったプロセスでも、
		// コントロールプロセスの初期化処理を追い越してしまい、異常終了したり、
		// 「タブバーが表示されない」のような問題が発生していた。
		//
		return true;
	}
	DWORD dwRet;
	dwRet = ::WaitForSingleObject( hEvent, 10000 );	// 最大10秒間待つ
	if( WAIT_TIMEOUT == dwRet ){	// コントロールプロセスの初期化が終了しない
		::CloseHandle( hEvent );
		TopErrorMessage( NULL, L"エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。" );
		return false;
	}
	::CloseHandle( hEvent );
	return true;
}

/*!
	@brief 「設定を保存して終了する」オプション処理（sakuext連携用）

	@author ryoji
	@date 2007.09.04
*/
bool CProcessFactory::TestWriteQuit()
{
	if( CCommandLine::getInstance()->IsWriteQuit() ){
		WCHAR szIniFileIn[_MAX_PATH];
		WCHAR szIniFileOut[_MAX_PATH];
		CFileNameManager::getInstance()->GetIniFileNameDirect( szIniFileIn, szIniFileOut, L"" );
		if( szIniFileIn[0] != L'\0' ){	// マルチユーザ用設定か
			// 既にマルチユーザ用のiniファイルがあればEXE基準のiniファイルに上書き更新して終了
			if( fexist(szIniFileIn) ){
				if( ::CopyFile( szIniFileIn, szIniFileOut, FALSE ) ){
					return true;
				}
			}
		}else{
			// 既にEXE基準のiniファイルがあれば何もせず終了
			if( fexist(szIniFileOut) ){
				return true;
			}
		}
	}
	return false;
}
