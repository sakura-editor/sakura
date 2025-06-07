﻿/*!	@file
	@brief コントロールプロセスクラス

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka CProcessより分離, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CControlProcess.h"
#include "CControlTray.h"
#include "env/DLLSHAREDATA.h"
#include "CCommandLine.h"
#include "env/CShareData_IO.h"
#include "debug/CRunningTimer.h"
#include "env/CShareData.h"
#include "sakura_rc.h"/// IDD_EXITTING 2002/2/10 aroka ヘッダー整理
#include "config/system_constants.h"
#include "String_define.h"

//-------------------------------------------------

/*!
	@brief コントロールプロセスを初期化する
	
	MutexCPを作成・ロックする。
	CControlTrayを作成する。
	
	@author aroka
	@date 2002/01/07
	@date 2002/02/17 YAZAKI 共有メモリを初期化するのはCProcessに移動。
	@date 2006/04/10 ryoji 初期化完了イベントの処理を追加、異常時の後始末はデストラクタに任せる
	@date 2013.03.20 novice コントロールプロセスのカレントディレクトリをシステムディレクトリに変更
*/
bool CControlProcess::InitializeProcess()
{
	MY_RUNNINGTIMER( cRunningTimer, L"CControlProcess::InitializeProcess" );

	// アプリケーション実行検出用(インストーラで使用)
	m_hMutex = ::CreateMutex( NULL, FALSE, GSTR_MUTEX_SAKURA );
	if( NULL == m_hMutex ){
		ErrorBeep();
		TopErrorMessage( NULL, L"CreateMutex()失敗。\n終了します。" );
		return false;
	}

	const auto pszProfileName = CCommandLine::getInstance()->GetProfileName();

	// 初期化完了イベントを作成する
	std::wstring strInitEvent = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	strInitEvent += pszProfileName;
	m_hEventCPInitialized = ::CreateEvent( NULL, TRUE, FALSE, strInitEvent.c_str() );
	if( NULL == m_hEventCPInitialized )
	{
		ErrorBeep();
		TopErrorMessage( NULL, L"CreateEvent()失敗。\n終了します。" );
		return false;
	}

	/* コントロールプロセスの目印 */
	std::wstring strCtrlProcEvent = GSTR_MUTEX_SAKURA_CP;
	strCtrlProcEvent += pszProfileName;
	m_hMutexCP = ::CreateMutex( NULL, TRUE, strCtrlProcEvent.c_str() );
	if( NULL == m_hMutexCP ){
		ErrorBeep();
		TopErrorMessage( NULL, L"CreateMutex()失敗。\n終了します。" );
		return false;
	}
	if( ERROR_ALREADY_EXISTS == ::GetLastError() ){
		return false;
	}
	
	/* 共有メモリを初期化 */
	if( !CProcess::InitializeProcess() ){
		return false;
	}

	// コントロールプロセスのカレントディレクトリをシステムディレクトリに変更
	WCHAR szDir[_MAX_PATH];
	::GetSystemDirectory( szDir, _countof(szDir) );
	::SetCurrentDirectory( szDir );

	/* 共有データのロード */
	if( !CShareData_IO::LoadShareData() ){
		/* レジストリ項目 作成 */
		CShareData_IO::SaveShareData();
	}

	/* 言語を選択する */
	CSelectLang::ChangeLang( GetDllShareData().m_Common.m_sWindow.m_szLanguageDll );
	RefreshString();

	MY_TRACETIME( cRunningTimer, L"Before new CControlTray" );

	/* タスクトレイにアイコン作成 */
	m_pcTray = new CControlTray;

	MY_TRACETIME( cRunningTimer, L"After new CControlTray" );

	HWND hwnd = m_pcTray->Create( GetProcessInstance() );
	if( !hwnd ){
		ErrorBeep();
		TopErrorMessage( NULL, LS(STR_ERR_CTRLMTX3) );
		return false;
	}
	SetMainWindow(hwnd);
	GetDllShareData().m_sHandles.m_hwndTray = hwnd;

	// 初期化完了イベントをシグナル状態にする
	if( !::SetEvent( m_hEventCPInitialized ) ){
		ErrorBeep();
		TopErrorMessage( NULL, LS(STR_ERR_CTRLMTX4) );
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
	if( m_pcTray && GetMainWindow() ){
		m_pcTray->MessageLoop();	/* メッセージループ */
		return true;
	}
	return false;
}

/*!
	@brief コントロールプロセスを終了する
	
	@author aroka
	@date 2002/01/07
	@date 2006/07/02 ryoji 共有データ保存を CControlTray へ移動
*/
void CControlProcess::OnExitProcess()
{
	GetDllShareData().m_sHandles.m_hwndTray = NULL;
}

CControlProcess::~CControlProcess()
{
	delete m_pcTray;

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
