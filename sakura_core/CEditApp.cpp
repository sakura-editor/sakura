//	$Id$
/*!	@file
	@brief 常駐部
	
	タスクトレイアイコンの管理，タスクトレイメニューのアクション，
	MRU、キー割り当て、共通設定、編集ウィンドウの管理など

	@author Norio Nakatani
	@date 1998/05/13 新規作成
	@date 2001/06/03 N.Nakatani grep単語単位で検索を実装するときのためにコマンドラインオプションの処理追加
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, stonee, jepro, genta, aroka, hor
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#define ID_HOTKEY_TRAYMENU	0x1234

#include <windows.h>
#include <io.h>
#include "debug.h"
#include "CEditApp.h"
#include "CEditView.h"		//Nov. 21, 2000 JEPROtest
#include "CEditDoc.h"		//Nov. 21, 2000 JEPROtest
#include "CEditWnd.h"		//Nov. 21, 2000 JEPROtest
#include "CDlgAbout.h"		//Nov. 21, 2000 JEPROtest
#include "CDlgTypeList.h"	//Nov. 21, 2000 JEPROtest
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

//Stonee, 2001/03/21
//Stonee, 2001/07/01  多重起動された場合は前回のダイアログを前面に出すようにした。
void CEditApp::DoGrep()
{
//	CDlgGrep	cDlgGrep;	// Jul. 2, 2001 genta
	char*			pCmdLine;
	char*			pOpt;
//  	int				nDataLen;
	int				nRet;
	CMemory			cmWork1;
	CMemory			cmWork2;
	CMemory			cmWork3;

	/* Grep */
	/*Grepダイアログの初期化１ */
//	cDlgGrep.Create( m_hInstance, /*m_hWnd*/NULL );

	//Stonee, 2001/06/30
	//前回のダイアログがあれば前面に (suggested by genta)
	if ( ::IsWindow(m_cDlgGrep.m_hWnd) ){
		::OpenIcon(m_cDlgGrep.m_hWnd);
		::BringWindowToTop(m_cDlgGrep.m_hWnd);
		return;
	}

	strcpy( m_cDlgGrep.m_szText, m_pShareData->m_szSEARCHKEYArr[0] );

	/* Grepダイアログの表示 */
	nRet = m_cDlgGrep.DoModal( m_hInstance, /*m_hWnd*/NULL, "" );
//					MYTRACE( "nRet=%d\n", nRet );
	if( FALSE == nRet || m_hWnd == NULL ){
		return;
	}

//					MYTRACE( "cDlgGrep.m_szText  =[%s]\n", cDlgGrep.m_szText );
//					MYTRACE( "cDlgGrep.m_szFile  =[%s]\n", cDlgGrep.m_szFile );
//					MYTRACE( "cDlgGrep.m_szFolder=[%s]\n", cDlgGrep.m_szFolder );

	/*======= Grepの実行 =============*/
	/* Grep結果ウィンドウの表示 */

	pCmdLine = new char[1024];
	pOpt = new char[64];

	cmWork1.SetDataSz( m_cDlgGrep.m_szText );
	cmWork2.SetDataSz( m_cDlgGrep.m_szFile );
	cmWork3.SetDataSz( m_cDlgGrep.m_szFolder );
	cmWork1.Replace( "\"", "\"\"" );
	cmWork2.Replace( "\"", "\"\"" );
	cmWork3.Replace( "\"", "\"\"" );

	/*
	|| -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
	*/
	wsprintf( pCmdLine, "-GREPMODE -GKEY=\"%s\" -GFILE=\"%s\" -GFOLDER=\"%s\" -GCODE=%d",
		cmWork1.GetPtr(),
		cmWork2.GetPtr(),
		cmWork3.GetPtr(),
		m_cDlgGrep.m_nGrepCharSet
	);

	pOpt[0] = '\0';
	if( m_cDlgGrep.m_bSubFolder ){			/* サブフォルダからも検索する */
		strcat( pOpt, "S" );
	}

	if( m_cDlgGrep.m_bLoHiCase ){				/* 英大文字と英小文字を区別する */
		strcat( pOpt, "L" );
	}
	if( m_cDlgGrep.m_bRegularExp ){			/* 正規表現 */
		strcat( pOpt, "R" );
	}
//	2002/09/20 Moca 文字コードセットオプションに統合
//	if( m_cDlgGrep.m_bKanjiCode_AutoDetect ){	/* 文字コード自動判別 */
//		strcat( pOpt, "K" );
//	}
	if( m_cDlgGrep.m_bGrepOutputLine ){		/* 行を出力するか該当部分だけ出力するか */
		strcat( pOpt, "P" );
	}
	if( m_cDlgGrep.m_bWordOnly ){				/* 単語単位で探す */
		strcat( pOpt, "W" );
	}
	if( 1 == m_cDlgGrep.m_nGrepOutputStyle ){	/* Grep: 出力形式 */
		strcat( pOpt, "1" );
	}
	if( 2 == m_cDlgGrep.m_nGrepOutputStyle ){	/* Grep: 出力形式 */
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
}

//	BOOL CALLBACK ExitingDlgProc(
//	  HWND hwndDlg,	// handle to dialog box
//	  UINT uMsg,	// message
//	  WPARAM wParam,// first message parameter
//	  LPARAM lParam	// second message parameter
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
//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CEditApp::CEditApp() :
	//	Apr. 24, 2001 genta
	m_uCreateTaskBarMsg( ::RegisterWindowMessage( TEXT("TaskbarCreated") ) ),
	m_bCreatedTrayIcon( FALSE ),	//トレイにアイコンを作った
	m_hInstance( NULL ),
	m_hWnd( NULL )
{
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();
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
		::MessageBox( NULL, "CEditApp::CEditApp()\nアクセラレータ テーブルが作成できません。\nシステムリソースが不足しています。", GSTR_APPNAME, MB_OK | MB_ICONSTOP );
	}

//	#ifdef _DEBUG
//		m_pszAppName = "CEditApp[デバッグバージョン]";
//	#endif
//	#ifndef _DEBUG
		m_pszAppName = GSTR_CEDITAPP;
//	#endif

	m_bUseTrayMenu = false;

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
	MY_RUNNINGTIMER( cRunningTimer, "CEditApp::Create" );

	WNDCLASS	wc;
//	HANDLE		hMutex;
//	HWND		hWnd;
//	MSG			msg;
	ATOM		atom;
//	char		szMutexName[260];
//	HICON		hIcon;
//	HACCEL		hAccel;
//	int			nRet;
	HWND		hwndWork;
//	const char *pszTitle="sakura new UR1.2.20.0";	//Nov. 12, 2000 JEPROtestnow トレイにバージョンが表示されなくなってしまったので修正してみたが失敗
//	const char *pszTips	="sakura tipsUR1.2.20.0";	//Nov. 12, 2000 JEPROtestnow トレイにバージョンが表示されなくなってしまったので修正してみたが失敗

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
		if( 0 == atom ){
			::MessageBox( NULL, "CEditApp::Create()\nウィンドウクラスを登録できませんでした。",
					GSTR_APPNAME, MB_OK | MB_ICONSTOP );
		}
//	}else{
//		return NULL;
//	}
	g_m_pCEditApp = this;

	/*hWnd =*/ ::CreateWindow(
		m_pszAppName,						// pointer to registered class name
//		pszTitle,							// pointer to registered class name	//Nov. 12, 2000 JEPROtestnow トレイにバージョンが表示されなくなってしまったので修正してみたが失敗
		m_pszAppName,						// pointer to window name
//		pszTips,							// pointer to window name			//Nov. 12, 2000 JEPROtestnow トレイにバージョンが表示されなくなってしまったので修正してみたが失敗
		WS_OVERLAPPEDWINDOW/*WS_VISIBLE *//*| WS_CHILD *//* | WS_CLIPCHILDREN*/	,	// window style
		CW_USEDEFAULT,						// horizontal position of window
		0,									// vertical position of window
		100,								// window width
		100,								// window height
		NULL,								// handle to parent or owner window
		NULL,								// handle to menu or child-window identifier
		m_hInstance,						// handle to application instance
		NULL								// pointer to window-creation data
	);
//	m_hWnd = hWnd;
//	MYMESSAGEBOX( "m_hWnd=%08xH \n", m_hWnd );
	MY_TRACETIME( cRunningTimer, "Window is created" );

	//	Oct. 16, 2000 genta
	m_hIcons.Create( m_hInstance, m_hWnd );
	
	MY_TRACETIME( cRunningTimer, "Icons are created" );
	
	m_CMenuDrawer.Create( m_hInstance, m_hWnd, &m_hIcons );

	if( NULL != m_hWnd ){
		CreateTrayIcon( m_hWnd );
	}else{
	}
//	::ShowWindow( m_hWnd, SW_SHOW );

	/* Windows アクセラレータの作成 */
//	m_CKeyBind.Create( m_hInstance );
//	m_hAccel = m_CKeyBind.CreateAccerelator();
	return m_hWnd;
}

//! タスクトレイにアイコンを登録する
bool CEditApp::CreateTrayIcon( HWND hWnd )
{
	HICON hIcon;
//		::SetWindowLong( m_hWnd, GWL_USERDATA, (LONG)this );
		/* タスクトレイのアイコンを作る */
		if( TRUE == m_pShareData->m_Common.m_bUseTaskTray ){	/* タスクトレイのアイコンを使う */
			//	Dec. 02, 2002 genta
			hIcon = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
//From Here Jan. 12, 2001 JEPRO トレイアイコンにポイントするとバージョンno.が表示されるように修正
//			TrayMessage( m_hWnd, NIM_ADD, 0,  hIcon, GSTR_APPNAME );
			/* バージョン情報 */
			//	UR version no.を設定 (cf. cDlgAbout.cpp)
			char	pszTips[64];
			char	pszTipsVerno[32];

			strcpy( pszTips, GSTR_APPNAME );
			wsprintf( pszTipsVerno, " %d.%d.%d.%d",		//Jul. 06, 2001 jepro UR はもう付けなくなったのを忘れていた
			HIWORD( m_pShareData->m_dwProductVersionMS ),
			LOWORD( m_pShareData->m_dwProductVersionMS ),
			HIWORD( m_pShareData->m_dwProductVersionLS ),
			LOWORD( m_pShareData->m_dwProductVersionLS )
			);
			strcat( pszTips, pszTipsVerno );
			TrayMessage( m_hWnd, NIM_ADD, 0,  hIcon, pszTips );
//To Here Jan. 12, 2001
			m_bCreatedTrayIcon = TRUE;	/* トレイにアイコンを作った */
		}
	return true;
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
//			if( NULL != pCEditWnd->m_hwndPrintPreviewBar && ::IsDialogMessage( pCEditWnd->m_hwndPrintPreviewBar, &msg )  ){	/* 印刷プレビュー 操作バー */
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
//	return;
}




/* タスクトレイのアイコンに関する処理 */
BOOL CEditApp::TrayMessage( HWND hDlg, DWORD dwMessage, UINT uID, HICON hIcon, const char* pszTip )
{
	BOOL			res;
	NOTIFYICONDATA	tnd;
	tnd.cbSize				= sizeof( NOTIFYICONDATA );
	tnd.hWnd				= hDlg;
	tnd.uID					= uID;
	tnd.uFlags				= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	tnd.uCallbackMessage	= MYWM_NOTIFYICON;
	tnd.hIcon				= hIcon;
	if( pszTip ){
		lstrcpyn( tnd.szTip, pszTip, sizeof( tnd.szTip ) );
	}else{
		tnd.szTip[0] = '\0';
	}
	res = Shell_NotifyIcon( dwMessage, &tnd );
	if( hIcon ){
		DestroyIcon( hIcon );
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
	int				nId;
//	int				i;
//	int				j;
	HWND			hwndWork;
	//static CDlgGrep	cDlgGrep;  //Stonee, 2001/03/21 Grepを多重起動したときエラーになるのでGrep部分を別関数にした
	LPHELPINFO		lphi;
//	HWND			hwndExitingDlg;

//	CEditWnd*	pCEditWnd_Test;
//	char*		pszCmdLine;
	int			nRowNum;
	EditNode*	pEditNodeArr;
//	HWND		hwndNew;
	static HWND	hwndHtmlHelp;

	static WORD		wHotKeyMods;
	static WORD		wHotKeyCode;
//	UINT				idCtl;	/* コントロールのID */
	LPMEASUREITEMSTRUCT	lpmis;	/* 項目サイズ情報 */
	LPDRAWITEMSTRUCT	lpdis;	/* 項目描画情報 */
	int					nItemWidth;
	int					nItemHeight;
	static bool			bLDClick = false;	/* 左ダブルクリックをしたか 03/02/20 ai */

	switch ( uMsg ){
	case WM_MENUCHAR:
		/* メニューアクセスキー押下時の処理(WM_MENUCHAR処理) */
		return m_CMenuDrawer.OnMenuChar( hwnd, uMsg, wParam, lParam );
	case WM_DRAWITEM:
//		idCtl = (UINT) wParam;				/* コントロールのID */
		lpdis = (DRAWITEMSTRUCT*) lParam;	/* 項目描画情報 */
		switch( lpdis->CtlType ){
		case ODT_MENU:	/* オーナー描画メニュー */
			/* メニューアイテム描画 */
			m_CMenuDrawer.DrawItem( lpdis );
			return TRUE;
		}
		return FALSE;
	case WM_MEASUREITEM:
//		idCtl = (UINT) wParam;					// control identifier
		lpmis = (MEASUREITEMSTRUCT*) lParam;	// item-size information
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

	/* タスクトレイ左クリックメニューへのショートカットキー登録 */
	case WM_HOTKEY:
		{
			int		idHotKey = (int) wParam;				// identifier of hot key
			UINT	fuModifiers = (UINT) LOWORD(lParam);	// key-modifier flags
			UINT	uVirtKey = (UINT) HIWORD(lParam);		// virtual-key code
			char	szClassName[100];
			char	szText[256];

			hwndWork = ::GetForegroundWindow();
			szClassName[0] = '\0';
			::GetClassName( hwndWork, szClassName, sizeof( szClassName ) - 1 );
			::GetWindowText( hwndWork, szText, sizeof( szText ) - 1 );
			if( 0 == strcmp( szText, "共通設定" ) ){
				return -1;
			}

			if( ID_HOTKEY_TRAYMENU == idHotKey
			 &&	( wHotKeyMods )  == fuModifiers
			 && wHotKeyCode == uVirtKey
			){
				// Jan. 1, 2003 AROKA
				// タスクトレイメニューの表示タイミングをLBUTTONDOWN→LBUTTONUPに変更したことによる
				::PostMessage( m_hWnd, MYWM_NOTIFYICON, 0, WM_LBUTTONUP );
			}
		}
		return 0;

	case MYWM_HTMLHELP:
		{
			HH_AKLINK	link;
			char		szHtmlHelpFile[1024];
			int			nLen;
			int			nLenKey;
			char*		pszKey;
			strcpy( szHtmlHelpFile, m_pShareData->m_szWork );
			nLen = lstrlen( szHtmlHelpFile );
			nLenKey = lstrlen( &m_pShareData->m_szWork[nLen + 1] );
			pszKey = new char[ nLenKey + 1 ];
			strcpy( pszKey, &m_pShareData->m_szWork[nLen + 1] );

//			if( NULL != hwndHtmlHelp ){
//				::BringWindowToTop( hwndHtmlHelp );
//			}

			//	Jul. 6, 2001 genta HtmlHelpの呼び出し方法変更
			hwndHtmlHelp = OpenHtmlHelp(
				/*hwnd*/NULL/*hwndFrame*//*m_pShareData->m_hwndTray*/,
				szHtmlHelpFile,
				HH_DISPLAY_TOPIC,
				(DWORD)0,
				true
			);

			link.cbStruct		= sizeof(HH_AKLINK);
			link.fReserved		= FALSE;
			link.pszKeywords	= (char*)pszKey;
			link.pszUrl			= NULL;
			link.pszMsgText		= NULL;
			link.pszMsgTitle	= NULL;
			link.pszWindow		= NULL;
			link.fIndexOnFail	= TRUE;

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
//			if( hwndHtmlHelp != NULL ){
//				DWORD	dwPID;
//				DWORD	dwTID = ::GetWindowThreadProcessId( hwndHtmlHelp, &dwPID );
//				::AttachThreadInput( ::GetCurrentThreadId(), dwTID, TRUE );
//				::SetFocus( hwndHtmlHelp );
//				::AttachThreadInput( ::GetCurrentThreadId(), dwTID, FALSE );
//			}

			//	Jul. 6, 2001 genta HtmlHelpの呼び出し方法変更
			hwndHtmlHelp = OpenHtmlHelp(
				/*hwnd*/NULL/*hwndFrame*//*m_pShareData->m_hwndTray*/,
				szHtmlHelpFile,
				HH_KEYWORD_LOOKUP,
				(DWORD)&link,
				false
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
		 || FALSE == m_bCreatedTrayIcon						/* トレイにアイコンを作っていない */
		 ){
			/* 現在開いている編集窓のリスト */
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
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
//			rasconnstate	= (RASCONNSTATE) wParam;	// connection state about to be entered
//			dwError			= (DWORD) lParam;			// error that may have occurred
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

		/* タスクトレイ左クリックメニューへのショートカットキー登録 */
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
		case WM_COMMAND:
			OnCommand( HIWORD(wParam), LOWORD(wParam), (HWND) lParam );
			return 0L;

//		case MYWM_SETFILEINFO:
//			return 0L;
		case MYWM_CHANGESETTING:
			::UnregisterHotKey( m_hWnd, ID_HOTKEY_TRAYMENU );
			/* タスクトレイ左クリックメニューへのショートカットキー登録 */
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
				::MessageBox( NULL, "CEditApp::DispatchEvent()\nアクセラレータ テーブルが作成できません。\nシステムリソースが不足しています。", GSTR_APPNAME, MB_OK | MB_ICONSTOP );
			}

			return 0L;

		case MYWM_NOTIFYICON:
//			MYTRACE( "MYWM_NOTIFYICON\n" );
			switch (lParam){
//キーワード：トレイ右クリックメニュー設定
//	From Here Oct. 12, 2000 JEPRO 左右とも同一処理になっていたのを別々に処理するように変更
			case WM_RBUTTONUP:	// Dec. 24, 2002 towest UPに変更
				::SetActiveWindow( m_hWnd );
				::SetForegroundWindow( m_hWnd );
				/* ポップアップメニュー(トレイ右ボタン) */
				nId = CreatePopUpMenu_R();
				switch( nId ){
// Nov. 21, 2000 JEPROtestnow
				case F_FONT:
					/* フォント設定 */
					{
//						HWND	hwndFrame;
//						hwndFrame = ::GetParent( m_hwndParent );
						/* フォント設定ダイアログ */
//						if( m_pcEditDoc->SelectFont( &(m_pShareData->m_Common.m_lf) ) ){
							if( m_pShareData->m_Common.m_lf.lfPitchAndFamily & FIXED_PITCH ){
								m_pShareData->m_Common.m_bFontIs_FIXED_PITCH = TRUE;	/* 現在のフォントは固定幅フォントである */
							}else{
								m_pShareData->m_Common.m_bFontIs_FIXED_PITCH = FALSE;	/* 現在のフォントは固定幅フォントである */
							}
							/* 設定変更を反映させる */
							/* 全編集ウィンドウへメッセージをポストする */
//							m_cShareData.PostMessageToAllEditors(
//								MYWM_CHANGESETTING,
//								(WPARAM)0, (LPARAM)0, hwndFrame
//							);
//						}
					}
					break;
//				case F_OPTION:
					/* 共通設定 */
//					{
						/* 設定プロパティシート テスト用 */
//						m_pcEditDoc->bOpenPropertySheet( -1/*, -1*/ );
//					}
//					break;
//				case F_OPTION_TYPE:
					/* タイプ別設定 */
//					{
//						CEditDoc::OpenPropertySheetTypes( -1, m_nSettingType );
//					}
//					break;
//				case F_TYPE_LIST:
					/* タイプ別設定一覧 */
//					{
//						CDlgTypeList	cDlgTypeList;
//						int				nSettingType;
//						nSettingType = m_pcEditDoc->m_nSettingType;
//						if( cDlgTypeList.DoModal( m_hInstance, m_hWnd, &nSettingType ) ){
//							/* タイプ別設定 */
//							m_pcEditDoc->OpenPropertySheetTypes( -1, nSettingType );
//						}
//					}
//					break;
				case F_HELP_CONTENTS:
					/* ヘルプ目次 */
					{
						char	szHelp[_MAX_PATH + 1];
						/* ヘルプファイルのフルパスを返す */
						::GetHelpFilePath( szHelp );
						ShowWinHelpContents( m_hWnd, szHelp );	//	目次を表示する
					}
					break;
				case F_HELP_SEARCH:
					/* ヘルプキーワード検索 */
					{
						char	szHelp[_MAX_PATH + 1];
						/* ヘルプファイルのフルパスを返す */
						::GetHelpFilePath( szHelp );
						::WinHelp( m_hWnd, szHelp, HELP_KEY, (unsigned long)"" );
					}
					break;
//				case F_MENU_ALLFUNC:	//Jan. 12, 2001 JEPRO コマンド一覧は右クリックメニューから一応除外
//					/* コマンド一覧 */
//						CEditView::Command_MENU_ALLFUNC();
//					break;
				case F_EXTHELP1:
					/* 外部ヘルプ１ */
//					{
//						CEditView::Command_EXTHELP1();
//					}
//					break;
					do{
						if( CShareData::getInstance()->ExtWinHelpIsSet() ) {	//	共通設定のみ確認
//						if( 0 != strlen( m_pShareData->m_Common.m_szExtHelp1 ) ){
							break;
						}else
						{
							::MessageBeep( MB_ICONHAND );
						}
					}while(IDYES == ::MYMESSAGEBOX( 
							NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
							GSTR_APPNAME,
							"外部ヘルプ１が設定されていません。\n今すぐ設定しますか?")
					);/*do-while*/
#if 0
					{
					retry:;
						if( 0 == strlen( m_pShareData->m_Common.m_szExtHelp1 ) ){
							::MessageBeep( MB_ICONHAND );
							if( IDYES == ::MYMESSAGEBOX( NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
								"外部ヘルプ１が設定されていません。\n今すぐ設定しますか?"
							) ){
								/* 共通設定 プロパティシート */
//								if( !m_pcEditDoc->OpenPropertySheet( ID_PAGENUM_HELPER/*, IDC_EDIT_EXTHELP1*/ ) ){
//									break;
//								}
								goto retry;
							}
							else {
								break;
							}
						}

//						CMemory		cmemCurText;
						/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
//						GetCurrentTextForSearch( cmemCurText );
//						::WinHelp( m_hwndParent, m_pShareData->m_Common.m_szExtHelp1, HELP_KEY, (DWORD)(char*)cmemCurText.GetPtr() );
//						break;
					}
#endif
					break;
				case F_EXTHTMLHELP:
					/* 外部HTMLヘルプ */
					{
//						CEditView::Command_EXTHTMLHELP();
					}
					break;
				case F_ABOUT:
					/* バージョン情報 */
					{
						CDlgAbout cDlgAbout;
						cDlgAbout.DoModal( m_hInstance, m_hWnd );
					}
					break;
//				case IDM_EXITALL:
				case F_EXITALL:	//Dec. 26, 2000 JEPRO F_に変更
					/* サクラエディタの全終了 */
					CEditApp::TerminateApplication();
					break;
				}
				return 0L;
//	To Here Oct. 12, 2000

			case WM_LBUTTONDOWN:
				//	Mar. 29, 2003 genta 念のためフラグクリア
				bLDClick = false;
				return 0L;
			case WM_LBUTTONUP:	// Dec. 24, 2002 towest UPに変更
//				MYTRACE( "WM_LBUTTONDOWN\n" );
				/* 03/02/20 左ダブルクリック後はメニューを表示しない ai Start */
				if( bLDClick ){
					bLDClick = false;
					return 0L;
				}
				/* 03/02/20 ai End */
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
//						int				i;
//						int				j;
						char**			ppszMRU;
						char**			ppszOPENFOLDER;

						/* MRUリストのファイルのリスト */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUにすべて依頼する
						CMRU cMRU;
//						ppszMRU = NULL;
						ppszMRU = new char*[ cMRU.Length() + 1 ];
						cMRU.GetPathList(ppszMRU);

						/* OPENFOLDERリストのファイルのリスト */
//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
						CMRUFolder cMRUFolder;
//						ppszOPENFOLDER = NULL;
						ppszOPENFOLDER = new char*[ cMRUFolder.Length() + 1 ];
						cMRUFolder.GetPathList(ppszOPENFOLDER);

						/* ファイルオープンダイアログの初期化 */
						strcpy( szPath, "" );
						nCharCode = CODE_AUTODETECT;	/* 文字コード自動判別 */
						bReadOnly = FALSE;
						cDlgOpenFile.Create(
							m_hInstance,
							NULL/*m_hWnd*/,
							"*.*",
							ppszMRU[0],//@@@ 2001.12.26 YAZAKI m_fiMRUArrにはアクセスしない
							(const char **)ppszMRU,
							(const char **)ppszOPENFOLDER
						);
						if( !cDlgOpenFile.DoModalOpenDlg( szPath, &nCharCode, &bReadOnly ) ){
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
						if( CShareData::getInstance()->IsPathOpened( szPath, &hWndOwner ) ){
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
								char*	pszCodeNameCur = "";
								char*	pszCodeNameNew = "";
								// gm_pszCodeNameArr_1 を使うように変更 Moca. 2002/05/26
								if( -1 < pfi->m_nCharCode && pfi->m_nCharCode < CODE_CODEMAX ){
									pszCodeNameCur = (char *)gm_pszCodeNameArr_1[pfi->m_nCharCode];
								}
								if( -1 < nCharCode && nCharCode < CODE_CODEMAX ){
									pszCodeNameNew = (char *)gm_pszCodeNameArr_1[nCharCode];
								}
								::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
									"%s\n\n既に開いているファイルを違う文字コードで開く場合は、\n一旦閉じてからでないと開けません。\n\n現在の文字コードセット＝%s\n新しい文字コードセット＝%s",
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
				case F_GREP_DIALOG:
					/* Grep */
					DoGrep();  //Stonee, 2001/03/21  Grepを別関数に
					break;
				case F_WIN_CLOSEALL:	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
					/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
					CEditApp::CloseAllEditor();
					break;
//				case IDM_EXITALL:
				case F_EXITALL:	//Dec. 26, 2000 JEPRO F_に変更
					/* サクラエディタの全終了 */
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
//						nId - IDM_SELMRU  < (( m_pShareData->m_nMRUArrNum < m_pShareData->m_Common.m_nMRUArrNum_MAX )?m_pShareData->m_nMRUArrNum :m_pShareData->m_Common.m_nMRUArrNum_MAX )
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
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
						CMRU cMRU;
						FileInfo openFileInfo;
						cMRU.GetFileInfo(nId - IDM_SELMRU, &openFileInfo);

						if( m_pShareData->m_Common.GetRestoreCurPosition() ){
							CEditApp::OpenNewEditor2( m_hInstance, m_hWnd, &openFileInfo, FALSE );
						}
						else {
							CEditApp::OpenNewEditor( m_hInstance, m_hWnd,
								openFileInfo.m_szPath,
								openFileInfo.m_nCharCode,
								FALSE );

						}
						//	To Here Oct. 27, 2000 genta
					}else
					if( nId - IDM_SELOPENFOLDER  >= 0 &&
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
//							int				i;
//							int				j;
							char**			ppszMRU;
							char**			ppszOPENFOLDER;

							/* MRUリストのファイルのリスト */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
							CMRU cMRU;
							ppszMRU = NULL;
							ppszMRU = new char*[ cMRU.Length() + 1 ];
							cMRU.GetPathList(ppszMRU);
							/* OPENFOLDERリストのファイルのリスト */
//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
							CMRUFolder cMRUFolder;
//							ppszOPENFOLDER = NULL;
							ppszOPENFOLDER = new char*[ cMRUFolder.Length() + 1 ];
							cMRUFolder.GetPathList(ppszOPENFOLDER);

							//Stonee, 2001/12/21 UNCであれば接続を試みる
							NetConnect( cMRUFolder.GetPath( nId - IDM_SELOPENFOLDER ) );

							/* ファイルオープンダイアログの初期化 */
							strcpy( szPath, "" );
							nCharCode = CODE_AUTODETECT;	/* 文字コード自動判別 */
							bReadOnly = FALSE;
							cDlgOpenFile.Create(
								m_hInstance,
								NULL/*m_hWnd*/,
								"*.*",
								ppszOPENFOLDER[ nId - IDM_SELOPENFOLDER ],
								(const char **)ppszMRU,
								(const char **)ppszOPENFOLDER
							);
							if( !cDlgOpenFile.DoModalOpenDlg( szPath, &nCharCode, &bReadOnly ) ){
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
							if( CShareData::getInstance()->IsPathOpened( szPath, &hWndOwner ) ){
								::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
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
									char*	pszCodeNameCur = "";
									char*	pszCodeNameNew = "";
									// gm_pszCodeNameArr_1 を使うように変更 Moca. 2002/05/26
									if( -1 < pfi->m_nCharCode && pfi->m_nCharCode < CODE_CODEMAX ){
										pszCodeNameCur = (char*)gm_pszCodeNameArr_1[pfi->m_nCharCode];
									}
									if( -1 < nCharCode && nCharCode < CODE_CODEMAX ){
										pszCodeNameNew = (char*)gm_pszCodeNameArr_1[nCharCode];
									}
									::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
										"%s\n\n既に開いているファイルを違う文字コードで開く場合は、\n一旦閉じてからでないと開けません。\n\n現在の文字コードセット＝%s\n新しい文字コードセット＝%s",
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
				bLDClick = true;		/* 03/02/20 ai */
				/* 新規編集ウィンドウの追加 */
				OpenNewEditor( m_hInstance, m_hWnd, (char*)NULL, 0, FALSE );
				// Apr. 1, 2003 genta この後で表示されたメニューは閉じる
				::PostMessage( m_hWnd, WM_CANCELMODE, 0, 0 );
				return 0L;
			case WM_RBUTTONDBLCLK:
//				MYTRACE( "WM_RBUTTONDBLCLK\n" );
				return 0L;
			}
			break;

		case WM_QUERYENDSESSION:
			/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
			if( CloseAllEditor() ){
				//	Jan. 31, 2000 genta
				//	この時点ではWindowsの終了が確定していないので常駐解除すべきではない．
				//	DestroyWindow( hwnd );
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

		//	From Here Jan. 31, 2000 genta	Windows終了時の後処理．
		//	Windows終了時はWM_CLOSEが呼ばれない上，DestroyWindowを
		//	呼び出す必要もない．また，メッセージループに戻らないので
		//	メッセージループの後ろの処理をここで完了させる必要がある．
		case WM_ENDSESSION:
			//	もしWindowsの終了が中断されたのなら何もしない
			if( wParam != TRUE )	return 0;

			//	ホットキーの破棄
			::UnregisterHotKey( m_hWnd, ID_HOTKEY_TRAYMENU );

			//	どうせExplorerも終了するのでトレイアイコンは処理しない．

			//	終了処理中に新しいウィンドウを作るのもいやな感じなので
			//	オプションに関わらず終了ダイアログの表示は行わない

			//	共有データの保存(重要)
			CShareData::getInstance()->SaveShareData();

			return 0;	//	もうこのプロセスに制御が戻ることはない
		//	To Here Jan. 31, 2000 genta
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

			if( m_bCreatedTrayIcon ){	/* トレイにアイコンを作った */
				TrayMessage( hwnd, NIM_DELETE, 0, NULL, NULL );
			}
			m_hWnd = NULL;
			/* Windows にスレッドの終了を要求します。*/
			::PostQuitMessage( 0 );
			return 0L;
		default:
// << 20010412 by aroka
//	Apr. 24, 2001 genta RegisterWindowMessageを使うように修正
			if( uMsg == m_uCreateTaskBarMsg ){
				/* TaskTray Iconの再登録を要求するメッセージ．
					Explorerが再起動したときに送出される．*/
				CreateTrayIcon( m_hWnd ) ;
			}
			break;	/* default */
// >> by aroka
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




/*!	新規編集ウィンドウの追加 ver 0

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date Oct. 24, 2000 genta WinExec -> CreateProcess．同期機能を付加
*/
bool CEditApp::OpenNewEditor( HINSTANCE hInstance, HWND hWndParent, char* pszPath, int nCharCode, BOOL bReadOnly, bool sync )
{
	DLLSHAREDATA*	pShareData;
	char szCmdLineBuf[1024];	//	コマンドライン
	char szEXE[MAX_PATH + 1];	//	アプリケーションパス名
	int nPos = 0;				//	コマンドライン構築用ポインタ

	/* 共有データ構造体のアドレスを返す */
	pShareData = CShareData::getInstance()->GetShareData();

	/* 編集ウィンドウの上限チェック */
	if( pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
		char szMsg[512];
		wsprintf( szMsg, "編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。", MAX_EDITWINDOWS );
		::MessageBox( NULL, szMsg, GSTR_APPNAME, MB_OK );
		return false;
	}

	::GetModuleFileName( ::GetModuleHandle( NULL ), szEXE, sizeof( szEXE ) );
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
	//	読み取り専用指定がある場合		//From Here Feb. 26, 2001 JEPRO 追加 (direcited by genta)
	if( bReadOnly ){
		nPos += wsprintf( szCmdLineBuf + nPos, " -R" );
	}		//To Here Feb. 26, 2001
//: do error check nPos

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


/*!	新規編集ウィンドウの追加 ver 2:

	@date Oct. 24, 2000 genta create.
*/
bool CEditApp::OpenNewEditor2( HINSTANCE hInstance, HWND hWndParent, FileInfo* pfi, BOOL bReadOnly, bool sync )
{
	char			pszCmdLine[1024];
	DLLSHAREDATA*	pShareData;
	int				nPos = 0;		//	引数作成用ポインタ

	/* 共有データ構造体のアドレスを返す */
	pShareData = CShareData::getInstance()->GetShareData();

	/* 編集ウィンドウの上限チェック */
	if( pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
		char szMsg[512];
		wsprintf( szMsg, "編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。", MAX_EDITWINDOWS );
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
//: do error check nPos
	}
	//	::MessageBox( NULL, pszCmdLine, "OpenNewEditor", MB_OK );
	return OpenNewEditor( hInstance, hWndParent, pszCmdLine, CODE_AUTODETECT, bReadOnly, sync );

}
//	To Here Oct. 24, 2000 genta





/*!	サクラエディタの全終了

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
void CEditApp::TerminateApplication( void )
{
	DLLSHAREDATA*	pShareData;

	/* 共有データ構造体のアドレスを返す */
	pShareData = CShareData::getInstance()->GetShareData();

	/* 現在の編集ウィンドウの数を調べる */
	if( pShareData->m_Common.m_bExitConfirm ){	//終了時の確認
		if( 0 < CShareData::getInstance()->GetEditorWindowsNum() ){
			if( IDYES != ::MYMESSAGEBOX(
				NULL,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				"現在開いている編集用のウィンドウをすべて閉じて終了しますか?"
			) ){
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




/*!	すべてのウィンドウを閉じる
	
	@date Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
BOOL CEditApp::CloseAllEditor( void )
{
	/* 全編集ウィンドウへ終了要求を出す */
	if( !CShareData::getInstance()->RequestCloseAllEditor() ){
		return FALSE;
	}else{
		return TRUE;
	}
}




/*! ポップアップメニュー(トレイ左ボタン) */
int	CEditApp::CreatePopUpMenu_L( void )
{
	int			i;
	int			j;
	int			nId;
	HMENU		hMenuTop;
	HMENU		hMenu;
	HMENU		hMenuPopUp;
	char		szMemu[100 + MAX_PATH * 2];	//	Jan. 19, 2001 genta
	char		szMenu2[MAX_PATH * 2];	//	Jan. 19, 2001 genta
	POINT		po;
	RECT		rc;
//	HWND		hwndDummy;
	int			nMenuNum;
	FileInfo*	pfi;

	//本当はセマフォにしないとだめ
	if( m_bUseTrayMenu ) return -1;
	m_bUseTrayMenu = true;

	m_CMenuDrawer.ResetContents();
	CShareData::getInstance()->TransformFileName_MakeCash();

	hMenuTop = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_TRAYMENU_L ) );
	hMenu = ::GetSubMenu( hMenuTop, 0 );
	nMenuNum = ::GetMenuItemCount( hMenu )/* - 1*/;
	for( i = nMenuNum - 1; i >= 0; i-- ){
		::DeleteMenu( hMenu, i, MF_BYPOSITION );
	}

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, "新規作成(&N)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, "開く(&O)...", FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GREP_DIALOG, "&Grep...", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	/* MRUリストのファイルのリストをメニューにする */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	CMRU cMRU;
	hMenuPopUp = cMRU.CreateMenu( &m_CMenuDrawer );	//	ファイルメニュー
	if ( cMRU.Length() > 0 ){
		//	アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "最近使ったファイル(&F)" );
	}
	else {
		//	非アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , "最近使ったファイル(&F)" );
	}

	/* 最近使ったフォルダのメニューを作成 */
//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
	CMRUFolder cMRUFolder;
	hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
	if ( cMRUFolder.Length() > 0 ){
		//	アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, "最近使ったフォルダ(&D)" );
	}
	else {
		//	非アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp, "最近使ったフォルダ(&D)" );
	}

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_CLOSEALL, "すべてのウィンドウを閉じる(&Q)", FALSE );	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)	//Feb. 18, 2001 JEPRO アクセスキー変更(L→Q)


	/* 現在開いている編集窓のリストをメニューにする */
	j = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( CShareData::IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			++j;
		}
	}

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
//						int			nDesLen;
//						const char*	pszDes;
						LimitStringLengthB( pfi->m_szGrepKey, lstrlen( pfi->m_szGrepKey ), 64, cmemDes );
						//	Jan. 19, 2002 genta
						//	メニュー文字列の&を考慮
						dupamp( cmemDes.GetPtr(), szMenu2 );
//	From Here Oct. 4, 2000 JEPRO commented out & modified
//		j >= 10 + 26 の時の考慮を省いた(に近い)が開くファイル数が36個を越えることはまずないので事実上OKでしょう
						//	Jan. 19, 2002 genta
						//	&の重複処理を追加したため継続判定を若干変更
						wsprintf( szMemu, "&%c 【Grep】\"%s%s\"", ((1 + i) <= 9)?('1' + i):('A' + i - 9),
							szMenu2, ( (int)lstrlen( pfi->m_szGrepKey ) > cmemDes.GetLength() ) ? "…":""
						);
					}else{
						char szFileName[_MAX_PATH];
						// 2003/01/27 Moca ファイル名の簡易表示
						CShareData::getInstance()->GetTransformFileNameFast( pfi->m_szPath, szFileName, MAX_PATH );
						//	Jan. 19, 2002 genta
						//	メニュー文字列の&を考慮
						dupamp( szFileName, szMenu2 );
						wsprintf( szMemu, "&%c %s %s", ((1 + i) <= 9)?('1' + i):('A' + i - 9),
							(0 < lstrlen( szMenu2 ))? szMenu2:"（無題）",
							pfi->m_bIsModified ? "*":" "
						);
//		To Here Oct. 4, 2000
						// gm_pszCodeNameArr_3 からコピーするように変更
						if( 0 < pfi->m_nCharCode && pfi->m_nCharCode < CODE_CODEMAX ){
							strcat( szMemu, gm_pszCodeNameArr_3[pfi->m_nCharCode] );
						}
					}

//				::InsertMenu( hMenu, IDM_EXITALL, MF_BYCOMMAND | MF_STRING, IDM_SELWINDOW + i, szMemu );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + i, szMemu, FALSE );
				++j;
			}
		}
	}
	if( j == 0 ){
		::EnableMenuItem( hMenu, F_WIN_CLOSEALL, MF_BYCOMMAND | MF_GRAYED );	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
	}

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	//	Jun. 9, 2001 genta ソフトウェア名改称
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, "サクラエディタの全終了(&X)", FALSE );	//Dec. 26, 2000 JEPRO F_に変更

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

	m_bUseTrayMenu = false;

	return nId;
}

//キーワード：トレイ右クリックメニュー順序
//	Oct. 12, 2000 JEPRO ポップアップメニュー(トレイ左ボタン) を参考にして新たに追加した部分

/*! ポップアップメニュー(トレイ右ボタン) */
int	CEditApp::CreatePopUpMenu_R( void )
{
	int		i;
	int		nId;
	HMENU	hMenuTop;
	HMENU	hMenu;
	POINT	po;
	RECT	rc;
	int		nMenuNum;

	//本当はセマフォにしないとだめ
	if( m_bUseTrayMenu ) return -1;
	m_bUseTrayMenu = true;

	m_CMenuDrawer.ResetContents();

	hMenuTop = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_TRAYMENU_L ) );
	hMenu = ::GetSubMenu( hMenuTop, 0 );
	nMenuNum = ::GetMenuItemCount( hMenu )/* - 1*/;
	for( i = nMenuNum - 1; i >= 0; i-- ){
		::DeleteMenu( hMenu, i, MF_BYPOSITION );
	}

#if 0
	2002/04/26 YAZAKI 使えないものは表示しない

	/* トレイ右クリックの「オプション」メニュー */
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_TYPE_LIST, "タイプ別設定一覧(&L)...", FALSE );	//Jan. 12, 2001 JEPRO このメニュー項目を無効化した
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_OPTION_TYPE, "タイプ別設定(&Y)...", FALSE );	//Jan. 12, 2001 JEPRO このメニュー項目を無効化した
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_OPTION, "共通設定(&C)...", FALSE );				//Jan. 12, 2001 JEPRO このメニュー項目を無効化した
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_FONT, "フォント設定(&F)...", FALSE );			//Jan. 12, 2001 JEPRO このメニュー項目を無効化した
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
#endif

	/* トレイ右クリックの「ヘルプ」メニュー */
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , "ヘルプ目次(&O)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH , "ヘルプキーワード検索(&S)", FALSE );	//Nov. 25, 2000 JEPRO 「トピックの」→「キーワード」に変更
//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MENU_ALLFUNC , "コマンド一覧(&M)", FALSE );	//Jan. 12, 2001 JEPRO まずコメントアウト第一号 (T_T)
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

#if 0
	2002/04/26 YAZAKI 使えないものは表示しない

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_EXTHELP1 , "外部ヘルプ１(&E)", FALSE );			//Jan. 12, 2001 JEPRO このメニュー項目を無効化した
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_EXTHTMLHELP , "外部HTMLヘルプ(&H)", FALSE );	//Jan. 12, 2001 JEPRO このメニュー項目を無効化した
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
#endif

//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_ABOUT, "バージョン情報(&A)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ABOUT, "バージョン情報(&A)", FALSE );	//Dec. 25, 2000 JEPRO F_に変更

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_EXITALL, "テキストエディタの全終了(&X)", FALSE );
//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, "テキストエディタの全終了(&X)", FALSE );	//Dec. 26, 2000 JEPRO F_に変更
	//	Jun. 18, 2001 genta ソフトウェア名改称
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, "サクラエディタの全終了(&X)", FALSE );	//De

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

	m_bUseTrayMenu = false;

	return nId;
}

/*[EOF]*/
