//	$Id$
/************************************************************************
*
*	CEditApp.cpp
*
*	タスクトレイアイコンの管理
*	Copyright (C) 1998-2000, Norio Nakatani
*
*
*    CREATE: 1998/5/13
*
************************************************************************/

#define ID_HOTKEY_TRAYMENU 0x1234

#include <windows.h>
//#include <stdio.h>
#include <io.h>
//#include <ras.h>
#include "debug.h"
#include "CEditApp.h"
#include "sakura_rc.h"
#include "mymessage.h"
#include "CDlgOpenFile.h"
#include "global.h"
#include "etc_uty.h"
#include "CRunningTimer.h"
#include <htmlhelp.h>


/////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK CEditAppWndProc( HWND, UINT, WPARAM, LPARAM );


CEditApp*	g_m_pCEditApp;


//	BOOL CALLBACK ExitingDlgProc(
//	  HWND hwndDlg, // handle to dialog box
//	  UINT uMsg, // message
//	  WPARAM wParam, // first message parameter
//	  LPARAM lParam // second message parameter
//	)
//	{
//		switch( uMsg ){
//		case WM_INITDIALOG:
//			return TRUE;
//		}
//		return FALSE;
//	}


/* ウィンドウプロシージャじゃ */
LRESULT CALLBACK CEditAppWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	CEditApp* pSApp;
	switch( uMsg ){
	case WM_CREATE:
		pSApp = ( CEditApp* )g_m_pCEditApp;
		return pSApp->DispatchEvent( hwnd, uMsg, wParam, lParam );
	default:
		pSApp = ( CEditApp* )::GetWindowLong( hwnd, GWL_USERDATA );
		if( NULL != pSApp ){
			return pSApp->DispatchEvent( hwnd, uMsg, wParam, lParam );
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}




/////////////////////////////////////////////////////////////////////////////
// CEditApp

CEditApp::CEditApp()
{
	m_bCreatedTrayIcon = FALSE;	/* トレイにアイコンを作った  */
//	m_hAccel		= NULL;
	m_hInstance		= NULL;
	m_hWnd			= NULL;
//	m_nEditArrNum	= 0;
//	m_pEditArr		= NULL;
	/* 共有データ構造体のアドレスを返す */
	m_cShareData.Init();
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );
	if( m_pShareData->m_hAccel != NULL ){
		::DestroyAcceleratorTable( m_pShareData->m_hAccel );
		m_pShareData->m_hAccel = NULL;
	}
	m_pShareData->m_hAccel =
		CKeyBind::CreateAccerelator(
			m_pShareData->m_nKeyNameArrNum,
			m_pShareData->m_pKeyNameArr
		);
	if( NULL == m_pShareData->m_hAccel ){
		::MessageBox( NULL, "CEditApp::CEditApp()\nアクセラレータ テーブルが作成できません\nシステムリソースが不足しています。", GSTR_APPNAME, MB_OK | MB_ICONSTOP );
	}

//	#ifdef _DEBUG
//		m_pszAppName = "CEditApp[デバッグバージョン]";
//	#endif
//	#ifndef _DEBUG
		m_pszAppName = GSTR_CEDITAPP;
//	#endif
	return;
}


CEditApp::~CEditApp()
{
//	if( NULL != m_pEditArr ){
//		free( m_pEditArr );
//		m_pEditArr = NULL;
//		m_nEditArrNum = 0;
//	}
	return;
}

/////////////////////////////////////////////////////////////////////////////
// CEditApp メンバ関数




/* 作成 */
HWND CEditApp::Create( HINSTANCE hInstance )
{
	WNDCLASS	wc;
//	HANDLE		hMutex;
	HWND		hWnd;
//	MSG			msg;
	ATOM		atom;
//	char		szMutexName[260];
	HICON		hIcon;
//	HACCEL		hAccel;
//	int			nRet;
	HWND		hwndWork;

	m_hInstance = hInstance;
	hwndWork = ::FindWindow( m_pszAppName, m_pszAppName );
	if( NULL != hwndWork ){
		return NULL;
	}



//	wsprintf( szMutexName, "%sIsAlreadyExist", m_pszAppName );
//	hMutex = CreateMutex( NULL, TRUE, szMutexName );
//	if( ERROR_ALREADY_EXISTS != GetLastError() ){
		wc.style			=	CS_HREDRAW |
								CS_VREDRAW |
								CS_DBLCLKS |
								CS_BYTEALIGNCLIENT |
								CS_BYTEALIGNWINDOW;
		wc.lpfnWndProc		= CEditAppWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= m_hInstance;
		wc.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
		wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= m_pszAppName;
		atom = RegisterClass( &wc );
//	}else{
//		return NULL;
//	}
	g_m_pCEditApp = this;
	hWnd = ::CreateWindow(
		m_pszAppName,						// pointer to registered class name
		m_pszAppName,						// pointer to window name
		WS_OVERLAPPEDWINDOW/*WS_VISIBLE *//*| WS_CHILD *//* | WS_CLIPCHILDREN*/	,	// window style
		CW_USEDEFAULT,						// horizontal position of window
		0,									// vertical position of window
		100,								// window width
		100,								// window height
		NULL,								// handle to parent or owner window
		NULL,								// handle to menu or child-window identifier
		m_hInstance,							// handle to application instance
		NULL								// pointer to window-creation data
	);
//	m_hWnd = hWnd;
//	MYMESSAGEBOX( "m_hWnd=%08xH \n", m_hWnd );

	//	Oct. 16, 2000 genta
	m_hIcons.Create( m_hInstance, m_hWnd );
	m_CMenuDrawer.Create( m_hInstance, m_hWnd, &m_hIcons );

	if( NULL != m_hWnd ){
//		::SetWindowLong( m_hWnd, GWL_USERDATA, (LONG)this );
		/* タスクトレイのアイコンを作る */
		if( TRUE == m_pShareData->m_Common.m_bUseTaskTray ){	/* タスクトレイのアイコンを使う */
#ifdef _DEBUG
			hIcon =	::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_DEBUG ) );
#else
			hIcon =	::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_STD ) );
#endif
			TrayMessage( m_hWnd, NIM_ADD, 0,  hIcon, GSTR_APPNAME );
			m_bCreatedTrayIcon = TRUE;	/* トレイにアイコンを作った  */
		}
	}else{
	}
//	::ShowWindow( m_hWnd, SW_SHOW );

	/* Windows アクセラレータの作成 */
//	m_CKeyBind.Create( m_hInstance );
//	m_hAccel = m_CKeyBind.CreateAccerelator();
	return m_hWnd;
}




/* メッセージループ */
void CEditApp::MessageLoop( void )
{
//複数プロセス版	
	MSG	msg;
	while ( m_hWnd != NULL && ::GetMessage(&msg, NULL, 0, 0 ) ){
		::TranslateMessage( &msg );
		::DispatchMessage( &msg );
	}
	return;

	
//シングルプロセス版	
//	MSG			msg;
//	MSG			msg2;
//	CEditWnd*	pCEditWnd;
//	char		szClassName[64];
//	BOOL		bFromEditWnd;
//	while ( m_hWnd != NULL && ::GetMessage(&msg, NULL/*m_hWnd*/, 0, 0 ) ){
//		bFromEditWnd = FALSE;
//		if( 0 != ::GetClassName( msg.hwnd, szClassName, sizeof(szClassName) - 1 ) ){
//			if( 0 == strcmp( GSTR_EDITWINDOWNAME, szClassName ) ){
//				bFromEditWnd = TRUE;
//				pCEditWnd = ( CEditWnd* )::GetWindowLong( msg.hwnd, GWL_USERDATA );
//			}
//		}
//		if( bFromEditWnd ){
//			if( NULL != pCEditWnd->m_hwndPrintPreviewBar && ::IsDialogMessage( pCEditWnd->m_hwndPrintPreviewBar, &msg )  ){	/* 印刷プレビュー　操作バー */
//			}else{	
//				if( NULL != pCEditWnd->m_pShareData->m_hAccel ){
//					msg2 = msg;
//					if( ::TranslateAccelerator( msg.hwnd, pCEditWnd->m_pShareData->m_hAccel, &msg ) ){
//					}else{
//						::TranslateMessage( &msg );
//						::DispatchMessage( &msg );
//					}
//				}else{
//					::TranslateMessage( &msg );
//					::DispatchMessage( &msg );
//				}
//			}
//		}else{
//			::TranslateMessage( &msg );
//			::DispatchMessage( &msg );
//		}
//	}
	return;
}






/* タスクトレイのアイコンに関する処理 */
BOOL CEditApp::TrayMessage(HWND hDlg, DWORD dwMessage, UINT uID, HICON hIcon, const char* pszTip)
{
    BOOL			res;
	NOTIFYICONDATA	tnd;
	tnd.cbSize				= sizeof(NOTIFYICONDATA);
	tnd.hWnd				= hDlg;
	tnd.uID					= uID;
	tnd.uFlags				= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	tnd.uCallbackMessage	= MYWM_NOTIFYICON;
	tnd.hIcon				= hIcon;
	if( pszTip ){
		lstrcpyn(tnd.szTip, pszTip, sizeof(tnd.szTip));
	}else{
		tnd.szTip[0] = '\0';
	}
	res = Shell_NotifyIcon(dwMessage, &tnd);
	if (hIcon){
	    DestroyIcon(hIcon);
	}
	return res;
}





/* メッセージ処理 */
LRESULT CEditApp::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
//	HMENU			hMenuTop;
//	HMENU			hMenu;
//	POINT			po;
	int				nId;
//	int				i;
//	int				j;
//	HGLOBAL			hgData;
//	char*			pData;
//	char			szMemu[300];
	HWND			hwndWork;
	static CDlgGrep	cDlgGrep;
	char*			pCmdLine;
	char*			pOpt;
	CMemory			cmWork1;
	CMemory			cmWork2;
	CMemory			cmWork3;
	int				nDataLen;
	int				nRet;
	LPHELPINFO		lphi;
//	HWND			hwndExitingDlg;

//	CEditWnd*	pCEditWnd_Test;
//	int			i;
//	char*		pszCmdLine;
	int			nRowNum;
	EditNode*	pEditNodeArr;
//	HWND		hwndNew;
	static HWND	hwndHtmlHelp;
//	HWND		hwndHtmlHelpChild;



	int				idHotKey;
	UINT			fuModifiers;
	UINT			uVirtKey;
	static WORD		wHotKeyMods;
	static WORD		wHotKeyCode;
	HWND			hwndFocused;
	char			szClassName[100];
	char			szText[256];
	UINT				idCtl;	/* コントロールのID	*/
	MEASUREITEMSTRUCT*	lpmis;	
//	char				szLabel[1024];
//	LPMEASUREITEMSTRUCT	lpmis;	/* 項目サイズ情報	*/
//	char*				pszwork;
	LPDRAWITEMSTRUCT	lpdis;	/* 項目描画情報	*/
	int					nItemWidth;
	int					nItemHeight;

	switch ( uMsg ){
	case WM_MENUCHAR:
		/* メニューアクセスキー押下時の処理(WM_MENUCHAR処理) */
		return m_CMenuDrawer.OnMenuChar( hwnd, uMsg, wParam, lParam );
	case WM_DRAWITEM:
		idCtl = (UINT) wParam;	/* コントロールのID	*/
		lpdis = (DRAWITEMSTRUCT*) lParam;	/* 項目描画情報	*/
		switch( lpdis->CtlType ){
		case ODT_MENU:	/* オーナー描画メニュー */
			/* メニューアイテム描画 */
			m_CMenuDrawer.DrawItem( lpdis );
			return TRUE;
		}
		return FALSE;
	case WM_MEASUREITEM:
		idCtl = (UINT) wParam;                // control identifier 
		lpmis = (MEASUREITEMSTRUCT*) lParam; // item-size information 
		switch( lpmis->CtlType ){
		case ODT_MENU:	/* オーナー描画メニュー */
//			CMenuDrawer* pCMenuDrawer;
//			pCMenuDrawer = (CMenuDrawer*)lpmis->itemData;
			
			
//			MYTRACE( "WM_MEASUREITEM  lpmis->itemID=%d\n", lpmis->itemID );
			/* メニューアイテムの描画サイズを計算 */
			nItemWidth = m_CMenuDrawer.MeasureItem( lpmis->itemID, &nItemHeight );
			if( -1 == nItemWidth ){
			}else{
				lpmis->itemWidth = nItemWidth;
				lpmis->itemHeight = nItemHeight;
			}
			return TRUE;
		}
		return FALSE; 

	/* タスクトレイメニューへのショートカットキー登録 */
	case WM_HOTKEY:
		idHotKey = (int) wParam;              // identifier of hot key 
		fuModifiers = (UINT) LOWORD(lParam);  // key-modifier flags 
		uVirtKey = (UINT) HIWORD(lParam);     // virtual-key code 
		
		hwndFocused = ::GetForegroundWindow();
		szClassName[0] = '\0';
		::GetClassName( hwndFocused, szClassName, sizeof( szClassName ) - 1 );
		::GetWindowText( hwndFocused, szText, sizeof( szText ) - 1 );
		if( 0 == strcmp( szText, "共通設定" ) ){
			return -1;
		}

		if( ID_HOTKEY_TRAYMENU == idHotKey 
		 &&	( wHotKeyMods )  == fuModifiers  	
		 && wHotKeyCode == uVirtKey
		){
			::PostMessage( m_hWnd, MYWM_NOTIFYICON, 0, WM_LBUTTONDOWN );
		}
		return 0;
 
	case MYWM_HTMLHELP:
		{
			HH_AKLINK	link;
			char		szHtmlHelpFile[1024];
			int			nLen;
			int			nLenKey;
			char*		pszKey;
			HWND		hwndFrame;
			hwndFrame = (HWND)wParam;
			strcpy( szHtmlHelpFile, m_pShareData->m_szWork );
			nLen = lstrlen( szHtmlHelpFile );
			nLenKey = lstrlen( &m_pShareData->m_szWork[nLen + 1] );
			pszKey = new char[ nLenKey + 1 ];
			strcpy( pszKey, &m_pShareData->m_szWork[nLen + 1] );

//			if( NULL != hwndHtmlHelp ){
//				::BringWindowToTop( hwndHtmlHelp );
//			}

			hwndHtmlHelp = ::HtmlHelp(
				/*hwnd*/NULL/*hwndFrame*//*m_pShareData->m_hwndTray*/,
				szHtmlHelpFile,
				HH_DISPLAY_TOPIC,
				(DWORD)0
			);

			link.cbStruct =     sizeof(HH_AKLINK) ;
			link.fReserved =    FALSE ;
			link.pszKeywords =  (char*)pszKey; 
			link.pszUrl =       NULL ; 
			link.pszMsgText =   NULL ; 
			link.pszMsgTitle =  NULL ; 
			link.pszWindow =    NULL ;
			link.fIndexOnFail = TRUE ;

//			if( NULL != hwndHtmlHelp ){
//				hwndHtmlHelpChild = ::FindWindow( "#32770", "Topics Found"/*NULL*/ );
//				if( NULL != hwndHtmlHelpChild ){
//					HWND	hwndWork;
//					hwndWork = ::GetParent( hwndHtmlHelpChild );
//					if( hwndHtmlHelp == hwndWork ){
//						hwndHtmlHelp = hwndHtmlHelpChild;
//					}
//				}else{
//				}
//			}
//			if (hwndHtmlHelp != NULL){ 
//				DWORD dwPID; 
//				DWORD dwTID = ::GetWindowThreadProcessId( hwndHtmlHelp, &dwPID ); 
//				::AttachThreadInput( ::GetCurrentThreadId(), dwTID, TRUE); 
//				::SetFocus( hwndHtmlHelp ); 
//				::AttachThreadInput( ::GetCurrentThreadId(), dwTID, FALSE ); 
//			}

			hwndHtmlHelp = ::HtmlHelp(
				/*hwnd*/NULL/*hwndFrame*//*m_pShareData->m_hwndTray*/,
				szHtmlHelpFile,
				HH_KEYWORD_LOOKUP,
				(DWORD)&link
			);
			delete [] pszKey;
		}
		return (LRESULT)hwndHtmlHelp;;


//シングルプロセス版
//	/* 新しい編集ウィンドウの作成依頼(コマンドラインを渡す) */
//	case MYWM_OPENNEWEDITOR:
//		pszCmdLine = new char[lstrlen( m_pShareData->m_szWork ) + 1];
//		strcpy( pszCmdLine, m_pShareData->m_szWork );
//		hwndNew = OpenNewEditor3( m_hInstance, hwnd, pszCmdLine, FALSE );
//		delete [] pszCmdLine;
//		return (LONG)hwndNew;
		
//	/* 編集ウィンドウオブジェクトからのアクティブ要求 */
//	case MYWM_ACTIVATE_ME:
//		{
//			CRunningTimer	cRunningTimer;
//			cRunningTimer.Reset();
//			while( 3000 > cRunningTimer.Read() ){
//				::BlockingHook();
//			}
//			/* アクティブにする */
//			ActivateFrameWindow( (HWND)wParam );
//		}
//		return 0;

	/* 編集ウィンドウオブジェクトからのオブジェクト削除要求 */
	case MYWM_DELETE_ME:
//シングルプロセス版
//		pCEditWnd_Test = (CEditWnd*)lParam;
//		delete pCEditWnd_Test;

		/* タスクトレイのアイコンを常駐しない場合 */
		if( FALSE == m_pShareData->m_Common.m_bStayTaskTray	/* タスクトレイのアイコンを常駐 */
		 || FALSE == m_bCreatedTrayIcon						/* トレイにアイコンを作っていない  */
		 ){
			/* 現在開いている編集窓のリスト */
			nRowNum = m_cShareData.GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if( 0 < nRowNum ){
				delete [] pEditNodeArr;
			}
			/* 編集ウィンドウの数が0になったら終了 */
			if( 0 == nRowNum ){
				::SendMessage( hwnd, WM_CLOSE, 0, 0 );
			}
		}
		return 0;		
		
//	case WM_RASDIALEVENT:
//		{
//			RASCONNSTATE	rasconnstate;
//			DWORD			dwError;
//
//			rasconnstate = (RASCONNSTATE) wParam; // connection state about to be entered 
//			dwError = (DWORD) lParam;             // error that may have occurred 
//		}
//		break;
	case WM_CREATE:
		m_hWnd = hwnd;
		hwndHtmlHelp = NULL;
		::SetWindowLong( m_hWnd, GWL_USERDATA, (LONG)this );

//		for( i = 0; i < 16; ++i ){ 
//			HWND	hwndWork;
//			pCEditWnd_Test = NULL;
//			pCEditWnd_Test = new CEditWnd;
//			if( NULL == pCEditWnd_Test ){
//				MYTRACE( "オブジェクト確保失敗\n" );
//				break;
//			}
//			hwndWork = pCEditWnd_Test->Create( m_hInstance, hwnd/*pShareData->m_hwndTray*/, NULL, 0, FALSE );
//			if( NULL == hwndWork ){
//				MYTRACE( "NULL == hwndWork\n" );
//				break;
//			}
//		}

		/* タスクトレイメニューへのショートカットキー登録 */
		wHotKeyMods = 0;
		if( HOTKEYF_SHIFT & m_pShareData->m_Common.m_wTrayMenuHotKeyMods ){
			wHotKeyMods |= MOD_SHIFT;
		}
		if( HOTKEYF_CONTROL & m_pShareData->m_Common.m_wTrayMenuHotKeyMods ){
			wHotKeyMods |= MOD_CONTROL;
		}
		if( HOTKEYF_ALT & m_pShareData->m_Common.m_wTrayMenuHotKeyMods ){
			wHotKeyMods |= MOD_ALT;
		}
		wHotKeyCode = m_pShareData->m_Common.m_wTrayMenuHotKeyCode;
		::RegisterHotKey(
			m_hWnd,
			ID_HOTKEY_TRAYMENU,
			wHotKeyMods,
			wHotKeyCode
		);
		return 0L;

//	case WM_QUERYENDSESSION:
	case WM_HELP:
		lphi = (LPHELPINFO) lParam; 
		switch( lphi->iContextType ){
		case HELPINFO_MENUITEM:
			CEditWnd::OnHelp_MenuItem( hwnd, lphi->iCtrlId );
			break;
		}
		return TRUE;
//		case WM_LBUTTONDOWN:
//			/* ポップアップメニュー(トレイ左ボタン) */
//			nId = CreatePopUpMenu_L();
//			MYTRACE( "nId = %d\n", nId );
//			return 0L;
		case WM_COMMAND:
			OnCommand( HIWORD(wParam), LOWORD(wParam), (HWND) lParam );
			return 0L;

//		case MYWM_SETFILEINFO:
//			return 0L;		
		case MYWM_CHANGESETTING:
			::UnregisterHotKey( m_hWnd, ID_HOTKEY_TRAYMENU );
			/* タスクトレイメニューへのショートカットキー登録 */
			wHotKeyMods = 0;
			if( HOTKEYF_SHIFT & m_pShareData->m_Common.m_wTrayMenuHotKeyMods ){
				wHotKeyMods |= MOD_SHIFT;
			}
			if( HOTKEYF_CONTROL & m_pShareData->m_Common.m_wTrayMenuHotKeyMods ){
				wHotKeyMods |= MOD_CONTROL;
			}
			if( HOTKEYF_ALT & m_pShareData->m_Common.m_wTrayMenuHotKeyMods ){
				wHotKeyMods |= MOD_ALT;
			}
			wHotKeyCode = m_pShareData->m_Common.m_wTrayMenuHotKeyCode;
			::RegisterHotKey(
				m_hWnd,
				ID_HOTKEY_TRAYMENU,
				wHotKeyMods,
				wHotKeyCode
			);

//@@		/* 共有データの保存 */
//@@		m_cShareData.SaveShareData();

			/* アクセラレータテーブルの再作成 */
			if( m_pShareData->m_hAccel != NULL ){
				::DestroyAcceleratorTable( m_pShareData->m_hAccel );
				m_pShareData->m_hAccel = NULL;
			}
			m_pShareData->m_hAccel =
				CKeyBind::CreateAccerelator(
					m_pShareData->m_nKeyNameArrNum,
					m_pShareData->m_pKeyNameArr
				);
			if( NULL == m_pShareData->m_hAccel ){
				::MessageBox( NULL, "CEditApp::DispatchEvent()\nアクセラレータ テーブルが作成できません\nシステムリソースが不足しています。", GSTR_APPNAME, MB_OK | MB_ICONSTOP );
			}

			return 0L;

		case MYWM_NOTIFYICON:
//			MYTRACE( "MYWM_NOTIFYICON\n" );
			switch (lParam){
//キーワード：トレイ右クリックメニュー設定
//	From Here Oct. 12, 2000 JEPRO 左右とも同一処理になっていたのを別々に処理するように変更(全く未完のため殺した)
			case WM_RBUTTONDOWN:
//				::SetActiveWindow( m_hWnd );
//				::SetForegroundWindow( m_hWnd );
//				/* ポップアップメニュー(トレイ右ボタン) */
//				nId = CreatePopUpMenu_R();
//				switch( nId ){
//				case IDM_HELP_CONTENTS:
//					OnHelp( m_hWnd, IDM_HELP_CONTENTS );
//					break;
//				case IDM_EXITALL:
//					/* テキストエディタの終了 */
//					CEditApp::TerminateApplication();
//					break;
//				}
//				return 0L;
//	To Here Oct. 12, 2000

			case WM_LBUTTONDOWN:
//				MYTRACE( "WM_LBUTTONDOWN\n" );
				::SetActiveWindow( m_hWnd );
				::SetForegroundWindow( m_hWnd );
				/* ポップアップメニュー(トレイ左ボタン) */
//				MYTRACE( "CreatePopUpMenu_L START\n" );
				nId = CreatePopUpMenu_L();
//				MYTRACE( "CreatePopUpMenu_L END\n" );
				switch( nId ){
				case F_FILENEW:	/* 新規作成 */
//					MYTRACE( "F_FILENEW\n" );
					/* 新規編集ウィンドウの追加 */
					OpenNewEditor( m_hInstance, m_hWnd, (char*)NULL, 0, FALSE );
					break;
				case F_FILEOPEN:	/* 開く */
//					MYTRACE( "F_FILEOPEN\n" );
					{
						CDlgOpenFile	cDlgOpenFile;
//						char*			pszMRU = NULL;;
						char			szPath[_MAX_PATH + 1];
						int				nCharCode;
						BOOL			bReadOnly;
						HWND			hWndOwner;
						FileInfo*		pfi;
						int				i;
						int				j;
						char**			ppszMRU;
						char**			ppszOPENFOLDER;

						/* MRUリストのファイルのリスト */
						ppszMRU = NULL;
						j = 0;
						if( m_pShareData->m_nMRUArrNum > 0 ){
							for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
								if( m_pShareData->m_Common.m_nMRUArrNum_MAX <= i ){
									break;
								}
								++j;
							}
						}
						ppszMRU = new char*[j + 1];
						if( j > 0 ){
							for( i = 0; i < j; ++i ){
					//			ppszMRU[i] = m_pShareData->m_Types[m_nSettingType].m_szMRUArr[i];
								ppszMRU[i] = m_pShareData->m_fiMRUArr[i].m_szPath;
							}
						}
						ppszMRU[j] = NULL;

						/* OPENFOLDERリストのファイルのリスト */
						ppszOPENFOLDER = NULL;
						j = 0;
						if( m_pShareData->m_nOPENFOLDERArrNum > 0 ){
							for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum; ++i ){
								if( m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX <= i ){
									break;
								} 
								++j;
							}
						}
						ppszOPENFOLDER = new char*[j + 1];
						if( j > 0 ){
							for( i = 0; i < j; ++i ){
					//			ppszOPENFOLDER[i] = m_pShareData->m_Types[m_nSettingType].m_szOPENFOLDERArr[i];
								ppszOPENFOLDER[i] = m_pShareData->m_szOPENFOLDERArr[i];
							}
						}
						ppszOPENFOLDER[j] = NULL;



						/* ファイルオープンダイアログの初期化 */
						strcpy( szPath, "" );
						nCharCode = CODE_AUTODETECT;	/* 文字コード自動判別 */
						bReadOnly = FALSE;
						cDlgOpenFile.Create( 
							m_hInstance, 
							NULL/*m_hWnd*/, 
							"*.*", 
							m_pShareData->m_fiMRUArr[0].m_szPath, 
							(const char **)ppszMRU, 
							(const char **)ppszOPENFOLDER 
						);
						if( !cDlgOpenFile.DoModalOpenDlg( szPath, &nCharCode, &bReadOnly )){
							delete [] ppszMRU;
							delete [] ppszOPENFOLDER;
							break;
						}
						if( NULL == m_hWnd ){
							delete [] ppszMRU;
							delete [] ppszOPENFOLDER;
							break;
						}
						delete [] ppszMRU;
						delete [] ppszOPENFOLDER;
						/* 指定ファイルが開かれているか調べる */
						if( m_cShareData.IsPathOpened( szPath, &hWndOwner ) ){
							::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
//							pfi = (FileInfo*)m_pShareData->m_szWork;
							pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

							int nCharCodeNew;
							if( CODE_AUTODETECT == nCharCode ){	/* 文字コード自動判別 */
								/* 
								|| ファイルの日本語コードセット判別
								||
								|| 【戻り値】
								||	SJIS	0
								||	JIS		1    
								||	EUC		2    
								||	Unicode	3    
								||	エラー	-1
								*/
								nCharCodeNew = CMemory::CheckKanjiCodeOfFile( szPath );
								if( -1 == nCharCodeNew ){

								}else{
									nCharCode = nCharCodeNew;
								}
							}
							if( nCharCode != pfi->m_nCharCode ){	/* 文字コード種別 */
								char*	pszCodeNameCur;
								char*	pszCodeNameNew;
								switch( pfi->m_nCharCode ){
								case CODE_SJIS:		/* SJIS */		pszCodeNameCur = "SJIS";break;	//	Sept. 1, 2000 jepro 'シフト'を'S'に変更
								case CODE_JIS:		/* JIS */		pszCodeNameCur = "JIS";break;
								case CODE_EUC:		/* EUC */		pszCodeNameCur = "EUC";break;
								case CODE_UNICODE:	/* Unicode */	pszCodeNameCur = "Unicode";break;
								case CODE_UTF8:	/* UTF-8 */			pszCodeNameCur = "UTF-8";break;
								case CODE_UTF7:	/* UTF-7 */			pszCodeNameCur = "UTF-7";break;
								} 
								switch( nCharCode ){
								case CODE_SJIS:		/* SJIS */		pszCodeNameNew = "SJIS";break;	//	Sept. 1, 2000 jepro 'シフト'を'S'に変更
								case CODE_JIS:		/* JIS */		pszCodeNameNew = "JIS";break;
								case CODE_EUC:		/* EUC */		pszCodeNameNew = "EUC";break;
								case CODE_UNICODE:	/* Unicode */	pszCodeNameNew = "Unicode";break;
								case CODE_UTF8:	/* UTF-8 */			pszCodeNameNew = "UTF-8";break;
								case CODE_UTF7:	/* UTF-7 */			pszCodeNameNew = "UTF-7";break;
								} 
								::MYMESSAGEBOX(	m_hWnd,	MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
									"%s\n\n\n既に開いているファイルを違う文字コードで開く場合は、\n一旦閉じてから開いてください。\n\n現在の文字コードセット=[%s]\n新しい文字コードセット=[%s]", 
									szPath, pszCodeNameCur, pszCodeNameNew
								);
							}
							/* 開いているウィンドウをアクティブにする */
							/* アクティブにする */
							ActivateFrameWindow( hWndOwner );
						}else{
							if( strchr( szPath, ' ' ) ){
								char	szFile2[_MAX_PATH + 3];
								wsprintf( szFile2, "\"%s\"", szPath );
								strcpy( szPath, szFile2 );
							}
							/* 新たな編集ウィンドウを起動 */
							CEditApp::OpenNewEditor( m_hInstance, m_hWnd, szPath, nCharCode, bReadOnly );
						}
					}
					break;
				case F_GREP:
					/* Grep */
					/* Grepダイアログの初期化１ */
//					cDlgGrep.Create( m_hInstance, /*m_hWnd*/NULL );

					strcpy( cDlgGrep.m_szText, m_pShareData->m_szSEARCHKEYArr[0] );

					/* Grepダイアログの表示 */
					nRet = cDlgGrep.DoModal( m_hInstance, /*m_hWnd*/NULL, "" );
//					MYTRACE( "nRet=%d\n", nRet );
					if( FALSE == nRet || m_hWnd == NULL ){
						break;;
					}

//					MYTRACE( "cDlgGrep.m_szText  =[%s]\n", cDlgGrep.m_szText   );
//					MYTRACE( "cDlgGrep.m_szFile  =[%s]\n", cDlgGrep.m_szFile   );
//					MYTRACE( "cDlgGrep.m_szFolder=[%s]\n", cDlgGrep.m_szFolder );

					/*======= Grepの実行 =============*/
					/* Grep結果ウィンドウの表示 */

					pCmdLine = new char[1024];
					pOpt = new char[64];

					cmWork1.SetDataSz( cDlgGrep.m_szText ); 
					cmWork2.SetDataSz( cDlgGrep.m_szFile ); 
					cmWork3.SetDataSz( cDlgGrep.m_szFolder ); 
					cmWork1.Replace( "\"", "\"\"" );
					cmWork2.Replace( "\"", "\"\"" );
					cmWork3.Replace( "\"", "\"\"" );

					/*
					|| -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GOPT=S
					*/
					wsprintf( pCmdLine, "-GREPMODE -GKEY=\"%s\" -GFILE=\"%s\" -GFOLDER=\"%s\"" ,  
						cmWork1.GetPtr( &nDataLen ),
						cmWork2.GetPtr( &nDataLen ),
						cmWork3.GetPtr( &nDataLen )
					);
					
					pOpt[0] = '\0';
					if( cDlgGrep.m_bSubFolder ){	/* サブフォルダからも検索する */
						strcat( pOpt, "S" ); 
					}
				//	if( m_bFromThisText ){	/* この編集中のテキストから検索する */
				//		
				//	}
					if( cDlgGrep.m_bLoHiCase ){	/* 英大文字と英小文字を区別する */
						strcat( pOpt, "L" ); 
					}
					if( cDlgGrep.m_bRegularExp ){	/* 正規表現 */
						strcat( pOpt, "R" ); 
					}
					if( cDlgGrep.m_bKanjiCode_AutoDetect ){	/* 文字コード自動判別 */ 
						strcat( pOpt, "K" );
					}
					if( cDlgGrep.m_bGrepOutputLine ){	/* 行を出力するか該当部分だけ出力するか */
						strcat( pOpt, "P" ); 
					}

					if( 1 == cDlgGrep.m_nGrepOutputStyle ){	/* Grep: 出力形式 */
						strcat( pOpt, "1" ); 
					}
					if( 2 == cDlgGrep.m_nGrepOutputStyle ){	/* Grep: 出力形式 */
						strcat( pOpt, "2" ); 
					}


					if( 0 < lstrlen( pOpt ) ){
						strcat( pCmdLine, " -GOPT=" ); 
						strcat( pCmdLine, pOpt ); 
					}

					/* 新規編集ウィンドウの追加 ver 0 */
					CEditApp::OpenNewEditor( m_hInstance, m_pShareData->m_hwndTray, pCmdLine, 0, FALSE );

					delete [] pCmdLine;
					delete [] pOpt;

					break;
				case F_WIN_CLOSEALL:	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
					/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
					CEditApp::CloseAllEditor();
					break;
				case IDM_EXITALL:
					/* テキストエディタの終了 */
					CEditApp::TerminateApplication();
					break;
				default:
					if( nId - IDM_SELWINDOW  >= 0 &&
					    nId - IDM_SELWINDOW  < m_pShareData->m_nEditArrNum ){
						hwndWork = m_pShareData->m_pEditArr[nId - IDM_SELWINDOW].m_hWnd;

						/* アクティブにする */
						ActivateFrameWindow( hwndWork );
//						if( ::IsIconic( hwndWork ) ){
//							::ShowWindow( hwndWork, SW_RESTORE );
//						}else{
//							::ShowWindow( hwndWork, SW_SHOW );
//						}	
//						::SetForegroundWindow( hwndWork );
//						::SetActiveWindow( hwndWork );
					}else
					if( nId - IDM_SELMRU  >= 0 &&
//					    nId - IDM_SELMRU  < (( m_pShareData->m_nMRUArrNum < m_pShareData->m_Common.m_nMRUArrNum_MAX )?m_pShareData->m_nMRUArrNum :m_pShareData->m_Common.m_nMRUArrNum_MAX ) 
						nId - IDM_SELMRU  < 999
					){

//						char	szPath[_MAX_PATH + 3];
//						strcpy( szPath, &(m_pShareData->m_fiMRUArr[nId - IDM_SELMRU]) );
//						if( strchr( szPath, ' ' ) ){
//							char	pszFile2[_MAX_PATH + 3];
//							sprintf( pszFile2, "\"%s\"", szPath );
//							strcpy( szPath, pszFile2 );
//						}
						/* 新しい編集ウィンドウを開く */
						//	From Here Oct. 27, 2000 genta	カーソル位置を復元しない機能
						if( m_pShareData->m_Common.GetRestoreCurPosition()){
							CEditApp::OpenNewEditor2( m_hInstance, m_hWnd, &(m_pShareData->m_fiMRUArr[nId - IDM_SELMRU]), FALSE );
						}
						else {
							CEditApp::OpenNewEditor( m_hInstance, m_hWnd,
								m_pShareData->m_fiMRUArr[nId - IDM_SELMRU].m_szPath,
								m_pShareData->m_fiMRUArr[nId - IDM_SELMRU].m_nCharCode,
								FALSE );
							
						}
						//	To Here Oct. 27, 2000 genta
					}else
					if( nId - IDM_SELOPENFOLDER  >= 0 &&
//						nId - IDM_SELOPENFOLDER  < (( m_pShareData->m_nOPENFOLDERArrNum < m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX )?m_pShareData->m_nOPENFOLDERArrNum:m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX ) 
						nId - IDM_SELOPENFOLDER  < 999 
					){
						{
							CDlgOpenFile	cDlgOpenFile;
//							char*			pszMRU = NULL;;
							char			szPath[_MAX_PATH + 1];
							int				nCharCode;
							BOOL			bReadOnly;
							HWND			hWndOwner;
							FileInfo*		pfi;
							int				i;
							int				j;
							char**			ppszMRU;
							char**			ppszOPENFOLDER;

							/* MRUリストのファイルのリスト */
							ppszMRU = NULL;
							j = 0;
							if( m_pShareData->m_nMRUArrNum > 0 ){
								for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
									if( m_pShareData->m_Common.m_nMRUArrNum_MAX <= i ){
										break;
									}
									++j;
								}
							}
							ppszMRU = new char*[j + 1];
							if( j > 0 ){
								for( i = 0; i < j; ++i ){
						//			ppszMRU[i] = m_pShareData->m_Types[m_nSettingType].m_szMRUArr[i];
									ppszMRU[i] = m_pShareData->m_fiMRUArr[i].m_szPath;
								}
							}
							ppszMRU[j] = NULL;

							/* OPENFOLDERリストのファイルのリスト */
							ppszOPENFOLDER = NULL;
							j = 0;
							if( m_pShareData->m_nOPENFOLDERArrNum > 0 ){
								for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum; ++i ){
									if( m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX <= i ){
										break;
									} 
									++j;
								}
							}
							ppszOPENFOLDER = new char*[j + 1];
							if( j > 0 ){
								for( i = 0; i < j; ++i ){
						//			ppszOPENFOLDER[i] = m_pShareData->m_Types[m_nSettingType].m_szOPENFOLDERArr[i];
									ppszOPENFOLDER[i] = m_pShareData->m_szOPENFOLDERArr[i];
								}
							}
							ppszOPENFOLDER[j] = NULL;

							/* ファイルオープンダイアログの初期化 */
							strcpy( szPath, "" );
							nCharCode = CODE_AUTODETECT;	/* 文字コード自動判別 */
							bReadOnly = FALSE;
							cDlgOpenFile.Create( 
								m_hInstance, 
								NULL/*m_hWnd*/, 
								"*.*", 
								m_pShareData->m_szOPENFOLDERArr[nId - IDM_SELOPENFOLDER], 
								(const char **)ppszMRU,
								(const char **)ppszOPENFOLDER 
							);
							if( !cDlgOpenFile.DoModalOpenDlg( szPath, &nCharCode, &bReadOnly )){
								delete [] ppszMRU;
								delete [] ppszOPENFOLDER;
								break;
							}
							if( NULL == m_hWnd ){
								delete [] ppszMRU;
								delete [] ppszOPENFOLDER;
								break;
							}
							delete [] ppszMRU;
							delete [] ppszOPENFOLDER;
							/* 指定ファイルが開かれているか調べる */
							if( m_cShareData.IsPathOpened( szPath, &hWndOwner ) ){
								::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
//								pfi = (FileInfo*)m_pShareData->m_szWork;
								pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

								int nCharCodeNew;
								if( CODE_AUTODETECT == nCharCode ){	/* 文字コード自動判別 */
									/* 
									|| ファイルの日本語コードセット判別
									||
									|| 【戻り値】
									||	SJIS	0
									||	JIS		1    
									||	EUC		2    
									||	Unicode	3    
									||	エラー	-1
									*/
									nCharCodeNew = CMemory::CheckKanjiCodeOfFile( szPath );
									if( -1 == nCharCodeNew ){

									}else{
										nCharCode = nCharCodeNew;
									}
								}
								if( nCharCode != pfi->m_nCharCode ){	/* 文字コード種別 */
									char*	pszCodeNameCur;
									char*	pszCodeNameNew;
									switch( pfi->m_nCharCode ){
									case CODE_SJIS:		/* SJIS */		pszCodeNameCur = "SJIS";break;	//	Sept. 1, 2000 jepro 'シフト'を'S'に変更
									case CODE_JIS:		/* JIS */		pszCodeNameCur = "JIS";break;
									case CODE_EUC:		/* EUC */		pszCodeNameCur = "EUC";break;
									case CODE_UNICODE:	/* Unicode */	pszCodeNameCur = "Unicode";break;
									case CODE_UTF8:	/* UTF-8 */			pszCodeNameCur = "UTF-8";break;
									case CODE_UTF7:	/* UTF-7 */			pszCodeNameCur = "UTF-7";break;
									} 
									switch( nCharCode ){
									case CODE_SJIS:		/* SJIS */		pszCodeNameNew = "SJIS";break;	//	Sept. 1, 2000 jepro 'シフト'を'S'に変更
									case CODE_JIS:		/* JIS */		pszCodeNameNew = "JIS";break;
									case CODE_EUC:		/* EUC */		pszCodeNameNew = "EUC";break;
									case CODE_UNICODE:	/* Unicode */	pszCodeNameNew = "Unicode";break;
									case CODE_UTF8:	/* UTF-8 */			pszCodeNameNew = "UTF-8";break;
									case CODE_UTF7:	/* UTF-7 */			pszCodeNameNew = "UTF-7";break;
									} 
									::MYMESSAGEBOX(	m_hWnd,	MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
										"%s\n\n\n既に開いているファイルを違う文字コードで開く場合は、\n一旦閉じてから開いてください。\n\n現在の文字コードセット=[%s]\n新しい文字コードセット=[%s]", 
										szPath, pszCodeNameCur, pszCodeNameNew
									);
								}
								/* 開いているウィンドウをアクティブにする */
								/* アクティブにする */
								ActivateFrameWindow( hWndOwner );
//								if( ::IsIconic( hWndOwner ) ){
//									::ShowWindow( hWndOwner, SW_RESTORE );
//								}else{
//									::ShowWindow( hWndOwner, SW_SHOW );
//								}
//								::SetForegroundWindow( hWndOwner );
//								::SetActiveWindow( hWndOwner );
							}else{
								if( strchr( szPath, ' ' ) ){
									char	szFile2[_MAX_PATH + 3];
									wsprintf( szFile2, "\"%s\"", szPath );
									strcpy( szPath, szFile2 );
								}
								/* 新たな編集ウィンドウを起動 */
								CEditApp::OpenNewEditor( m_hInstance, m_hWnd, szPath, nCharCode, bReadOnly );
							}
						}
					}else{

					}
					break;
				}
				return 0L;
			case WM_LBUTTONDBLCLK:
//				MYTRACE( "WM_LBUTTONDBLCLK\n" );
				/* 新規編集ウィンドウの追加 */
				OpenNewEditor( m_hInstance, m_hWnd, (char*)NULL, 0, FALSE );
				return 0L;
			case WM_RBUTTONDBLCLK:
//				MYTRACE( "WM_RBUTTONDBLCLK\n" );
				return 0L;
			}
			break;

		case WM_QUERYENDSESSION:
			/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
			if( CloseAllEditor() ){
				DestroyWindow( hwnd );
				return TRUE;
			}else{
				return FALSE;
			}
		case WM_CLOSE:
			/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
			if( CloseAllEditor() ){
				DestroyWindow( hwnd );
			}
			return 0L;
			
		case WM_DESTROY:
			::UnregisterHotKey( m_hWnd, ID_HOTKEY_TRAYMENU );


		
//			/* 終了ダイアログを表示する */
//			if( TRUE == m_pShareData->m_Common.m_bDispExitingDialog ){
//				/* 終了中ダイアログの表示 */
//				hwndExitingDlg = ::CreateDialog( 
//					m_hInstance, 
//					MAKEINTRESOURCE( IDD_EXITING ),
//					m_hWnd/*::GetDesktopWindow()*/,
//					(DLGPROC)ExitingDlgProc
//				);
//				::ShowWindow( hwndExitingDlg, SW_SHOW );
//			}
//
//			/* 共有データの保存 */
//			m_cShareData.SaveShareData();
//
//			/* 終了ダイアログを表示する */
//			if( TRUE == m_pShareData->m_Common.m_bDispExitingDialog ){
//				/* 終了中ダイアログの破棄 */
//				::DestroyWindow( hwndExitingDlg );
//			}

			if( m_bCreatedTrayIcon ){	/* トレイにアイコンを作った  */
				TrayMessage( hwnd, NIM_DELETE, 0, NULL, NULL );
			}
			m_hWnd = NULL;
			/* Windows にスレッドの終了を要求します。*/
			::PostQuitMessage( 0 );
			return 0L;
	}
	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}




/* WM_COMMANDメッセージ処理 */
void CEditApp::OnCommand( WORD wNotifyCode, WORD wID , HWND hwndCtl )
{
	switch( wNotifyCode ){
	/* メニューからのメッセージ */
	case 0:
		break;
	}
	return;
}




/* 新規編集ウィンドウの追加 ver 0 */
//	Oct. 24, 2000 genta
//	WinExec -> CreateProcess．同期機能を付加，
bool CEditApp::OpenNewEditor( HINSTANCE hInstance, HWND hWndParent, char* pszPath, int nCharCode, BOOL bReadOnly, bool sync )
{
	CShareData		cShareData;
	DLLSHAREDATA*	pShareData;
	char szCmdLineBuf[1024];	//	コマンドライン
	char szEXE[MAX_PATH + 1];	//	アプリケーションパス名
	int nPos = 0;	//	コマンドライン構築用ポインタ

	/* 共有データ構造体のアドレスを返す */
	cShareData.Init();
	pShareData = cShareData.GetShareData( NULL, NULL );

	/* 編集ウィンドウの上限チェック */	
	if( pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
		char szMsg[512];
		wsprintf( szMsg, "編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。", MAX_EDITWINDOWS	);
		::MessageBox( NULL, szMsg, GSTR_APPNAME, MB_OK );
		return false;
	}

	::GetModuleFileName( ::GetModuleHandle( NULL ),	szEXE, sizeof(szEXE) );
	nPos += wsprintf( szCmdLineBuf + nPos, "\"%s\"", szEXE );

	//	ファイル名が指定されている場合
	//	コマンドライン引数が指定されているので，全体を""で囲んではいけない
	if( pszPath != NULL ){
		nPos += wsprintf( szCmdLineBuf + nPos, " %s", pszPath );
	}
	//	コード指定がある場合
	if( nCharCode != CODE_AUTODETECT ){
		nPos += wsprintf( szCmdLineBuf + nPos, " -CODE=%d", nCharCode );
	}

	//	DEBUG
	//	::MessageBox( NULL, szCmdLineBuf, "OpenNewEditor", MB_OK );

	//	プロセスの起動
    PROCESS_INFORMATION p;
    STARTUPINFO s;

    s.cb = sizeof( s );
    s.lpReserved = NULL;
    s.lpDesktop = NULL;
    s.lpTitle = NULL;
    /*
    s.dwX = CW_USEDEFAULT;
    s.dwY = CW_USEDEFAULT;
    s.dwXSize = CW_USEDEFAULT;
    s.dwYSize = CW_USEDEFAULT;
    */

    s.dwFlags = STARTF_USESHOWWINDOW;
    s.wShowWindow = SW_SHOWDEFAULT;
    s.cbReserved2 = 0;
    s.lpReserved2 = NULL;

    if( CreateProcess( szEXE, szCmdLineBuf, NULL, NULL, FALSE,
        CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &s, &p ) == 0 ){
		//	失敗
		LPVOID pMsg;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR) &pMsg,
						0,
						NULL
		);
		::MYMESSAGEBOX(
			hWndParent,
			MB_OK | MB_ICONSTOP,
			GSTR_APPNAME,
			"\'%s\'\nプロセスの起動に失敗しました。\n%s",
			szEXE,
			(char*)pMsg
		);
		::LocalFree( (HLOCAL)pMsg );	//	エラーメッセージバッファを解放
		return false;
	}

	if( sync ){
		//	起動したプロセスが完全に立ち上がるまでちょっと待つ．
		int nResult = WaitForInputIdle( p.hProcess, 10000 );	//	最大10秒間待つ
		if( nResult != 0 ){
			::MYMESSAGEBOX(
				hWndParent,
				MB_OK | MB_ICONSTOP,
				GSTR_APPNAME,
				"\'%s\'\nプロセスの起動に失敗しました。",
				szEXE
			);
		    CloseHandle( p.hThread );
		    CloseHandle( p.hProcess );
			return false;
		}
	}

    CloseHandle( p.hThread );
    CloseHandle( p.hProcess );

	return true;
}


#if 0
/* 新規編集ウィンドウの追加 ver 1 */
//void CEditApp::OpenNewEditor2( HINSTANCE hInstance, HWND hWndParent, FileInfo* pfi, BOOL bReadOnly )
{
	UINT			nRet;
	char*			pszCmdLine;
	char*			pszMsg;
	pszCmdLine = new char[1024];
	char			szPath[_MAX_PATH + 3];
	char			szEXE[MAX_PATH];
	char*			pszReadOnly;
	CShareData		cShareData;
	DLLSHAREDATA*	pShareData;

	/* 共有データ構造体のアドレスを返す */
	cShareData.Init();
	pShareData = cShareData.GetShareData( NULL, NULL );

	/* 編集ウィンドウの上限チェック */	
	if( pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
		char szMsg[512];
		wsprintf( szMsg, "編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。", MAX_EDITWINDOWS	);
		::MessageBox( NULL, szMsg, GSTR_APPNAME, MB_OK );
		return;
	}
	::GetModuleFileName( ::GetModuleHandle( NULL ),	szEXE, sizeof(szEXE) );
	strcpy( szPath, pfi->m_szPath );
	if( strchr( szPath, ' ' ) ){
		char	pszFile2[_MAX_PATH + 3];
		wsprintf( pszFile2, "\"%s\"", szPath );
		strcpy( szPath, pszFile2 );
	}
	if( bReadOnly ){
		pszReadOnly = "-R";
	}else{
		pszReadOnly = "";
	}
	/* 行桁指定を1開始にした */
	wsprintf( pszCmdLine, "%s %s -X=%d -Y=%d -VX=%d -VY=%d -CODE=%d %s", 
		szEXE, 
		szPath, 
		pfi->m_nX + 1,
		pfi->m_nY + 1,
		pfi->m_nViewLeftCol + 1,
		pfi->m_nViewTopLine + 1,
		pfi->m_nCharCode,
		pszReadOnly
	);

//複数プロセス版		
	if( 31 >= ( nRet = ::WinExec( pszCmdLine, SW_SHOW ) ) ){
		switch( nRet ){
		case 0:						pszMsg = "システムにメモリまたはリソースが足りません。";break;
		case ERROR_FILE_NOT_FOUND:	pszMsg = "指定されたファイルが見つかりませんでした。";break;
		case ERROR_PATH_NOT_FOUND:	pszMsg = "指定されたパスが見つかりませんでした。";break;
		case ERROR_BAD_FORMAT:		pszMsg = ".EXEファイルが無効です。";break;
		default:					pszMsg = "原因不明です。";break;
		}
		::MYMESSAGEBOX(
			hWndParent,
			MB_OK | MB_ICONSTOP,
			GSTR_APPNAME,
			"\'%s\'\nプロセスの起動に失敗しました。\n%s",
			szEXE,
			pszMsg
		);
	}

	delete [] pszCmdLine;
	return;
}
#endif

//	From Here Oct. 24, 2000 genta
//	新規編集ウィンドウの追加 ver 2:
bool CEditApp::OpenNewEditor2( HINSTANCE hInstance, HWND hWndParent, FileInfo* pfi, BOOL bReadOnly, bool sync )
{
	char			pszCmdLine[1024];
	CShareData		cShareData;
	DLLSHAREDATA*	pShareData;
	int				nPos = 0;		//	引数作成用ポインタ

	/* 共有データ構造体のアドレスを返す */
	cShareData.Init();
	pShareData = cShareData.GetShareData( NULL, NULL );

	/* 編集ウィンドウの上限チェック */	
	if( pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
		char szMsg[512];
		wsprintf( szMsg, "編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。", MAX_EDITWINDOWS	);
		::MessageBox( NULL, szMsg, GSTR_APPNAME, MB_OK );
		return false;
	}
	
	if( pfi != NULL ){
		if( pfi->m_szPath != NULL ){
			if( strlen( pfi->m_szPath ) > 0 ){
				nPos += wsprintf( pszCmdLine + nPos, " \"%s\"", pfi->m_szPath );
			}
		}
		if( pfi->m_nX >= 0 ){
			nPos += wsprintf( pszCmdLine + nPos, " -X=%d", pfi->m_nX +1 );
		}
		if( pfi->m_nY >= 0 ){
			nPos += wsprintf( pszCmdLine + nPos, " -Y=%d", pfi->m_nY +1 );
		}
		if( pfi->m_nViewLeftCol >= 0 ){
			nPos += wsprintf( pszCmdLine + nPos, " -VX=%d", pfi->m_nViewLeftCol +1 );
		}
		if( pfi->m_nViewTopLine >= 0 ){
			nPos += wsprintf( pszCmdLine + nPos, " -VY=%d", pfi->m_nViewTopLine +1 );
		}
		if( pfi->m_nCharCode >= 0 && pfi->m_nCharCode != CODE_AUTODETECT ){
			nPos += wsprintf( pszCmdLine + nPos, " -CODE=%d", pfi->m_nCharCode );
		}
		if( bReadOnly ){
			nPos += wsprintf( pszCmdLine + nPos, " -R" );
		}
	}
	//	::MessageBox( NULL, pszCmdLine, "OpenNewEditor", MB_OK );
	return OpenNewEditor( hInstance, hWndParent, pszCmdLine, CODE_AUTODETECT, bReadOnly, sync );

}
//	To Here	Oct. 24, 2000 genta


//シングルプロセス版用
//	/* 新規編集ウィンドウの追加 ver 2 */
//	HWND CEditApp::OpenNewEditor3( 
//		HINSTANCE hInstance, HWND hWndParent, const char* pszCommandLine, BOOL bActivate )
//	{
//		/* システムリソースのチェック */
//		if( FALSE == CheckSystemResources( GSTR_APPNAME ) ){
//			return NULL;
//		}
//
//		CShareData		m_cShareData;
//		DLLSHAREDATA*	pShareData;
//		CEditWnd*		pcEditWnd;
//		HWND			hWnd;
//		/* コマンドラインオプション */
//		BOOL			bGrepMode;
//		CMemory			cmGrepKey;
//		CMemory			cmGrepFile;
//		CMemory			cmGrepFolder;
//		CMemory			cmWork;
//		BOOL			bGrepSubFolder;
//		BOOL			bGrepLoHiCase;
//		BOOL			bGrepRegularExp;
//		BOOL			bGrepKanjiCode_AutoDetect;
//		BOOL			bGrepOutputLine;
//		BOOL			bDebugMode;
//		BOOL			bNoWindow;
//		FileInfo		fi;
//		BOOL			bReadOnly;
//
//		/* 共有データ構造体のアドレスを返す */
//		m_cShareData.Init();
//		pShareData = m_cShareData.GetShareData( NULL, NULL);
//
//		/* コマンドラインの解析 */
//		ParseCommandLine( 
//			pszCommandLine,
//			&bGrepMode,
//			&cmGrepKey,
//			&cmGrepFile,
//			&cmGrepFolder,
//			&bGrepSubFolder,
//			&bGrepLoHiCase,
//			&bGrepRegularExp,
//			&bGrepKanjiCode_AutoDetect,
//			&bGrepOutputLine,
//			&bDebugMode,
//			&bNoWindow,
//			&fi,
//			&bReadOnly
//		);
//
//		/* コマンドラインで受け取ったファイルが開かれている場合は、*/
//		/* その編集ウィンドウをアクティブにする */
//		if( 0 < lstrlen( fi.m_szPath ) ){
//			HWND hwndOwner;
//			/* 指定ファイルが開かれているか調べる */
//			if( TRUE == m_cShareData.IsPathOpened( fi.m_szPath, &hwndOwner ) ){
//				if( bActivate ){
//					/* アクティブにする */
//					ActivateFrameWindow( hwndOwner );
//				}
//				return hwndOwner;
//			}
//		}
//
//		pcEditWnd = new CEditWnd;
//		if( bDebugMode ){
//			hWnd = pcEditWnd->Create( hInstance, pShareData->m_hwndTray, NULL, 0, FALSE );
//
//	#ifdef _DEBUG/////////////////////////////////////////////
//			/* デバッグモニタモードに設定 */
//			pcEditWnd->SetDebugModeON();
//	#endif////////////////////////////////////////////////////		
//		}else
//		if( bGrepMode ){
//			hWnd = pcEditWnd->Create( hInstance, pShareData->m_hwndTray, NULL, 0, FALSE );
//			/* アクティブにする */
//			ActivateFrameWindow( hWnd );
//			/* GREP */
//			int			nHitCount;
//			GrepParam	GP;
//			GP.pCEditView             = (void*)&pcEditWnd->m_cEditDoc.m_cEditViewArr[0];
//			GP.pszGrepKey             = cmGrepKey.GetPtr( NULL );
//			GP.pszGrepFile            = cmGrepFile.GetPtr( NULL );
//			GP.pszGrepFolder          = cmGrepFolder.GetPtr( NULL );
//			GP.bGrepSubFolder         = bGrepSubFolder;
//			GP.bGrepLoHiCase          = bGrepLoHiCase;
//			GP.bGrepRegularExp        = bGrepRegularExp;
//			GP.bKanjiCode_AutoDetect  = bGrepKanjiCode_AutoDetect;
//			GP.bGrepOutputLine        = bGrepOutputLine;
//
//			/*nHitCount = */pcEditWnd->m_cEditDoc.m_cEditViewArr[0].DoGrep_Thread( (DWORD)&GP );
//	//		nHitCount = pcEditWnd->m_cEditDoc.m_cEditViewArr[0].DoGrep(
//	//			&cmGrepKey,
//	//			&cmGrepFile,
//	//			&cmGrepFolder,
//	//			bGrepSubFolder,
//	//			bGrepLoHiCase,
//	//			bGrepRegularExp,
//	//			bGrepKanjiCode_AutoDetect, 
//	//			bGrepOutputLine
//	//		);
//		}else{
//			if( 0 < (int)lstrlen( fi.m_szPath ) ){
//				hWnd = pcEditWnd->Create( hInstance, pShareData->m_hwndTray, fi.m_szPath, fi.m_nCharCode, bReadOnly/* 読み取り専用か */ );
//				if( fi.m_nViewTopLine < pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nViewTopLine = fi.m_nViewTopLine;
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nViewLeftCol = fi.m_nViewLeftCol;
//				}		
//				/*
//				  カーソル位置変換
//				  物理位置(行頭からのバイト数、折り返し無し行位置)
//				  →
//				  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
//				*/
//				int		nPosX;
//				int		nPosY;
//				pcEditWnd->m_cEditDoc.m_cLayoutMgr.CaretPos_Phys2Log(
//					fi.m_nX,
//					fi.m_nY,
//					&nPosX,
//					&nPosY
//				);
//				if( nPosY < pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX = nPosX;
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = nPosX;
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosY = nPosY;
//				}else{
//					int i;
//					i = pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() - 1;
//					if( i < 0 ){
//						i = 0;
//					}
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX = 0;
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = 0;
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosY = i;
//				}
//				pcEditWnd->m_cEditDoc.m_cEditViewArr[0].RedrawAll();
//			}else{
//				hWnd = pcEditWnd->Create( hInstance, pShareData->m_hwndTray, NULL, 0, FALSE );
//			}
//		}
//		if( bActivate ){
//			/* アクティブにする */
//			ActivateFrameWindow( hWnd );
//		}
//		return hWnd;
//	}





/* テキストエディタの終了 */
void CEditApp::TerminateApplication( void )
{
	CShareData		cShareData;
	DLLSHAREDATA*	pShareData;
//	int				nSettingType;

	/* 共有データ構造体のアドレスを返す */
	cShareData.Init();
	pShareData = cShareData.GetShareData( NULL, NULL );

	/* 現在の編集ウィンドウの数を調べる */
	if( pShareData->m_Common.m_bExitConfirm ){	//終了時の確認
		if( 0 < cShareData.GetEditorWindowsNum() ){
			if( IDYES != ::MYMESSAGEBOX(
				NULL,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				"現在開いている編集用のウィンドウをすべて閉じて終了しますか?"
			)){
				return;
			}
		}
	}
	/* 「すべてのウィンドウを閉じる」要求 */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
	if( CloseAllEditor() ){
		::PostMessage( pShareData->m_hwndTray, WM_CLOSE, 0, 0 );
	}
	return;
}




/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
BOOL CEditApp::CloseAllEditor( void )
{
//	int				i;
//	int				j;
//	HWND*			phWndArr;
//	DLLSHAREDATA*	pShareData;
//	/* 共有データ構造体のアドレスを返す */
//	pShareData = ::GetShareData();
	CShareData		cShareData;
	DLLSHAREDATA*	pShareData;
//	int				nSettingType;


	/* 共有データ構造体のアドレスを返す */
	cShareData.Init();
	pShareData = cShareData.GetShareData( NULL, NULL );

	/* 全編集ウィンドウへ終了要求を出す */
	if( !cShareData.RequestCloseAllEditor() ){
		return FALSE;
	}else{
		return TRUE;
	}
}




/* ポップアップメニュー(トレイ左ボタン) */
int	CEditApp::CreatePopUpMenu_L( void )
{
	int			i;
	int			j;
	int			nId;
	HMENU		hMenuTop;
	HMENU		hMenu;
	HMENU		hMenuPopUp;
	char		szMemu[300];
	POINT		po;
	RECT		rc;
	HWND		hwndDummy;
	int			nMenuNum;
	FileInfo*	pfi;

	m_CMenuDrawer.ResetContents();

	hMenuTop = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_TRAYMENU_L ) );
	hMenu = ::GetSubMenu( hMenuTop, 0 );
	nMenuNum = ::GetMenuItemCount( hMenu )/* - 1*/;
	for( i = nMenuNum - 1; i >= 0; i-- ){
		::DeleteMenu( hMenu, i, MF_BYPOSITION );
	}

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, "新規作成(&N)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, "開く(&O)...", FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GREP, "&Grep...", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	/* MRUリストのファイルのリストをメニューにする */
	j = 0;
	if( m_pShareData->m_nMRUArrNum > 0 ){
		for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
			/* 指定ファイルが開かれているか調べる */
			if( m_cShareData.IsPathOpened( m_pShareData->m_fiMRUArr[i].m_szPath, &hwndDummy ) ){
				continue;
			}
			j++;
		}
	}
	if( j > 0 ){
		hMenuPopUp = ::CreateMenu();
		j = 0;
		for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
			if( j >= m_pShareData->m_Common.m_nMRUArrNum_MAX )
				break;

			/* 指定ファイルが開かれているか調べる */
			if( m_cShareData.IsPathOpened( m_pShareData->m_fiMRUArr[i].m_szPath, &hwndDummy ) ){
				continue;
			}

//	From Here Oct. 4, 2000 JEPRO added, commented out & modified
//		ファイル名やパス名に'&'が使われているときに履歴等でキチンと表示されない問題を修正(&を&&に置換するだけ)
			char	szFile2[_MAX_PATH + 3];	//	'+1'かな？ ようわからんので多めにしとこ。わかる人修正報告下さい！
			char	*p;
			strcpy( szFile2, m_pShareData->m_fiMRUArr[i].m_szPath );
			if( (p = strchr( szFile2, '&' )) != NULL ){
				char	buf[_MAX_PATH + 3];	//	'+1'かな？ ようわからんので多めにしとこ。わかる人修正報告下さい！
				do {
					*p = '\0';
					strcpy( buf, p + strlen("&") );
					strcat( szFile2, "&&");
					strcat( szFile2, buf);
					p = strchr( p + strlen("&&"), '&' );
				} while ( p != NULL );
			}

//			if( j < 10 ){
//				wsprintf( szMemu, "&%c %s", j + '0', m_pShareData->m_fiMRUArr[i].m_szPath );
//			}
//			else if( j < 10 + 26 ){
//				wsprintf( szMemu, "&%c %s", j - 10 + 'A', m_pShareData->m_fiMRUArr[i].m_szPath );
//			}else{
//				wsprintf( szMemu, "  %s", m_pShareData->m_fiMRUArr[i].m_szPath );
//			}
//		修正の都合上作業変数で書くように変更
//		j >= 10 + 26 の時の考慮を省いた(に近い)がファイルの履歴MAXを36個にしてあるので事実上OKでしょう。
			wsprintf( szMemu, "&%c %s", (j < 10)?('0' + j):('A' + j - 10), szFile2 );
//		To Here Oct. 4, 2000
			if( 0 != m_pShareData->m_fiMRUArr[i].m_nCharCode ){		/* 文字コード種別 */
				switch( m_pShareData->m_fiMRUArr[i].m_nCharCode ){
//	From Here Oct. 5, 2000 JEPRO commented out & modified
//				case 1:
//					strcat( szMemu, "　[JIS]" );
//					break;
//				case 2:
//					strcat( szMemu, "　[EUC]" );
//					break;
//				case 3:
//					strcat( szMemu, "　[Unicode]" );
//					break;
					case CODE_JIS:	/* JIS */
						strcat( szMemu, "　[JIS]" );
						break;
					case CODE_EUC:	/* EUC */
						strcat( szMemu, "　[EUC]" );
						break;
					case CODE_UNICODE:	/* Unicode */
						strcat( szMemu, "　[Unicode]" );
						break;
					case CODE_UTF8:	/* UTF-8 */
						strcat( szMemu, "　[UTF-8]" );
						break;
					case CODE_UTF7:	/* UTF-7 */
						strcat( szMemu, "　[UTF-7]" );
						break;
//	To Here Oct. 5, 2000
				}
			}					
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELMRU + i, szMemu );
			j++;
//			if( m_cShareData.IsPathOpened( m_pShareData->m_fiMRUArr[i].m_szPath, &hwndDummy ) ){
//				if( 0 != _stricmp( m_cEditDoc.m_szFilePath, m_pShareData->m_fiMRUArr[i].m_szPath ) ){
//					::SetMenuItemBitmaps( hMenuPopUp, IDM_SELMRU + i, MF_BYCOMMAND | MF_CHECKED, NULL, m_hbmpOPENED );
//					::CheckMenuItem( hMenuPopUp, IDM_SELMRU + i, MF_BYCOMMAND | MF_CHECKED );
//				}else{
//					::SetMenuItemBitmaps( hMenuPopUp, IDM_SELMRU + i, MF_BYCOMMAND | MF_CHECKED, NULL, m_hbmpOPENED_THIS );
//					::CheckMenuItem( hMenuPopUp, IDM_SELMRU + i, MF_BYCOMMAND | MF_CHECKED );
//				}
//			}
		}
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "最近使ったファイル(&F)" );
	}else{
		hMenuPopUp = ::CreateMenu();
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , "最近使ったファイル(&F)" );
	}

	/* 最近使ったフォルダのメニューを作成 */
	if( m_pShareData->m_nOPENFOLDERArrNum > 0 ){
		hMenuPopUp = ::CreateMenu();
		j = 0;
		for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum ; ++i ){
			if( j >= m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX )
				break;

//	From Here Oct. 4, 2000 JEPRO added, commented out & modified
//		ファイル名やパス名に'&'が使われているときに履歴等でキチンと表示されない問題を修正(&を&&に置換するだけ)
			char	szFolder2[_MAX_PATH + 3];	//	'+1'かな？ ようわからんので多めにしとこ。わかる人修正報告下さい！
			char	*p;
			strcpy( szFolder2, m_pShareData->m_szOPENFOLDERArr[i] );
			if( (p = strchr( szFolder2, '&' )) != NULL ){
				char	buf[_MAX_PATH + 3];	//	'+1'かな？ ようわからんので多めにしとこ。わかる人修正報告下さい！
				do {
					*p = '\0';
					strcpy( buf, p + strlen("&") );
					strcat( szFolder2, "&&");
					strcat( szFolder2, buf);
					p = strchr( p + strlen("&&"), '&' );
				} while ( p != NULL );
			}

//			if( j < 10 ){
//				wsprintf( szMemu, "&%c %s", j + '0', m_pShareData->m_szOPENFOLDERArr[i] );
//			}
//			else if( j < 10 + 26 ){
//				wsprintf( szMemu, "&%c %s", j - 10 + 'A', m_pShareData->m_szOPENFOLDERArr[i] );
//			}else{
//				wsprintf( szMemu, "  %s", m_pShareData->m_szOPENFOLDERArr[i] );
//			}
//		修正の都合上作業変数で書くように変更
//		j >= 10 + 26 の時の考慮を省いた(に近い)がフォルダの履歴MAXを36個にしてあるので事実上OKでしょう。
			wsprintf( szMemu, "&%c %s", (j < 10)?('0' + j):('A' + j - 10), szFolder2 );
//		To Here Oct. 4, 2000
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELOPENFOLDER + i, szMemu );
			j++;
		}
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "最近使ったフォルダ(&D)" );
	}else{
		hMenuPopUp = ::CreateMenu();
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , "最近使ったフォルダ(&D)" );
	}


	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_CLOSEALL, "すべてのウィンドウを閉じる(&L)", FALSE );	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)

	

	/* 現在開いている編集窓のリストをメニューにする */
	j = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( CShareData::IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			++j;
		}
	}
//	::InsertMenu( hMenu, nMenuNum, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	++nMenuNum;

	if( j > 0 ){
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
		j = 0;
		for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
			if( CShareData::IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
				/* トレイからエディタへの編集ファイル名要求通知 */
				::SendMessage( m_pShareData->m_pEditArr[i].m_hWnd, MYWM_GETFILEINFO, 0, 0 );
//				pfi = (FileInfo*)m_pShareData->m_szWork;
				pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;
					if( pfi->m_bIsGrep ){
						/* データを指定バイト数以内に切り詰める */
						CMemory		cmemDes;
						int			nDesLen;
						const char*	pszDes;
						LimitStringLengthB( pfi->m_szGrepKey, lstrlen( pfi->m_szGrepKey ), 64, cmemDes );
						pszDes = cmemDes.GetPtr( NULL );
						nDesLen = lstrlen( pszDes );
//	From Here Oct. 4, 2000 JEPRO commented out & modified
//						wsprintf( szMemu, "&%d 【Grep】\"%s%s\"", ((i + 1) <= 9)? (i + 1):9, 
//							pszDes, ( (int)lstrlen( pfi->m_szGrepKey ) > nDesLen ) ? "・・・":""
//						);
//					}else{
//						wsprintf( szMemu, "&%d %s %s", ((i + 1) <= 9)? (i + 1):9, 
//							(0 < lstrlen(pfi->m_szPath))?pfi->m_szPath:"（無題）", 
//							pfi->m_bIsModified ? "*":" "
//						);
//		j >= 10 + 26 の時の考慮を省いた(に近い)が開くファイル数が36個を越えることはまずないので事実上OKでしょう。
						wsprintf( szMemu, "&%c 【Grep】\"%s%s\"", ((1 + i) <= 9)?('1' + i):('A' + i - 9), 
							pszDes, ( (int)lstrlen( pfi->m_szGrepKey ) > nDesLen ) ? "・・・":""
						);
					}else{
						wsprintf( szMemu, "&%c %s %s", ((1 + i) <= 9)?('1' + i):('A' + i - 9), 
							(0 < lstrlen(pfi->m_szPath))?pfi->m_szPath:"（無題）", 
							pfi->m_bIsModified ? "*":" "
						);
//		To Here Oct. 4, 2000
						if( 0 != pfi->m_nCharCode ){		/* 文字コード種別 */
							switch( pfi->m_nCharCode ){
							case CODE_JIS:	/* JIS */
								strcat( szMemu, "　[JIS]" );
								break;
							case CODE_EUC:	/* EUC */
								strcat( szMemu, "　[EUC]" );
								break;
							case CODE_UNICODE:	/* Unicode */
								strcat( szMemu, "　[Unicode]" );
								break;
							case CODE_UTF8:	/* UTF-8 */
								strcat( szMemu, "　[UTF-8]" );
								break;
							case CODE_UTF7:	/* UTF-7 */
								strcat( szMemu, "　[UTF-7]" );
								break;
							}
						}					
					}
//				
//				if( j <= 9 ){
//					wsprintf( szMemu, "&%d %s %s", j, 
//						(0 < lstrlen(pfi->m_szPath))?pfi->m_szPath:"（無題）", 
//						pfi->m_bIsModified ? "*":" " );
//				}else{
//					wsprintf( szMemu, "&%c %s %s", 'A' + j - 10, 
//						(0 < lstrlen(pfi->m_szPath))?pfi->m_szPath:"（無題）", 
//						pfi->m_bIsModified ? "*":" " );
//				}
//				if( 0 != pfi->m_nCharCode ){		/* 文字コード種別 */
//					switch( pfi->m_nCharCode ){
//					case 1:	/* JIS */
//						strcat( szMemu, "　[JIS]" );
//						break;
//					case 2:	/* EUC */
//						strcat( szMemu, "　[EUC]" );
//						break;
//					case 3:	/* Unicode */
//						strcat( szMemu, "　[Unicode]" );
//						break;
//					}
//				}					

					
//				::InsertMenu( hMenu, IDM_EXITALL, MF_BYCOMMAND | MF_STRING, IDM_SELWINDOW + i, szMemu );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + i, szMemu, FALSE );
				++j;
			}
		}
		if( j > 0 ){
//			::InsertMenu( hMenu, IDM_EXITALL, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
			++nMenuNum;
		}
		nMenuNum += j;
	}
	if( j == 0 ){
		::EnableMenuItem( hMenu, F_WIN_CLOSEALL, MF_BYCOMMAND | MF_GRAYED );	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
	}

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_EXITALL, "テキストエディタの全終了(&X)", FALSE );

	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );
	po.y -= 4;

	rc.left = 0;
	rc.right = 0;
	rc.top = 0;
	rc.bottom = 0;

	::SetForegroundWindow( m_hWnd );
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_BOTTOMALIGN
		| TPM_RIGHTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		m_hWnd,
		&rc
	);
	::PostMessage( m_hWnd, WM_USER + 1, 0, 0 );
	::DestroyMenu( hMenuTop );
//	MYTRACE( "nId=%d\n", nId );
	return nId;
}





//	Oct. 12, 2000 JEPRO 下のコードを部分的に下手にいじるより全部コメントアウトしてその下に新たに追加することにした
/* ポップアップメニュー(トレイ右ボタン) */
//int	CEditApp::CreatePopUpMenu_R( void )
//{
//	int		i;
//	int		j;
//	int		nId;
//	HMENU	hMenuTop;
//	HMENU	hMenu;
//	char	szMemu[300];
//	POINT	po;
//	RECT	rc;
//
////	m_CMenuDrawer.ResetContents();
//
//
//	hMenuTop = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_TRAYMENU_L ) );	//Oct. 12, 2000 jepro note: ここでリソースのMenuにある「トレイ左ボタンポップアップメニュー」を読み込んでいる
//	hMenu = ::GetSubMenu( hMenuTop, 0 );
//	if( m_pShareData->m_nMRUArrNum > 0 ){	//Oct. 12, 2000 jepro note: ここで最近使ったファイルのリストを作成している
//		j = 0;
//		::InsertMenu( hMenu, IDM_EXITALL, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL );
//		for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
//			if( m_pShareData->m_Common.m_nMRUArrNum_MAX <= i ){
//				break;
//			}
////	From Here Oct. 4, 2000 JEPRO commented out & modified
////			if( j <= 9 ){
////				wsprintf( szMemu, "&%d %s", j, m_pShareData->m_fiMRUArr[i].m_szPath );
////			}else{
////				wsprintf( szMemu, "&%c %s", 'A' + j - 10, m_pShareData->m_fiMRUArr[i].m_szPath );
////			}
////		j >= 10 + 26 の時の考慮を省いた(に近い)がファイルの履歴MAXを36個にしてあるので事実上OKでしょう。
//			wsprintf( szMemu, "&%c %s", (j < 10)?('0' + j):('A' + j - 10), m_pShareData->m_fiMRUArr[i].m_szPath );
////	To Here Oct. 4, 2000
//			::InsertMenu( hMenu, IDM_EXITALL, MF_BYCOMMAND | MF_STRING, IDM_SELWINDOW + i, szMemu );
////			m_CMenuDrawer.MyAppendMenu( 
////				hMenu, MF_BYPOSITION | MF_STRING | MF_ENABLED, 
////				IDM_SELWINDOW + i , szMemu 
////			);
//			++j;
//		}
//	}
//	::InsertMenu( hMenu, IDM_EXITALL, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL );
//	po.x = 0;
//	po.y = 0;
//	::GetCursorPos( &po );
//	po.y -= 4;
//
//	rc.left = 0;
//	rc.right = 0;
//	rc.top = 0;
//	rc.bottom = 0;
//
//	::SetForegroundWindow( m_hWnd );
//	nId = ::TrackPopupMenu(
//		hMenu,
//		TPM_BOTTOMALIGN
//		| TPM_RIGHTALIGN
//		| TPM_RETURNCMD
//		| TPM_LEFTBUTTON
//		/*| TPM_RIGHTBUTTON*/
//		,
//		po.x,
//		po.y,
//		0,
//		m_hWnd,
//		&rc
//	);
//	::PostMessage( m_hWnd, WM_USER + 1, 0, 0 );
//	::DestroyMenu( hMenuTop );
////	MYTRACE( "nId=%d\n", nId );
//	return nId;
//}




//キーワード：トレイ右クリックメニュー順序
//	Oct. 12, 2000 JEPRO ポップアップメニュー(トレイ左ボタン) を参考にして新たに追加した部分

/* ポップアップメニュー(トレイ右ボタン) */
int	CEditApp::CreatePopUpMenu_R( void )
{
	int		i;
	int		nId;
	HMENU	hMenuTop;
	HMENU	hMenu;
	POINT	po;
	RECT	rc;
	int		nMenuNum;

	m_CMenuDrawer.ResetContents();

	hMenuTop = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_TRAYMENU_L ) );
	hMenu = ::GetSubMenu( hMenuTop, 0 );
	nMenuNum = ::GetMenuItemCount( hMenu )/* - 1*/;
	for( i = nMenuNum - 1; i >= 0; i-- ){
		::DeleteMenu( hMenu, i, MF_BYPOSITION );
	}

	/* トレイ右クリックの「オプション」メニュー */
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TYPE_LIST, "タイプ別設定一覧(&L)...", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION_TYPE, "タイプ別設定(&Y)...", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION, "共通設定(&C)...", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FONT, "フォント設定(&F)...", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );


	/* トレイ右クリックの「ヘルプ」メニュー */
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_HELP_CONTENTS , "ヘルプ目次(&O)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_HELP_SEARCH , "ヘルプトピックの検索(&S)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MENU_ALLFUNC , "コマンド一覧(&M)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
		
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHELP1 , "外部ヘルプ１(&E)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHTMLHELP , "外部HTMLヘルプ(&H)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_ABOUT, "バージョン情報(&A)", FALSE );
	
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_EXITALL, "テキストエディタの全終了(&X)", FALSE );

	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );
	po.y -= 4;

	rc.left = 0;
	rc.right = 0;
	rc.top = 0;
	rc.bottom = 0;

	::SetForegroundWindow( m_hWnd );
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_BOTTOMALIGN
		| TPM_RIGHTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		m_hWnd,
		&rc
	);
	::PostMessage( m_hWnd, WM_USER + 1, 0, 0 );
	::DestroyMenu( hMenuTop );
//	MYTRACE( "nId=%d\n", nId );
	return nId;
}





/* コマンドラインの解析 */
void CEditApp::ParseCommandLine( 
	const char*	pszCmdLineSrc,
	BOOL*		pbGrepMode,
	CMemory*	pcmGrepKey,
	CMemory*	pcmGrepFile,
	CMemory*	pcmGrepFolder,
	BOOL*		pbGrepSubFolder,
	BOOL*		pbGrepLoHiCase,
	BOOL*		pbGrepRegularExp,
	BOOL*		pbGrepKanjiCode_AutoDetect,
	BOOL*		pbGrepOutputLine,
	int	*		pnGrepOutputStyle,
	BOOL*		pbDebugMode,
	BOOL*		pbNoWindow,
	FileInfo*	pfi,
	BOOL*		pbReadOnly
)
{
	BOOL			bGrepMode;
	CMemory			cmGrepKey;
	CMemory			cmGrepFile;
	CMemory			cmGrepFolder;
	BOOL			bGrepSubFolder;
	BOOL			bGrepLoHiCase;
	BOOL			bGrepRegularExp;
	BOOL			bGrepKanjiCode_AutoDetect;
	BOOL			bGrepOutputLine;
	int				nGrepOutputStyle;
	BOOL			bDebugMode;
	BOOL			bNoWindow;
	FileInfo		fi;
	BOOL			bReadOnly;
	char*			pszCmdLineWork;
	int				nCmdLineWorkLen;
	BOOL			bFind;
//	WIN32_FIND_DATA	w32fd;
//	HANDLE			hFind;
	char			szPath[_MAX_PATH + 1];
	int				i;
	int				j;
	int				nPos;
	char*			pszToken;
	CMemory			cmWork;
	const char*		pszOpt;
	int				nOptLen;

	bGrepMode = FALSE;
	bGrepSubFolder = FALSE;
	bGrepLoHiCase = FALSE;
	bGrepRegularExp = FALSE;
	bGrepKanjiCode_AutoDetect = FALSE;
	bGrepOutputLine = FALSE;
	nGrepOutputStyle = 1;
	bDebugMode = FALSE;
	bNoWindow = FALSE;

	fi.m_nViewTopLine = 0;		/* 表示域の一番上の行(0開始) */	
	fi.m_nViewLeftCol = 0;		/* 表示域の一番左の桁(0開始) */
	fi.m_nX = 0;		/* カーソル　物理位置(行頭からのバイト数) */
	fi.m_nY = 0;		/* カーソル　物理位置(折り返し無し行位置) */
	fi.m_bIsModified = 0;		/* 変更フラグ */
	fi.m_nCharCode = CODE_AUTODETECT;		/* 文字コード種別 *//* 文字コード自動判別 */
	fi.m_szPath[0] = '\0';	/* ファイル名 */
	bReadOnly = FALSE;	/* 読み取り専用か */

	//	May 30, 2000 genta
	//	実行ファイル名をもとに漢字コードを固定する．
	{
		char exename[512];
		::GetModuleFileName( NULL, exename, 512 );

		int len = strlen( exename );

		for( char *p = exename + len - 1; p > exename; p-- ){
			if( *p == '.' ){
				if( '0' <= p[-1] && p[-1] <= '5' )
					fi.m_nCharCode = p[-1] - '0';
				break;
			}
		}
	}





	bFind = FALSE;
	if( pszCmdLineSrc[0] != '-' ){
		memset( (char*)szPath, 0, sizeof( szPath ) );
		i = 0;
		j = 0;
		for( ; i < sizeof( szPath ) - 1 && i <= (int)lstrlen(pszCmdLineSrc); ++i ){
			if( pszCmdLineSrc[i] != ' ' && pszCmdLineSrc[i] != '\0' ){
				szPath[j] = pszCmdLineSrc[i];
				++j;
				continue;
			}
			/* ファイルの存在と、ファイルかどうかをチェック */
			if( -1 != _access( szPath, 0 ) ){
//? 2000.01.18 システム属性のファイルが開けない問題
//?				hFind = ::FindFirstFile( szPath, &w32fd );
//?				::FindClose( hFind );
//?				if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ||
//?					w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
//?				}else{
					bFind = TRUE;
					break;
//?				}
			}			
			szPath[j] = pszCmdLineSrc[i];
			++j;
		}
	}
	if( bFind ){
		strcpy( fi.m_szPath, szPath );	/* ファイル名 */
		nPos = j + 1;
	}else{
		nPos = 0;
	}
	pszCmdLineWork = new char[lstrlen( pszCmdLineSrc ) + 1];
	strcpy( pszCmdLineWork, pszCmdLineSrc );
	nCmdLineWorkLen = lstrlen( pszCmdLineWork );
	pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, " " );
	while( pszToken != NULL ){
		if( !bFind && pszToken[0] != '-' ){
		    if( pszToken[0] == '\"' ){
				cmWork.SetData( &pszToken[1],  lstrlen( pszToken ) - 2 );				
				cmWork.Replace( "\"\"", "\"" );
				strcpy( fi.m_szPath, cmWork.GetPtr( NULL/*&nDummy*/ ) );	/* ファイル名 */
			}else{
				strcpy( fi.m_szPath, pszToken );	/* ファイル名 */
			}
		}else{
			pszOpt = "-X=";
			nOptLen = lstrlen( pszOpt );
			if( ( (int)lstrlen( pszToken ) > nOptLen ) && ( 0 == memcmp( pszOpt, pszToken, nOptLen ) ) ){
				/* 行桁指定を1開始にした */
				fi.m_nX = atoi( pszToken + nOptLen ) - 1;
				goto end_of_options;
			}
			pszOpt = "-Y=";
			nOptLen = lstrlen( pszOpt );
			if( ( (int)lstrlen( pszToken ) > nOptLen ) && ( 0 == memcmp( pszOpt, pszToken, nOptLen ) ) ){
				/* 行桁指定を1開始にした */
				fi.m_nY = atoi( pszToken + nOptLen ) - 1;
				goto end_of_options;
			}
			pszOpt = "-VX=";
			nOptLen = lstrlen( pszOpt );
			if( ( (int)lstrlen( pszToken ) > nOptLen ) && ( 0 == memcmp( pszOpt, pszToken, nOptLen ) ) ){
				/* 行桁指定を1開始にした */
				fi.m_nViewLeftCol = atoi( pszToken + nOptLen ) - 1;
				goto end_of_options;
			}
			pszOpt = "-VY=";
			nOptLen = lstrlen( pszOpt );
			if( ( (int)lstrlen( pszToken ) > nOptLen ) && ( 0 == memcmp( pszOpt, pszToken, nOptLen ) ) ){
				/* 行桁指定を1開始にした */
				fi.m_nViewTopLine = atoi( pszToken + nOptLen ) - 1;
				goto end_of_options;
			}
			pszOpt = "-CODE=";
			nOptLen = lstrlen( pszOpt );
			if( ( (int)lstrlen( pszToken ) > nOptLen ) && ( 0 == memcmp( pszOpt, pszToken, nOptLen ) ) ){
				fi.m_nCharCode = atoi( pszToken + nOptLen );
				goto end_of_options;
			}
			pszOpt = "-R";
			nOptLen = lstrlen( pszOpt );
			if( ( (int)lstrlen( pszToken ) == nOptLen ) && ( 0 == memcmp( pszOpt, pszToken, nOptLen ) ) ){
				bReadOnly = TRUE;
				goto end_of_options;
			}
			pszOpt = "-NOWIN";
			nOptLen = lstrlen( pszOpt );
			if( ( (int)lstrlen( pszToken ) == nOptLen ) && ( 0 == memcmp( pszOpt, pszToken, nOptLen ) ) ){
				bNoWindow = TRUE;
				goto end_of_options;
			}
			pszOpt = "-GREPMODE";
			nOptLen = lstrlen( pszOpt );
			if( ( (int)lstrlen( pszToken ) == nOptLen ) && ( 0 == memcmp( pszOpt, pszToken, nOptLen ) ) ){
				bGrepMode = TRUE;
				goto end_of_options;
			}
			pszOpt = "-GKEY=";
			nOptLen = lstrlen( pszOpt );
			if( ( (int)lstrlen( pszToken ) > nOptLen ) && ( 0 == memcmp( pszOpt, pszToken, nOptLen ) ) ){
				cmGrepKey.SetData( &pszToken[nOptLen + 1],  lstrlen( pszToken ) - (nOptLen + 2) );				
				cmGrepKey.Replace( "\"\"", "\"" );
				goto end_of_options;
			}
			pszOpt = "-GFILE=";
			nOptLen = lstrlen( pszOpt );
			if( ( (int)lstrlen( pszToken ) > nOptLen ) && ( 0 == memcmp( pszOpt, pszToken, nOptLen ) ) ){
				cmGrepFile.SetData( &pszToken[nOptLen + 1],  lstrlen( pszToken ) - (nOptLen + 2) );				
				cmGrepFile.Replace( "\"\"", "\"" );
				goto end_of_options;
			}
			pszOpt = "-GFOLDER=";
			nOptLen = lstrlen( pszOpt );
			if( ( (int)lstrlen( pszToken ) > nOptLen ) && ( 0 == memcmp( pszOpt, pszToken, nOptLen ) ) ){
				cmGrepFolder.SetData( &pszToken[nOptLen + 1],  lstrlen( pszToken ) - (nOptLen + 2) );				
				cmGrepFolder.Replace( "\"\"", "\"" );
				goto end_of_options;
			}
			pszOpt = "-GOPT=";
			nOptLen = lstrlen( pszOpt );
			if( ( (int)lstrlen( pszToken ) > nOptLen ) && ( 0 == memcmp( pszOpt, pszToken, nOptLen ) ) ){
				for( i = nOptLen; i < (int)lstrlen( pszToken ); ++i ){
					switch( pszToken[i] ){
					case 'S':	/* サブフォルダからも検索する */ 
						bGrepSubFolder = TRUE;	break;
					case 'L':	/* 英大文字と英小文字を区別する */ 
						bGrepLoHiCase = TRUE;	break;
					case 'R':	/* 正規表現 */ 
						bGrepRegularExp = TRUE;	break;
					case 'K':	/* 文字コード自動判別 */ 
						bGrepKanjiCode_AutoDetect = TRUE;	break;
					case 'P':	/* 行を出力するか該当部分だけ出力するか */
						bGrepOutputLine = TRUE;	break;
					case '1':	/* Grep: 出力形式 */
						nGrepOutputStyle = 1;	break;
					case '2':	/* Grep: 出力形式 */
						nGrepOutputStyle = 2;	break;
					}
				}
				goto end_of_options;
			}
			pszOpt = "-DEBUGMODE";
			nOptLen = lstrlen( pszOpt );
			if( ( (int)lstrlen( pszToken ) == nOptLen ) && ( 0 == memcmp( pszOpt, pszToken, nOptLen ) ) ){
				bDebugMode = TRUE;
				goto end_of_options;
			}
			end_of_options:;
		}
		pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, " " );
	}
	delete [] pszCmdLineWork;

	/* ファイル名 */
	if( '\0' != fi.m_szPath[0] ){
		/* ショートカット(.lnk)の解決 */
		if( TRUE == ResolveShortcutLink( NULL, fi.m_szPath, szPath ) ){
			strcpy( fi.m_szPath, szPath );
		}
		/* ロングファイル名を取得する */
		if( TRUE == ::GetLongFileName( fi.m_szPath, szPath ) ){
			strcpy( fi.m_szPath, szPath );
		}

		/* MRUから情報取得 */

	}
	
	/* 処理結果を格納 */
	*pbGrepMode                 = bGrepMode;
	*pcmGrepKey                 = cmGrepKey;
	*pcmGrepFile                = cmGrepFile;
	*pcmGrepFolder              = cmGrepFolder;
	*pbGrepSubFolder            = bGrepSubFolder;
	*pbGrepLoHiCase             = bGrepLoHiCase;
	*pbGrepRegularExp           = bGrepRegularExp;
	*pbGrepKanjiCode_AutoDetect = bGrepKanjiCode_AutoDetect;
	*pbGrepOutputLine           = bGrepOutputLine;
	*pnGrepOutputStyle			= nGrepOutputStyle;
	*pbDebugMode                = bDebugMode;
	*pbNoWindow                 = bNoWindow;
	*pfi                        = fi;
	*pbReadOnly                 = bReadOnly;
	return;
}


/*[EOF]*/
