//	$Id$
/*!	@file
	@brief コントロールプロセスクラス

	@author aroka
	@date 2002/01/07 Create
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka CProcessより分離, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "CControlProcess.h"
#include "CCommandLine.h"
#include "CShareData.h"
#include "Debug.h"
#include "CEditApp.h"
#include "CMemory.h"
#include "etc_uty.h"
#include "sakura_rc.h"/// IDD_EXITTING 2002/2/10 aroka ヘッダ整理
#include <tchar.h>
#include "CRunningTimer.h"



/*!
	@brief コントロールプロセス終了ダイアログ用プロシージャ
*/
BOOL CALLBACK CControlProcess::ExitingDlgProc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam		// second message parameter
)
{
	switch( uMsg ){
	case WM_INITDIALOG:
		return TRUE;
	}
	return FALSE;
}
//-------------------------------------------------


/*!
	@brief コントロールプロセスを初期化する
	
	MutexCPを作成・ロックする。
	CEditAppを作成する。
	
	@author aroka
	@date 2002/01/07
	@date 2002/02/17 YAZAKI 共有メモリを初期化するのはCProcessに移動。
*/
bool CControlProcess::Initialize()
{
	MY_RUNNINGTIMER( cRunningTimer, "CControlProcess::Initialize" );

	// 旧バージョンとの互換性：「異なるバージョン...」が二回出ないように
	m_hMutex = ::CreateMutex( NULL, FALSE, GSTR_MUTEX_SAKURA );
	if( NULL == m_hMutex ){
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			_T("CreateMutex()失敗。\n終了します。") );
		return false;
	}
	/* コントロールプロセスの目印 */
	m_hMutexCP = ::CreateMutex( NULL, TRUE, GSTR_MUTEX_SAKURA_CP );
	if( NULL == m_hMutexCP ){
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			_T("CreateMutex()失敗。\n終了します。") );
		return false;
	}
	if( ERROR_ALREADY_EXISTS == ::GetLastError() ){
		DWORD dwRet = ::WaitForSingleObject( m_hMutexCP, 0 );
		if( WAIT_TIMEOUT == dwRet ){// すでにロックされてる
			::CloseHandle( m_hMutexCP );
			m_hMutexCP = NULL;
			return false;
		}
	}
	
	/* 共有メモリを初期化 */
	if ( CProcess::Initialize() == false ){
		::ReleaseMutex( m_hMutexCP );
		m_hMutexCP = NULL;
		return false;
	}

	/* 共有データのロード */
	if( FALSE == m_cShareData.LoadShareData() ){
		/* レジストリ項目 作成 */
		m_cShareData.SaveShareData();
	}

	MY_TRACETIME( cRunningTimer, "Before new CEditApp" );

	/* タスクトレイにアイコン作成 */
	m_pcEditApp = new CEditApp;

	MY_TRACETIME( cRunningTimer, "After new CEditApp" );

	if( NULL == ( m_hWnd = m_pcEditApp->Create( m_hInstance ) ) ){
		::ReleaseMutex( m_hMutexCP );
		m_hMutexCP = NULL;
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST,
			GSTR_APPNAME, _T("ウィンドウの作成に失敗しました。\n起動できません。") );
		return false;
	}
	m_pShareData->m_hwndTray = m_hWnd;

	return true;
}

/*!
	@brief コントロールプロセスのメッセージループ
	
	@author aroka
	@date 2002/01/07
*/
bool CControlProcess::MainLoop()
{
	if( NULL != m_pcEditApp && NULL != m_hWnd ){
		m_pcEditApp->MessageLoop();	/* メッセージループ */
		return true;
	}
	return false;
}

/*!
	@brief コントロールプロセスを終了する
	
	@author aroka
	@date 2002/01/07
*/
void CControlProcess::Terminate()
{
	/* 終了ダイアログを表示する */
	HWND hwndExitingDlg;
	if( TRUE == m_pShareData->m_Common.m_bDispExitingDialog ){
		/* 終了中ダイアログの表示 */
		hwndExitingDlg = ::CreateDialog(
			m_hInstance,
			MAKEINTRESOURCE( IDD_EXITING ),
			/*m_hWnd*/::GetDesktopWindow(),
			(DLGPROC)CControlProcess::ExitingDlgProc
		);
		::ShowWindow( hwndExitingDlg, SW_SHOW );
	}

	/* 共有データの保存 */
	m_cShareData.SaveShareData();

	/* 終了ダイアログを表示する */
	if( FALSE != m_pShareData->m_Common.m_bDispExitingDialog ){
		/* 終了中ダイアログの破棄 */
		::DestroyWindow( hwndExitingDlg );
	}

	m_pShareData->m_hwndTray = NULL;
	/* アクセラレータテーブルの削除 */
	if( m_pShareData->m_hAccel != NULL ){
		::DestroyAcceleratorTable( m_pShareData->m_hAccel );
		m_pShareData->m_hAccel = NULL;
	}

}

CControlProcess::~CControlProcess()
{
	if( m_pcEditApp ){
		delete m_pcEditApp;
	}
	if( m_hMutexCP ){
		::ReleaseMutex( m_hMutexCP );
	}
	::CloseHandle( m_hMutexCP );
	// 旧バージョンとの互換性：「異なるバージョン...」が二回出ないように
	if( m_hMutex ){
		::ReleaseMutex( m_hMutex );
	}
	::CloseHandle( m_hMutex );
};

/*[EOF]*/
