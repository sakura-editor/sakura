/*!	@file
	@brief コントロールプロセスクラス

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka CProcessより分離, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "StdAfx.h"
#include "CControlProcess.h"
#include "CCommandLine.h"
#include "CShareData.h"
#include "Debug.h"
#include "CEditApp.h"
#include "etc_uty.h"
#include "CRunningTimer.h"
#include "sakura_rc.h"/// IDD_EXITTING 2002/2/10 aroka ヘッダ整理


//-------------------------------------------------


/*!
	@brief コントロールプロセスを初期化する
	
	MutexCPを作成・ロックする。
	CEditAppを作成する。
	
	@author aroka
	@date 2002/01/07
	@date 2002/02/17 YAZAKI 共有メモリを初期化するのはCProcessに移動。
	@date 2006/04/10 ryoji 初期化完了イベントの処理を追加、異常時の後始末はデストラクタに任せる
*/
bool CControlProcess::InitializeProcess()
{
	MY_RUNNINGTIMER( cRunningTimer, "CControlProcess::InitializeProcess" );

	// 旧バージョン（1.2.104.1以前）との互換性：「異なるバージョン...」が二回出ないように
	m_hMutex = ::CreateMutex( NULL, FALSE, GSTR_MUTEX_SAKURA_OLD );
	if( NULL == m_hMutex ){
		ErrorBeep();
		TopErrorMessage( NULL, _T("CreateMutex()失敗。\n終了します。") );
		return false;
	}

	// 初期化完了イベントを作成する
	m_hEventCPInitialized = ::CreateEvent( NULL, TRUE, FALSE, GSTR_EVENT_SAKURA_CP_INITIALIZED );
	if( NULL == m_hEventCPInitialized )
	{
		ErrorBeep();
		TopErrorMessage( NULL, _T("CreateEvent()失敗。\n終了します。") );
		return false;
	}

	/* コントロールプロセスの目印 */
	m_hMutexCP = ::CreateMutex( NULL, TRUE, GSTR_MUTEX_SAKURA_CP );
	if( NULL == m_hMutexCP ){
		ErrorBeep();
		TopErrorMessage( NULL, _T("CreateMutex()失敗。\n終了します。") );
		return false;
	}
	if( ERROR_ALREADY_EXISTS == ::GetLastError() ){
		return false;
	}
	
	/* 共有メモリを初期化 */
	if( !CProcess::InitializeProcess() ){
		return false;
	}

	/* 共有データのロード */
	// 2007.05.19 ryoji 「設定を保存して終了する」オプション処理（sakuext連携用）を追加
	TCHAR szIniFile[_MAX_PATH];
	m_cShareData.LoadShareData();
	m_cShareData.GetIniFileName( szIniFile );	// 出力iniファイル名
	if( !fexist(szIniFile) || CCommandLine::getInstance()->IsWriteQuit() ){
		/* レジストリ項目 作成 */
		m_cShareData.SaveShareData();
		if( CCommandLine::getInstance()->IsWriteQuit() ){
			return false;
		}
	}

	MY_TRACETIME( cRunningTimer, "Before new CEditApp" );

	/* タスクトレイにアイコン作成 */
	m_pcEditApp = new CEditApp;

	MY_TRACETIME( cRunningTimer, "After new CEditApp" );

	m_hWnd = m_pcEditApp->Create( m_hInstance );
	if( !m_hWnd ){
		ErrorBeep();
		TopErrorMessage( NULL, _T("ウィンドウの作成に失敗しました。\n起動できません。") );
		return false;
	}
	m_pShareData->m_sHandles.m_hwndTray = m_hWnd;

	// 初期化完了イベントをシグナル状態にする
	if( !::SetEvent( m_hEventCPInitialized ) ){
		ErrorBeep();
		TopErrorMessage( NULL, _T("SetEvent()失敗。\n終了します。") );
		return false;
	}

	return true;
}

/*!
	@brief コントロールプロセスのメッセージループ
	
	@author aroka
	@date 2002/01/07
*/
bool CControlProcess::MainLoop()
{
	if( m_pcEditApp && m_hWnd ){
		m_pcEditApp->MessageLoop();	/* メッセージループ */
		return true;
	}
	return false;
}

/*!
	@brief コントロールプロセスを終了する
	
	@author aroka
	@date 2002/01/07
	@date 2006/07/02 ryoji 共有データ保存を CEditApp へ移動
*/
void CControlProcess::OnExitProcess()
{
	m_pShareData->m_sHandles.m_hwndTray = NULL;
}

CControlProcess::~CControlProcess()
{
	delete m_pcEditApp;

	if( m_hEventCPInitialized ){
		::ResetEvent( m_hEventCPInitialized );
	}
	::CloseHandle( m_hEventCPInitialized );
	if( m_hMutexCP ){
		::ReleaseMutex( m_hMutexCP );
	}
	::CloseHandle( m_hMutexCP );
	// 旧バージョン（1.2.104.1以前）との互換性：「異なるバージョン...」が二回出ないように
	if( m_hMutex ){
		::ReleaseMutex( m_hMutex );
	}
	::CloseHandle( m_hMutex );
};

/*[EOF]*/
