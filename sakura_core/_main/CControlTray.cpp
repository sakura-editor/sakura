/*!	@file
	@brief 常駐部
	
	タスクトレイアイコンの管理，タスクトレイメニューのアクション，
	MRU、キー割り当て、共通設定、編集ウィンドウの管理など

	@author Norio Nakatani
	@date 1998/05/13 新規作成
	@date 2001/06/03 N.Nakatani grep単語単位で検索を実装するときのためにコマンドラインオプションの処理追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, Stonee, jepro, genta, aroka, hor, YAZAKI
	Copyright (C) 2002, MIK, Moca, genta, YAZAKI, towest
	Copyright (C) 2003, MIK, Moca, KEITA, genta, aroka
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#define ID_HOTKEY_TRAYMENU	0x1234

#include <HtmlHelp.h>
#include "CControlTray.h"
#include "CEditApp.h"
#include "debug/CRunningTimer.h"
#include "window/CEditWnd.h"		//Nov. 21, 2000 JEPROtest
#include "dlg/CDlgAbout.h"		//Nov. 21, 2000 JEPROtest
#include "io/CTextStream.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/string_ex2.h"
#include "env/CShareData.h"
#include "env/CShareData_IO.h"
#include "env/CSakuraEnvironment.h"
#include "env/CHelpManager.h"
#include "doc/CDocListener.h" // SLoadInfo,EditInfo
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"
#include "sakura_rc.h"

/////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK CEditAppWndProc( HWND, UINT, WPARAM, LPARAM );


CControlTray*	g_m_pCEditApp;

//Stonee, 2001/03/21
//Stonee, 2001/07/01  多重起動された場合は前回のダイアログを前面に出すようにした。
void CControlTray::DoGrep()
{
	//Stonee, 2001/06/30
	//前回のダイアログがあれば前面に (suggested by genta)
	if ( ::IsWindow(m_cDlgGrep.GetHwnd()) ){
		::OpenIcon(m_cDlgGrep.GetHwnd());
		::BringWindowToTop(m_cDlgGrep.GetHwnd());
		return;
	}

	if( 0 < m_pShareData->m_sSearchKeywords.m_aSearchKeys.size()
		&& m_nCurSearchKeySequence < GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence ){
		m_cDlgGrep.m_strText = m_pShareData->m_sSearchKeywords.m_aSearchKeys[0];
	}
	if( 0 < m_pShareData->m_sSearchKeywords.m_aGrepFiles.size() ){
		_tcscpy( m_cDlgGrep.m_szFile, m_pShareData->m_sSearchKeywords.m_aGrepFiles[0] );		/* 検索ファイル */
	}
	if( 0 < m_pShareData->m_sSearchKeywords.m_aGrepFolders.size() ){
		_tcscpy( m_cDlgGrep.m_szFolder, m_pShareData->m_sSearchKeywords.m_aGrepFolders[0] );	/* 検索フォルダ */
	}

	/* Grepダイアログの表示 */
	int nRet = m_cDlgGrep.DoModal( m_hInstance, NULL, _T("") );
	if( !nRet || GetTrayHwnd() == NULL ){
		return;
	}
	m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;
	DoGrepCreateWindow(m_hInstance, GetDllShareData().m_sHandles.m_hwndTray, m_cDlgGrep);
}

void CControlTray::DoGrepCreateWindow(HINSTANCE hinst, HWND msgParent, CDlgGrep& cDlgGrep)
{

	/*======= Grepの実行 =============*/
	/* Grep結果ウィンドウの表示 */

	CNativeW		cmWork1;
	CNativeT		cmWork2;
	CNativeT		cmWork3;
	cmWork1.SetString( cDlgGrep.m_strText.c_str() );
	cmWork2.SetString( cDlgGrep.m_szFile );
	cmWork3.SetString( cDlgGrep.m_szFolder );
	cmWork1.Replace( L"\"", L"\"\"" );
	cmWork2.Replace( _T("\""), _T("\"\"") );
	cmWork3.Replace( _T("\""), _T("\"\"") );

	// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
	CNativeT cCmdLine;
	TCHAR szTemp[20];
	cCmdLine.AppendString(_T("-GREPMODE -GKEY=\""));
	cCmdLine.AppendStringW(cmWork1.GetStringPtr());
	cCmdLine.AppendString(_T("\" -GFILE=\""));
	cCmdLine.AppendString(cmWork2.GetStringPtr());
	cCmdLine.AppendString(_T("\" -GFOLDER=\""));
	cCmdLine.AppendString(cmWork3.GetStringPtr());
	cCmdLine.AppendString(_T("\" -GCODE="));
	auto_sprintf( szTemp, _T("%d"), cDlgGrep.m_nGrepCharSet );
	cCmdLine.AppendString(szTemp);

	//GOPTオプション
	TCHAR pOpt[64] = _T("");
	if( cDlgGrep.m_bSubFolder					)_tcscat( pOpt, _T("S") );	// サブフォルダからも検索する
	if( cDlgGrep.m_sSearchOption.bLoHiCase		)_tcscat( pOpt, _T("L") );	// 英大文字と英小文字を区別する
	if( cDlgGrep.m_sSearchOption.bRegularExp	)_tcscat( pOpt, _T("R") );	// 正規表現
	if( cDlgGrep.m_bGrepOutputLine				)_tcscat( pOpt, _T("P") );	// 行を出力するか該当部分だけ出力するか
	if( cDlgGrep.m_sSearchOption.bWordOnly		)_tcscat( pOpt, _T("W") );	// 単語単位で探す
	if( 1 == cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("1") );	// Grep: 出力形式
	if( 2 == cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("2") );	// Grep: 出力形式
	if( pOpt[0] != _T('\0') ){
		cCmdLine.AppendString( _T(" -GOPT=") );
		cCmdLine.AppendString( pOpt );
	}

	/* 新規編集ウィンドウの追加 ver 0 */
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = _T("");
	sLoadInfo.eCharCode = CODE_NONE;
	sLoadInfo.bViewMode = false;
	OpenNewEditor( hinst, msgParent, sLoadInfo, cCmdLine.GetStringPtr(),
		false, NULL, GetDllShareData().m_Common.m_sTabBar.m_bNewWindow? true : false );
}


/* ウィンドウプロシージャじゃ */
LRESULT CALLBACK CEditAppWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	CControlTray* pSApp;
	switch( uMsg ){
	case WM_CREATE:
		pSApp = ( CControlTray* )g_m_pCEditApp;
		return pSApp->DispatchEvent( hwnd, uMsg, wParam, lParam );
	default:
		// Modified by KEITA for WIN64 2003.9.6
		//RELPRINT( _T("dispatch\n") );
		pSApp = ( CControlTray* )::GetWindowLongPtr( hwnd, GWLP_USERDATA );
		if( NULL != pSApp ){
			return pSApp->DispatchEvent( hwnd, uMsg, wParam, lParam );
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}




/////////////////////////////////////////////////////////////////////////////
// CControlTray
//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CControlTray::CControlTray()
//	Apr. 24, 2001 genta
: m_uCreateTaskBarMsg( ::RegisterWindowMessage( TEXT("TaskbarCreated") ) )
, m_bCreatedTrayIcon( FALSE )	//トレイにアイコンを作った
, m_hInstance( NULL )
, m_hWnd( NULL )
, m_nCurSearchKeySequence(-1)
{
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();

	// アクセラレータテーブル作成
	CreateAccelTbl();

	m_bUseTrayMenu = false;

	return;
}


CControlTray::~CControlTray()
{
	return;
}

/////////////////////////////////////////////////////////////////////////////
// CControlTray メンバ関数




/* 作成 */
HWND CControlTray::Create( HINSTANCE hInstance )
{
	MY_RUNNINGTIMER( cRunningTimer, "CControlTray::Create" );

	//同名同クラスのウィンドウが既に存在していたら、失敗
	m_hInstance = hInstance;
	HWND hwndWork = ::FindWindow( GSTR_CEDITAPP, GSTR_CEDITAPP );
	if( NULL != hwndWork ){
		return NULL;
	}

	//ウィンドウクラス登録
	WNDCLASS	wc;
	{
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
		wc.lpszClassName	= GSTR_CEDITAPP;
		ATOM	atom = RegisterClass( &wc );
		if( 0 == atom ){
			ErrorMessage( NULL, _T("CControlTray::Create()\nウィンドウクラスを登録できませんでした。") );
		}
	}
	g_m_pCEditApp = this;

	// ウィンドウ作成 (WM_CREATEで、GetHwnd() に HWND が格納される)
	::CreateWindow(
		GSTR_CEDITAPP,						// pointer to registered class name
		GSTR_CEDITAPP,						// pointer to window name
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

	// 最前面にする（トレイからのポップアップウィンドウが最前面になるように）
	::SetWindowPos( GetTrayHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
	
	// タスクトレイアイコン作成
	m_hIcons.Create( m_hInstance );	//	Oct. 16, 2000 genta
	m_CMenuDrawer.Create( m_hInstance, GetTrayHwnd(), &m_hIcons );
	if( GetTrayHwnd() ){
		CreateTrayIcon( GetTrayHwnd() );
	}

	return GetTrayHwnd();
}

//! タスクトレイにアイコンを登録する
bool CControlTray::CreateTrayIcon( HWND hWnd )
{
	// タスクトレイのアイコンを作る
	if( TRUE == m_pShareData->m_Common.m_sGeneral.m_bUseTaskTray ){	/* タスクトレイのアイコンを使う */
		//	Dec. 02, 2002 genta
		HICON hIcon = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
//From Here Jan. 12, 2001 JEPRO トレイアイコンにポイントするとバージョンno.が表示されるように修正
//			TrayMessage( GetTrayHwnd(), NIM_ADD, 0,  hIcon, GSTR_APPNAME );
		/* バージョン情報 */
		//	UR version no.を設定 (cf. cDlgAbout.cpp)
		TCHAR	pszTips[64];
		//	2004.05.13 Moca バージョン番号は、プロセスごとに取得する
		DWORD dwVersionMS, dwVersionLS;
		GetAppVersionInfo( NULL, VS_VERSION_INFO,
			&dwVersionMS, &dwVersionLS );

		auto_snprintf_s( pszTips, _countof(pszTips), _T("%ts %d.%d.%d.%d"),		//Jul. 06, 2001 jepro UR はもう付けなくなったのを忘れていた
			GSTR_APPNAME,
			HIWORD( dwVersionMS ),
			LOWORD( dwVersionMS ),
			HIWORD( dwVersionLS ),
			LOWORD( dwVersionLS )
		);
		TrayMessage( GetTrayHwnd(), NIM_ADD, 0,  hIcon, pszTips );
//To Here Jan. 12, 2001
		m_bCreatedTrayIcon = TRUE;	/* トレイにアイコンを作った */
	}
	return true;
}




/* メッセージループ */
void CControlTray::MessageLoop( void )
{
//複数プロセス版
	MSG	msg;
	int ret;
	
	//2004.02.17 Moca GetMessageのエラーチェック
	while ( GetTrayHwnd() != NULL && (ret = ::GetMessage(&msg, NULL, 0, 0 )) != 0 ){
		if( ret == -1 ){
			break;
		}
		::TranslateMessage( &msg );
		::DispatchMessage( &msg );
	}
	return;

}




/* タスクトレイのアイコンに関する処理 */
BOOL CControlTray::TrayMessage( HWND hDlg, DWORD dwMessage, UINT uID, HICON hIcon, const TCHAR* pszTip )
{
	BOOL			res;
	NOTIFYICONDATA	tnd;
	tnd.cbSize				= sizeof_raw( tnd );
	tnd.hWnd				= hDlg;
	tnd.uID					= uID;
	tnd.uFlags				= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	tnd.uCallbackMessage	= MYWM_NOTIFYICON;
	tnd.hIcon				= hIcon;
	if( pszTip ){
		lstrcpyn( tnd.szTip, pszTip, _countof( tnd.szTip ) );
	}else{
		tnd.szTip[0] = _T('\0');
	}
	res = Shell_NotifyIcon( dwMessage, &tnd );
	if( hIcon ){
		DestroyIcon( hIcon );
	}
	return res;
}





/* メッセージ処理 */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
LRESULT CControlTray::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	int				nId;
	HWND			hwndWork;
	LPHELPINFO		lphi;

	int			nRowNum;
	EditNode*	pEditNodeArr;
	static HWND	hwndHtmlHelp;

	static WORD		wHotKeyMods;
	static WORD		wHotKeyCode;
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
		lpdis = (DRAWITEMSTRUCT*) lParam;	/* 項目描画情報 */
		switch( lpdis->CtlType ){
		case ODT_MENU:	/* オーナー描画メニュー */
			/* メニューアイテム描画 */
			m_CMenuDrawer.DrawItem( lpdis );
			return TRUE;
		}
		return FALSE;
	case WM_MEASUREITEM:
		lpmis = (MEASUREITEMSTRUCT*) lParam;	// item-size information
		switch( lpmis->CtlType ){
		case ODT_MENU:	/* オーナー描画メニュー */
			/* メニューアイテムの描画サイズを計算 */
			nItemWidth = m_CMenuDrawer.MeasureItem( lpmis->itemID, &nItemHeight );
			if( 0 < nItemWidth ){
				lpmis->itemWidth = nItemWidth;
				lpmis->itemHeight = nItemHeight;
			}
			return TRUE;
		}
		return FALSE;
	case WM_EXITMENULOOP:
		m_CMenuDrawer.EndDrawMenu();
		break;


	/* タスクトレイ左クリックメニューへのショートカットキー登録 */
	case WM_HOTKEY:
		{
			int		idHotKey = (int) wParam;				// identifier of hot key
			UINT	fuModifiers = (UINT) LOWORD(lParam);	// key-modifier flags
			UINT	uVirtKey = (UINT) HIWORD(lParam);		// virtual-key code
			TCHAR	szClassName[100];
			TCHAR	szText[256];

			hwndWork = ::GetForegroundWindow();
			szClassName[0] = L'\0';
			::GetClassName( hwndWork, szClassName, _countof( szClassName ) - 1 );
			::GetWindowText( hwndWork, szText, _countof( szText ) - 1 );
			if( 0 == _tcscmp( szText, _T("共通設定") ) ){
				return -1;
			}

			if( ID_HOTKEY_TRAYMENU == idHotKey
			 &&	( wHotKeyMods )  == fuModifiers
			 && wHotKeyCode == uVirtKey
			){
				// Jan. 1, 2003 AROKA
				// タスクトレイメニューの表示タイミングをLBUTTONDOWN→LBUTTONUPに変更したことによる
				::PostMessageAny( GetTrayHwnd(), MYWM_NOTIFYICON, 0, WM_LBUTTONUP );
			}
		}
		return 0;

	case MYWM_UIPI_CHECK:
		/* エディタ－トレイ間でのUI特権分離の確認メッセージ */	// 2007.06.07 ryoji
		::SendMessage( (HWND)lParam, MYWM_UIPI_CHECK,  (WPARAM)0, (LPARAM)0 );	// 返事を返す
		return 0L;

	case MYWM_HTMLHELP:
		{
			TCHAR* pWork = m_pShareData->m_sWorkBuffer.GetWorkBuffer<TCHAR>();

			//szHtmlFile取得
			TCHAR	szHtmlHelpFile[1024];
			_tcscpy( szHtmlHelpFile, pWork );
			int		nLen = _tcslen( szHtmlHelpFile );

			//	Jul. 6, 2001 genta HtmlHelpの呼び出し方法変更
			hwndHtmlHelp = OpenHtmlHelp(
				NULL,
				szHtmlHelpFile,
				HH_DISPLAY_TOPIC,
				(DWORD_PTR)0,
				true
			);

			HH_AKLINK	link;
			link.cbStruct		= sizeof_raw(link);
			link.fReserved		= FALSE;
			link.pszKeywords	= to_tchar(&pWork[nLen+1]);
			link.pszUrl			= NULL;
			link.pszMsgText		= NULL;
			link.pszMsgTitle	= NULL;
			link.pszWindow		= NULL;
			link.fIndexOnFail	= TRUE;

			//	Jul. 6, 2001 genta HtmlHelpの呼び出し方法変更
			hwndHtmlHelp = OpenHtmlHelp(
				NULL,
				szHtmlHelpFile,
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link,
				false
			);
		}
		return (LRESULT)hwndHtmlHelp;


	/* 編集ウィンドウオブジェクトからのオブジェクト削除要求 */
	case MYWM_DELETE_ME:
		// タスクトレイのアイコンを常駐しない、または、トレイにアイコンを作っていない
		if( !(m_pShareData->m_Common.m_sGeneral.m_bStayTaskTray && m_pShareData->m_Common.m_sGeneral.m_bUseTaskTray) || !m_bCreatedTrayIcon ){
			// 現在開いている編集窓のリスト
			nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if( 0 < nRowNum ){
				delete [] pEditNodeArr;
			}
			// 編集ウィンドウの数が0になったら終了
			if( 0 == nRowNum ){
				::SendMessage( hwnd, WM_CLOSE, 0, 0 );
			}
		}
		return 0;

	case WM_CREATE:
		m_hWnd = hwnd;
		hwndHtmlHelp = NULL;
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( GetTrayHwnd(), GWLP_USERDATA, (LONG_PTR)this );

		/* タスクトレイ左クリックメニューへのショートカットキー登録 */
		wHotKeyMods = 0;
		if( HOTKEYF_SHIFT & m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods ){
			wHotKeyMods |= MOD_SHIFT;
		}
		if( HOTKEYF_CONTROL & m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods ){
			wHotKeyMods |= MOD_CONTROL;
		}
		if( HOTKEYF_ALT & m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods ){
			wHotKeyMods |= MOD_ALT;
		}
		wHotKeyCode = m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyCode;
		::RegisterHotKey(
			GetTrayHwnd(),
			ID_HOTKEY_TRAYMENU,
			wHotKeyMods,
			wHotKeyCode
		);

		// 2006.07.09 ryoji 最後の方でシャットダウンするアプリケーションにする
		BOOL (WINAPI *pfnSetProcessShutdownParameters)( DWORD dwLevel, DWORD dwFlags );
		HINSTANCE hDll;
		hDll = ::GetModuleHandle(_T("KERNEL32"));
		if( NULL != hDll ){
			*(FARPROC*)&pfnSetProcessShutdownParameters = ::GetProcAddress( hDll, "SetProcessShutdownParameters" );
			if( NULL != pfnSetProcessShutdownParameters ){
				pfnSetProcessShutdownParameters( 0x180, 0 );
			}
		}
		return 0L;

//	case WM_QUERYENDSESSION:
	case WM_HELP:
		lphi = (LPHELPINFO) lParam;
		switch( lphi->iContextType ){
		case HELPINFO_MENUITEM:
			MyWinHelp( hwnd, HELP_CONTEXT, FuncID_To_HelpContextID( (EFunctionCode)lphi->iCtrlId ) );
			break;
		}
		return TRUE;
		case WM_COMMAND:
			OnCommand( HIWORD(wParam), LOWORD(wParam), (HWND) lParam );
			return 0L;

//		case MYWM_SETFILEINFO:
//			return 0L;
		case MYWM_CHANGESETTING:
			switch( (e_PM_CHANGESETTING_SELECT)lParam ){
			case PM_CHANGESETTING_ALL:
				::UnregisterHotKey( GetTrayHwnd(), ID_HOTKEY_TRAYMENU );
				/* タスクトレイ左クリックメニューへのショートカットキー登録 */
				wHotKeyMods = 0;
				if( HOTKEYF_SHIFT & m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods ){
					wHotKeyMods |= MOD_SHIFT;
				}
				if( HOTKEYF_CONTROL & m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods ){
					wHotKeyMods |= MOD_CONTROL;
				}
				if( HOTKEYF_ALT & m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods ){
					wHotKeyMods |= MOD_ALT;
				}
				wHotKeyCode = m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyCode;
				::RegisterHotKey(
					GetTrayHwnd(),
					ID_HOTKEY_TRAYMENU,
					wHotKeyMods,
					wHotKeyCode
				);

//@@			/* 共有データの保存 */
//@@			m_cShareData.SaveShareData();

				/* アクセラレータテーブルの再作成 */
				// アクセラレータテーブル破棄
				DeleteAccelTbl();
				// アクセラレータテーブル作成
				CreateAccelTbl();
				break;
			default:
				break;
			}
			return 0L;

		case MYWM_NOTIFYICON:
//			MYTRACE( _T("MYWM_NOTIFYICON\n") );
			switch (lParam){
//キーワード：トレイ右クリックメニュー設定
//	From Here Oct. 12, 2000 JEPRO 左右とも同一処理になっていたのを別々に処理するように変更
			case WM_RBUTTONUP:	// Dec. 24, 2002 towest UPに変更
				::SetActiveWindow( GetTrayHwnd() );
				::SetForegroundWindow( GetTrayHwnd() );
				/* ポップアップメニュー(トレイ右ボタン) */
				nId = CreatePopUpMenu_R();
				switch( nId ){
				case F_HELP_CONTENTS:
					/* ヘルプ目次 */
					ShowWinHelpContents( GetTrayHwnd() );	//	目次を表示する
					break;
				case F_HELP_SEARCH:
					/* ヘルプキーワード検索 */
					MyWinHelp( GetTrayHwnd(), HELP_KEY, (ULONG_PTR)_T("") );	// 2006.10.10 ryoji MyWinHelpに変更に変更
					break;
				case F_EXTHELP1:
					/* 外部ヘルプ１ */
					do{
						if( CHelpManager().ExtWinHelpIsSet() ) {	//	共通設定のみ確認
							break;
						}
						else{
							ErrorBeep();
						}
					}while(IDYES == ::MYMESSAGEBOX( 
							NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
							GSTR_APPNAME,
							_T("外部ヘルプ１が設定されていません。\n今すぐ設定しますか?"))
					);/*do-while*/

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
						cDlgAbout.DoModal( m_hInstance, GetTrayHwnd() );
					}
					break;
//				case IDM_EXITALL:
				case F_EXITALL:	//Dec. 26, 2000 JEPRO F_に変更
					/* サクラエディタの全終了 */
					CControlTray::TerminateApplication( GetTrayHwnd() );	// 2006.12.25 ryoji 引数追加
					break;
				default:
					break;
				}
				return 0L;
//	To Here Oct. 12, 2000

			case WM_LBUTTONDOWN:
				//	Mar. 29, 2003 genta 念のためフラグクリア
				bLDClick = false;
				return 0L;
			case WM_LBUTTONUP:	// Dec. 24, 2002 towest UPに変更
//				MYTRACE( _T("WM_LBUTTONDOWN\n") );
				/* 03/02/20 左ダブルクリック後はメニューを表示しない ai Start */
				if( bLDClick ){
					bLDClick = false;
					return 0L;
				}
				/* 03/02/20 ai End */
				::SetActiveWindow( GetTrayHwnd() );
				::SetForegroundWindow( GetTrayHwnd() );
				/* ポップアップメニュー(トレイ左ボタン) */
				nId = CreatePopUpMenu_L();
				switch( nId ){
				case F_FILENEW:	/* 新規作成 */
					/* 新規編集ウィンドウの追加 */
					OnNewEditor( false );
					break;
				case F_FILEOPEN:	/* 開く */
					{
						CDlgOpenFile	cDlgOpenFile;

						// MRUリストのファイルのリスト
						const CMRUFile cMRU;
						std::vector<LPCTSTR> vMRU = cMRU.GetPathList();

						// ファイルオープンダイアログの初期化
						SLoadInfo sLoadInfo;
						sLoadInfo.cFilePath = _T("");
						sLoadInfo.eCharCode = CODE_AUTODETECT;	// 文字コード自動判別
						sLoadInfo.bViewMode = false;
						cDlgOpenFile.Create(
							m_hInstance,
							NULL,
							_T("*.*"),
							vMRU.empty()? NULL: vMRU[0],//@@@ 2001.12.26 YAZAKI m_fiMRUArrにはアクセスしない
							vMRU,
							CMRUFolder().GetPathList()	// OPENFOLDERリストのファイルのリスト
						);
						std::vector<std::tstring> files;
						if( !cDlgOpenFile.DoModalOpenDlg( &sLoadInfo, &files ) ){
							break;
						}
						if( NULL == GetTrayHwnd() ){
							break;
						}
						
						// 新たな編集ウィンドウを起動
						size_t nSize = files.size();
						for( size_t f = 0; f < nSize; f++ ){
							sLoadInfo.cFilePath = files[f].c_str();
							CControlTray::OpenNewEditor( m_hInstance, GetTrayHwnd(), sLoadInfo,
								NULL, true, NULL, m_pShareData->m_Common.m_sTabBar.m_bNewWindow? true : false );
						}
					}
					break;
				case F_GREP_DIALOG:
					/* Grep */
					DoGrep();  //Stonee, 2001/03/21  Grepを別関数に
					break;
				case F_FILESAVEALL:	// Jan. 24, 2005 genta 全て上書き保存
					CAppNodeGroupHandle(0).PostMessageToAllEditors(
						WM_COMMAND,
						MAKELONG( F_FILESAVE_QUIET, 0 ),
						(LPARAM)0,
						NULL
					);
					break;
				case F_EXITALLEDITORS:	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)	// 2007.02.13 ryoji →F_EXITALLEDITORS
					/* 編集の全終了 */
					CControlTray::CloseAllEditor( TRUE, GetTrayHwnd(), TRUE, 0 );	// 2006.12.25, 2007.02.13 ryoji 引数追加
					break;
				case F_EXITALL:	//Dec. 26, 2000 JEPRO F_に変更
					/* サクラエディタの全終了 */
					CControlTray::TerminateApplication( GetTrayHwnd() );	// 2006.12.25 ryoji 引数追加
					break;
				default:
					if( nId - IDM_SELWINDOW  >= 0 && nId - IDM_SELWINDOW  < m_pShareData->m_sNodes.m_nEditArrNum ){
						hwndWork = m_pShareData->m_sNodes.m_pEditArr[nId - IDM_SELWINDOW].GetHwnd();

						/* アクティブにする */
						ActivateFrameWindow( hwndWork );
					}
					else if( nId-IDM_SELMRU >= 0 && nId-IDM_SELMRU < 999 ){

						/* 新しい編集ウィンドウを開く */
						//	From Here Oct. 27, 2000 genta	カーソル位置を復元しない機能
						const CMRUFile cMRU;
						EditInfo openEditInfo;
						cMRU.GetEditInfo(nId - IDM_SELMRU, &openEditInfo);

						if( m_pShareData->m_Common.m_sFile.GetRestoreCurPosition() ){
							CControlTray::OpenNewEditor2( m_hInstance, GetTrayHwnd(), &openEditInfo, false );
						}
						else {
							SLoadInfo sLoadInfo;
							sLoadInfo.cFilePath = openEditInfo.m_szPath;
							sLoadInfo.eCharCode = openEditInfo.m_nCharCode;
							sLoadInfo.bViewMode = false;
							CControlTray::OpenNewEditor(
								m_hInstance,
								GetTrayHwnd(),
								sLoadInfo,
								NULL,
								false,
								NULL,
								m_pShareData->m_Common.m_sTabBar.m_bNewWindow? true : false
							);

						}
						//	To Here Oct. 27, 2000 genta
					}
					else if( nId - IDM_SELOPENFOLDER  >= 0 && nId - IDM_SELOPENFOLDER  < 999 ){
						/* MRUリストのファイルのリスト */
						const CMRUFile cMRU;
						std::vector<LPCTSTR> vMRU = cMRU.GetPathList();

						/* OPENFOLDERリストのファイルのリスト */
						const CMRUFolder cMRUFolder;
						std::vector<LPCTSTR> vOPENFOLDER = cMRUFolder.GetPathList();

						//Stonee, 2001/12/21 UNCであれば接続を試みる
						NetConnect( cMRUFolder.GetPath( nId - IDM_SELOPENFOLDER ) );

						/* ファイルオープンダイアログの初期化 */
						CDlgOpenFile	cDlgOpenFile;
						cDlgOpenFile.Create(
							m_hInstance,
							NULL,
							_T("*.*"),
							vOPENFOLDER[ nId - IDM_SELOPENFOLDER ],
							vMRU,
							vOPENFOLDER
						);
						SLoadInfo sLoadInfo( _T(""), CODE_AUTODETECT, false);
						std::vector<std::tstring> files;
						if( !cDlgOpenFile.DoModalOpenDlg( &sLoadInfo, &files ) ){
							break;
						}
						if( NULL == GetTrayHwnd() ){
							break;
						}

						// 新たな編集ウィンドウを起動
						size_t nSize = files.size();
						for( size_t f = 0; f < nSize; f++ ){
							sLoadInfo.cFilePath = files[f].c_str();
							CControlTray::OpenNewEditor( m_hInstance, GetTrayHwnd(), sLoadInfo,
								NULL, true, NULL, m_pShareData->m_Common.m_sTabBar.m_bNewWindow? true : false );
						}
					}
					break;
				}
				return 0L;
			case WM_LBUTTONDBLCLK:
				bLDClick = true;		/* 03/02/20 ai */
				/* 新規編集ウィンドウの追加 */
				OnNewEditor( m_pShareData->m_Common.m_sTabBar.m_bNewWindow == TRUE );
				// Apr. 1, 2003 genta この後で表示されたメニューは閉じる
				::PostMessageAny( GetTrayHwnd(), WM_CANCELMODE, 0, 0 );
				return 0L;
			case WM_RBUTTONDBLCLK:
				return 0L;
			}
			break;

		case WM_QUERYENDSESSION:
			/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
			if( CloseAllEditor( FALSE, GetTrayHwnd(), TRUE, 0 ) ){	// 2006.12.25, 2007.02.13 ryoji 引数追加
				//	Jan. 31, 2000 genta
				//	この時点ではWindowsの終了が確定していないので常駐解除すべきではない．
				//	::DestroyWindow( hwnd );
				return TRUE;
			}else{
				return FALSE;
			}
		case WM_CLOSE:
			/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
			if( CloseAllEditor( FALSE, GetTrayHwnd(), TRUE, 0 ) ){	// 2006.12.25, 2007.02.13 ryoji 引数追加
				::DestroyWindow( hwnd );
			}
			return 0L;

		//	From Here Jan. 31, 2000 genta	Windows終了時の後処理．
		//	Windows終了時はWM_CLOSEが呼ばれない上，DestroyWindowを
		//	呼び出す必要もない．また，メッセージループに戻らないので
		//	メッセージループの後ろの処理をここで完了させる必要がある．
		case WM_ENDSESSION:
			//	もしWindowsの終了が中断されたのなら何もしない
			if( wParam == TRUE )
				OnDestroy();	// 2006.07.09 ryoji WM_DESTROY と同じ処理をする（トレイアイコンの破棄などもNT系では必要）

			return 0;	//	もうこのプロセスに制御が戻ることはない
		//	To Here Jan. 31, 2000 genta
		case WM_DESTROY:
			OnDestroy();

			/* Windows にスレッドの終了を要求します。*/
			::PostQuitMessage( 0 );
			return 0L;
		default:
// << 20010412 by aroka
//	Apr. 24, 2001 genta RegisterWindowMessageを使うように修正
			if( uMsg == m_uCreateTaskBarMsg ){
				/* TaskTray Iconの再登録を要求するメッセージ．
					Explorerが再起動したときに送出される．*/
				CreateTrayIcon( GetTrayHwnd() ) ;
			}
			break;	/* default */
// >> by aroka
	}
	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}




/* WM_COMMANDメッセージ処理 */
void CControlTray::OnCommand( WORD wNotifyCode, WORD wID , HWND hwndCtl )
{
	switch( wNotifyCode ){
	/* メニューからのメッセージ */
	case 0:
		break;
	}
	return;
}

/*!
	@brief 新規ウィンドウを作成する
	
	タスクトレイからの新規作成の場合にはカレントディレクトリ＝
	保存時のデフォルトディレクトリを最後に使われたディレクトリとする．
	ただし最後に使われたディレクトリが存在しない場合は次に使われたディレクトリとし，
	順次存在するディレクトリが見つかるまで履歴を順に試す．
	
	どの履歴も見つからなかった場合には現在のカレントディレクトリで作成する．

	@author genta
	@date 2003.05.30 新規作成
*/
void CControlTray::OnNewEditor( bool bNewWindow )
{

	const TCHAR* szCurDir = NULL;

	//
	//  szCurDir を設定
	//
	//	最近使ったフォルダを順番にたどる
	const CMRUFolder mrufolder;

	// 新規ウィンドウで開くオプションは、タブバー＆グループ化を前提とする
	bNewWindow = bNewWindow
				 && m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd == TRUE
				 && m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin == FALSE;

	int nCount = mrufolder.Length();
	for( int i = 0; i < nCount ; i++ ){
		const TCHAR* recentdir = mrufolder.GetPath( i );
		DWORD attr = GetFileAttributes( recentdir );

		if( attr != -1 ){
			if(( attr & FILE_ATTRIBUTE_DIRECTORY ) != 0 ){
				szCurDir = recentdir;
				break;
			}
		}
	}


	// 編集ウインドウを開く
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = _T("");
	sLoadInfo.eCharCode = CODE_NONE;
	sLoadInfo.bViewMode = false;
	OpenNewEditor( m_hInstance, GetTrayHwnd(), sLoadInfo, NULL, false, szCurDir, bNewWindow );
}

/*!
	新規編集ウィンドウの追加 ver 0

	@date 2000.10.24 genta WinExec -> CreateProcess．同期機能を付加
	@date 2002.02.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2003.05.30 genta 外部プロセス起動時のカレントディレクトリ指定を可能に．
	@date 2007.06.26 ryoji 新規編集ウィンドウは hWndParent と同じグループを指定して起動する
	@date 2008.04.19 ryoji MYWM_FIRST_IDLE 待ちを追加
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
bool CControlTray::OpenNewEditor(
	HINSTANCE			hInstance,			//!< [in] インスタンスID (実は未使用)
	HWND				hWndParent,			//!< [in] 親ウィンドウハンドル．エラーメッセージ表示用
	const SLoadInfo&	sLoadInfo,			//!< [in]
	const TCHAR*		szCmdLineOption,	//!< [in] 追加のコマンドラインオプション
	bool				sync,				//!< [in] trueなら新規エディタの起動まで待機する
	const TCHAR*		szCurDir,			//!< [in] 新規エディタのカレントディレクトリ
	bool				bNewWindow			//!< [in] 新規エディタを新しいウインドウで開く
)
{
	/* 共有データ構造体のアドレスを返す */
	DLLSHAREDATA*	pShareData = CShareData::getInstance()->GetShareData();

	/* 編集ウィンドウの上限チェック */
	if( pShareData->m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
		OkMessage( NULL, _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
		return false;
	}

	// -- -- -- -- コマンドライン文字列を生成 -- -- -- -- //
	CCommandLineString cCmdLineBuf;

	//アプリケーションパス
	TCHAR szEXE[MAX_PATH + 1];
	::GetModuleFileName( NULL, szEXE, _countof( szEXE ) );
	cCmdLineBuf.AppendF( _T("\"%ts\""), szEXE );

	// ファイル名
	if( sLoadInfo.cFilePath.c_str()[0] != _T('\0') )	cCmdLineBuf.AppendF( _T(" \"%ts\""), sLoadInfo.cFilePath.c_str() );

	// コード指定
	if( IsValidCodeType(sLoadInfo.eCharCode) )cCmdLineBuf.AppendF( _T(" -CODE=%d"), sLoadInfo.eCharCode );

	// ビューモード指定
	if( sLoadInfo.bViewMode )cCmdLineBuf.AppendF( _T(" -R") );

	// グループID
	if( false == bNewWindow ){	// 新規エディタをウインドウで開く
		// グループIDを親ウィンドウから取得
		HWND hwndAncestor = MyGetAncestor( hWndParent, GA_ROOTOWNER2 );	// 2007.10.22 ryoji GA_ROOTOWNER -> GA_ROOTOWNER2
		int nGroup = CAppNodeManager::getInstance()->GetEditNode( hwndAncestor )->GetGroup();
		if( nGroup > 0 ){
			cCmdLineBuf.AppendF( _T(" -GROUP=%d"), nGroup );
		}
	}else{
		// 空いているグループIDを使用する
		cCmdLineBuf.AppendF( _T(" -GROUP=%d"), CAppNodeManager::getInstance()->GetFreeGroupId() );
	}

	// 追加のコマンドラインオプション
	TCHAR szResponseFile[_MAX_PATH] = _T("");
	struct CResponsefileDeleter{
		LPCTSTR fileName;
		CResponsefileDeleter(): fileName(NULL){}
		~CResponsefileDeleter(){
			if( fileName && fileName[0] ){
				::DeleteFile( fileName );
				fileName = NULL;
			}
		}
	};
	CResponsefileDeleter respDeleter;
	if( szCmdLineOption ){
		// Grepなどで入りきらない場合はレスポンスファイルを利用する
		if( cCmdLineBuf.max_size() < cCmdLineBuf.size() + auto_strlen(szCmdLineOption) ){
			TCHAR szIniDir[_MAX_PATH];
			GetInidir(szIniDir);
			LPTSTR pszTempFile = _ttempnam(szIniDir, _T("skr_resp"));
			if( !pszTempFile ){
				ErrorMessage(hWndParent, _T("レスポンスファイルの作成に失敗しました。"));
				return false;
			}
			auto_strcpy(szResponseFile, pszTempFile);
			free(pszTempFile);
			CTextOutputStream output(szResponseFile);
			if( !output ){
				ErrorMessage(hWndParent, _T("レスポンスファイルの作成に失敗しました。"));
				return false;
			}
			respDeleter.fileName = szResponseFile;
			// 出力
			output.WriteString(to_wchar(szCmdLineOption));
			output.Close();
			sync = true;
			cCmdLineBuf.AppendF(_T(" -@=\"%ts\""), szResponseFile);
		}else{
			cCmdLineBuf.AppendF(_T(" %ts"), szCmdLineOption);
		}
	}
	// -- -- -- -- プロセス生成 -- -- -- -- //

	//	プロセスの起動
	PROCESS_INFORMATION p;
	STARTUPINFO s;

	s.cb = sizeof_raw( s );
	s.lpReserved = NULL;
	s.lpDesktop = NULL;
	s.lpTitle = NULL;

	s.dwFlags = STARTF_USESHOWWINDOW;
	s.wShowWindow = SW_SHOWDEFAULT;
	s.cbReserved2 = 0;
	s.lpReserved2 = NULL;

	//	May 30, 2003 genta カレントディレクトリ指定を可能に
	//エディタプロセスを起動
	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;
#ifdef _DEBUG
//	dwCreationFlag |= DEBUG_PROCESS; //2007.09.22 kobake デバッグ用フラグ
#endif
	TCHAR szCmdLine[1024]; _tcscpy_s(szCmdLine, _countof(szCmdLine), cCmdLineBuf.c_str());
	BOOL bCreateResult = CreateProcess(
		szEXE,					// 実行可能モジュールの名前
		szCmdLine,				// コマンドラインの文字列
		NULL,					// セキュリティ記述子
		NULL,					// セキュリティ記述子
		FALSE,					// ハンドルの継承オプション
		dwCreationFlag,			// 作成のフラグ
		NULL,					// 新しい環境ブロック
		szCurDir,				// カレントディレクトリの名前
		&s,						// スタートアップ情報
		&p						// プロセス情報
	);
	if( !bCreateResult ){
		//	失敗
		TCHAR* pMsg;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR)&pMsg,
						0,
						NULL
		);
		ErrorMessage(
			hWndParent,
			_T("\'%ts\'\nプロセスの起動に失敗しました。\n%ts"),
			szEXE,
			pMsg
		);
		::LocalFree( (HLOCAL)pMsg );	//	エラーメッセージバッファを解放
		return false;
	}

	bool bRet = true;
	if( sync ){
		//	起動したプロセスが完全に立ち上がるまでちょっと待つ．
		int nResult = WaitForInputIdle( p.hProcess, 10000 );	//	最大10秒間待つ
		if( nResult != 0 ){
			ErrorMessage(
				hWndParent,
				_T("\'%ts\'\nプロセスの起動に失敗しました。"),
				szEXE
			);
			bRet = false;
		}
	}
	else{
		// タブまとめ時は起動したプロセスが立ち上がるまでしばらくタイトルバーをアクティブに保つ	// 2007.02.03 ryoji
		if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
			WaitForInputIdle( p.hProcess, 3000 );
			sync = true;
		}
	}

	// MYWM_FIRST_IDLE が届くまでちょっとだけ余分に待つ	// 2008.04.19 ryoji
	// Note. 起動先プロセスが初期化処理中に COM 関数（SHGetFileInfo API なども含む）を実行すると、
	//       その時点で COM の同期機構が動いて WaitForInputIdle は終了してしまう可能性がある（らしい）。
	if( sync && bRet )
	{
		int i;
		for( i = 0; i < 200; i++ ){
			MSG msg;
			DWORD dwExitCode;
			if( ::PeekMessage( &msg, 0, MYWM_FIRST_IDLE, MYWM_FIRST_IDLE, PM_REMOVE ) ){
				if( msg.message == WM_QUIT ){	// 指定範囲外でも WM_QUIT は取り出される
					::PostQuitMessage( msg.wParam );
					break;
				}
				// 監視対象プロセスからのメッセージなら抜ける
				// そうでなければ破棄して次を取り出す
				if( msg.wParam == p.dwProcessId ){
					break;
				}
			}
			if( ::GetExitCodeProcess( p.hProcess, &dwExitCode ) && dwExitCode != STILL_ACTIVE ){
				break;	// 監視対象プロセスが終了した
			}
			::Sleep(10);
		}
	}

	CloseHandle( p.hThread );
	CloseHandle( p.hProcess );

	return bRet;
}


/*!	新規編集ウィンドウの追加 ver 2:

	@date Oct. 24, 2000 genta create.
	@date Feb. 25, 2012 novice -CODE/-RはOpenNewEditor側で処理するので削除
*/
bool CControlTray::OpenNewEditor2(
	HINSTANCE		hInstance,
	HWND			hWndParent,
	const EditInfo*	pfi,
	bool			bViewMode,
	bool			sync,
	bool			bNewWindow			//!< [in] 新規エディタを新しいウインドウで開く
)
{
	DLLSHAREDATA*	pShareData;

	/* 共有データ構造体のアドレスを返す */
	pShareData = CShareData::getInstance()->GetShareData();

	/* 編集ウィンドウの上限チェック */
	if( pShareData->m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
		OkMessage( NULL, _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
		return false;
	}

	// 追加のコマンドラインオプション
	CCommandLineString cCmdLine;
	if( pfi != NULL ){
		if( pfi->m_ptCursor.x >= 0					)cCmdLine.AppendF( _T(" -X=%d"), pfi->m_ptCursor.x +1 );
		if( pfi->m_ptCursor.y >= 0					)cCmdLine.AppendF( _T(" -Y=%d"), pfi->m_ptCursor.y +1 );
		if( pfi->m_nViewLeftCol >= CLayoutInt(0)	)cCmdLine.AppendF( _T(" -VX=%d"), (Int)pfi->m_nViewLeftCol + 1 );
		if( pfi->m_nViewTopLine >= CLayoutInt(0)	)cCmdLine.AppendF( _T(" -VY=%d"), (Int)pfi->m_nViewTopLine + 1 );
	}
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = pfi ? pfi->m_szPath : _T("");
	sLoadInfo.eCharCode = pfi ? pfi->m_nCharCode : CODE_NONE;
	sLoadInfo.bViewMode = bViewMode;
	return OpenNewEditor( hInstance, hWndParent, sLoadInfo, cCmdLine.c_str(), sync, NULL, bNewWindow );
}
//	To Here Oct. 24, 2000 genta



void CControlTray::ActiveNextWindow()
{
	/* 現在開いている編集窓のリストを得る */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if(  nRowNum > 0 ){
		/* 自分のウィンドウを調べる */
		int				nGroup = 0;
		int				i;
		for( i = 0; i < nRowNum; ++i ){
			if( CEditWnd::getInstance()->GetHwnd() == pEditNodeArr[i].GetHwnd() )
			{
				nGroup = pEditNodeArr[i].m_nGroup;
				break;
			}
		}
		if( i < nRowNum ){
			// 前のウィンドウ
			int		j;
			for( j = i - 1; j >= 0; --j ){
				if( nGroup == pEditNodeArr[j].m_nGroup )
					break;
			}
			if( j < 0 ){
				for( j = nRowNum - 1; j > i; --j ){
					if( nGroup == pEditNodeArr[j].m_nGroup )
						break;
				}
			}
			/* 前のウィンドウをアクティブにする */
			HWND	hwndWork = pEditNodeArr[j].GetHwnd();
			ActivateFrameWindow( hwndWork );
			/* 最後のペインをアクティブにする */
			::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 1 );
		}
		delete [] pEditNodeArr;
	}
}

void CControlTray::ActivePrevWindow()
{
	/* 現在開いている編集窓のリストを得る */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if(  nRowNum > 0 ){
		/* 自分のウィンドウを調べる */
		int				nGroup = 0;
		int				i;
		for( i = 0; i < nRowNum; ++i ){
			if( CEditWnd::getInstance()->GetHwnd() == pEditNodeArr[i].GetHwnd() ){
				nGroup = pEditNodeArr[i].m_nGroup;
				break;
			}
		}
		if( i < nRowNum ){
			// 次のウィンドウ
			int		j;
			for( j = i + 1; j < nRowNum; ++j ){
				if( nGroup == pEditNodeArr[j].m_nGroup )
					break;
			}
			if( j >= nRowNum ){
				for( j = 0; j < i; ++j ){
					if( nGroup == pEditNodeArr[j].m_nGroup )
						break;
				}
			}
			/* 次のウィンドウをアクティブにする */
			HWND	hwndWork = pEditNodeArr[j].GetHwnd();
			ActivateFrameWindow( hwndWork );
			/* 最初のペインをアクティブにする */
			::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 0 );
		}
		delete [] pEditNodeArr;
	}
}



/*!	サクラエディタの全終了

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2006.12.25 ryoji 複数の編集ウィンドウを閉じるときの確認（引数追加）
*/
void CControlTray::TerminateApplication(
	HWND hWndFrom	//!< [in] 呼び出し元のウィンドウハンドル
)
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();	/* 共有データ構造体のアドレスを返す */

	/* 現在の編集ウィンドウの数を調べる */
	if( pShareData->m_Common.m_sGeneral.m_bExitConfirm ){	//終了時の確認
		if( 0 < CAppNodeGroupHandle(0).GetEditorWindowsNum() ){
			if( IDYES != ::MYMESSAGEBOX(
				hWndFrom,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				_T("現在開いている編集用のウィンドウをすべて閉じて終了しますか?")
			) ){
				return;
			}
		}
	}
	/* 「すべてのウィンドウを閉じる」要求 */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
	BOOL bCheckConfirm = (pShareData->m_Common.m_sGeneral.m_bExitConfirm)? FALSE: TRUE;	// 2006.12.25 ryoji 終了確認済みならそれ以上は確認しない
	if( CloseAllEditor( bCheckConfirm, hWndFrom, TRUE, 0 ) ){	// 2006.12.25, 2007.02.13 ryoji 引数追加
		::PostMessageAny( pShareData->m_sHandles.m_hwndTray, WM_CLOSE, 0, 0 );
	}
	return;
}




/*!	すべてのウィンドウを閉じる

	@date Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2006.12.25 ryoji 複数の編集ウィンドウを閉じるときの確認（引数追加）
	@date 2007.02.13 ryoji 「編集の全終了」を示す引数(bExit)を追加
	@date 2007.06.20 ryoji nGroup引数を追加
*/
BOOL CControlTray::CloseAllEditor(
	BOOL	bCheckConfirm,	//!< [in] [すべて閉じる]確認オプションに従って問い合わせをするかどうか
	HWND	hWndFrom,		//!< [in] 呼び出し元のウィンドウハンドル
	BOOL	bExit,			//!< [in] TRUE: 編集の全終了 / FALSE: すべて閉じる
	int		nGroup			//!< [in] グループID
)
{
	EditNode*	pWndArr;
	int		n;

	n = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}
	
	/* 全編集ウィンドウへ終了要求を出す */
	BOOL	bRes = CAppNodeGroupHandle(nGroup).RequestCloseEditor( pWndArr, n, bExit, bCheckConfirm, hWndFrom );	// 2007.02.13 ryoji bExitを引き継ぐ
	delete []pWndArr;
	return bRes;
}




/*! ポップアップメニュー(トレイ左ボタン) */
int	CControlTray::CreatePopUpMenu_L( void )
{
	int			i;
	int			j;
	int			nId;
	HMENU		hMenuTop;
	HMENU		hMenu;
	HMENU		hMenuPopUp;
	TCHAR		szMenu[100 + MAX_PATH * 2];	//	Jan. 19, 2001 genta
	POINT		po;
	RECT		rc;
	EditInfo*	pfi;

	//本当はセマフォにしないとだめ
	if( m_bUseTrayMenu ) return -1;
	m_bUseTrayMenu = true;

	m_CMenuDrawer.ResetContents();
	CFileNameManager::getInstance()->TransformFileName_MakeCache();

	// リソースを使わないように
	hMenuTop = ::CreatePopupMenu();
	hMenu = ::CreatePopupMenu();
	m_CMenuDrawer.MyAppendMenu( hMenuTop, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenu, L"TrayL", L"" );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, _T(""), _T("N"), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, _T(""), _T("O"), FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GREP_DIALOG, _T(""), _T("G"), FALSE );
	m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	/* MRUリストのファイルのリストをメニューにする */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	const CMRUFile cMRU;
	hMenuPopUp = cMRU.CreateMenu( &m_CMenuDrawer );	//	ファイルメニュー
	if ( cMRU.MenuLength() > 0 ){
		//	アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("最近使ったファイル"), _T("F") );
	}
	else {
		//	非アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , _T("最近使ったファイル"), _T("F") );
	}

	/* 最近使ったフォルダのメニューを作成 */
//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
	const CMRUFolder cMRUFolder;
	hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
	if ( cMRUFolder.MenuLength() > 0 ){
		//	アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("最近使ったフォルダ"), _T("D") );
	}
	else {
		//	非アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp, _T("最近使ったフォルダ"), _T("D") );
	}

	m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEALL, _T(""), _T("Z"), FALSE );	// Jan. 24, 2005 genta

	/* 現在開いている編集窓のリストをメニューにする */
	j = 0;
	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; ++i ){
		if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].GetHwnd() ) ){
			++j;
		}
	}

	if( j > 0 ){
		m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
		j = 0;
		for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; ++i ){
			if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].GetHwnd() ) ){
				/* トレイからエディタへの編集ファイル名要求通知 */
				::SendMessage( m_pShareData->m_sNodes.m_pEditArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0 );
				pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

				// メニューラベル。1からアクセスキーを振る
				CFileNameManager::getInstance()->GetMenuFullLabel_WinList( szMenu, _countof(szMenu), pfi, m_pShareData->m_sNodes.m_pEditArr[i].m_nId, i );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + i, szMenu, _T(""), FALSE );
				++j;
			}
		}
	}
	m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALLEDITORS, _T(""), _T("Q"), FALSE );	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)	//Feb. 18, 2001 JEPRO アクセスキー変更(L→Q)	// 2006.10.21 ryoji 表示文字列変更	// 2007.02.13 ryoji →F_EXITALLEDITORS
	if( j == 0 ){
		::EnableMenuItem( hMenu, F_EXITALLEDITORS, MF_BYCOMMAND | MF_GRAYED );	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)	// 2007.02.13 ryoji →F_EXITALLEDITORS
		::EnableMenuItem( hMenu, F_FILESAVEALL, MF_BYCOMMAND | MF_GRAYED );	// Jan. 24, 2005 genta
	}

	//	Jun. 9, 2001 genta ソフトウェア名改称
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, _T(""), _T("X"), FALSE );	//Dec. 26, 2000 JEPRO F_に変更

	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );
	po.y -= 4;

	rc.left = 0;
	rc.right = 0;
	rc.top = 0;
	rc.bottom = 0;

	::SetForegroundWindow( GetTrayHwnd() );
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
		GetTrayHwnd(),
		&rc
	);
	::PostMessageAny( GetTrayHwnd(), WM_USER + 1, 0, 0 );
	::DestroyMenu( hMenuTop );
//	MYTRACE( _T("nId=%d\n"), nId );

	m_bUseTrayMenu = false;

	return nId;
}

//キーワード：トレイ右クリックメニュー順序
//	Oct. 12, 2000 JEPRO ポップアップメニュー(トレイ左ボタン) を参考にして新たに追加した部分

/*! ポップアップメニュー(トレイ右ボタン) */
int	CControlTray::CreatePopUpMenu_R( void )
{
	int		nId;
	HMENU	hMenuTop;
	HMENU	hMenu;
	POINT	po;
	RECT	rc;

	//本当はセマフォにしないとだめ
	if( m_bUseTrayMenu ) return -1;
	m_bUseTrayMenu = true;

	m_CMenuDrawer.ResetContents();

	// リソースを使わないように
	hMenuTop = ::CreatePopupMenu();
	hMenu = ::CreatePopupMenu();
	m_CMenuDrawer.MyAppendMenu( hMenuTop, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenu, L"TrayR", L"" );

	/* トレイ右クリックの「ヘルプ」メニュー */
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , _T(""), _T("O"), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH , _T(""), _T("S"), FALSE );	//Nov. 25, 2000 JEPRO 「トピックの」→「キーワード」に変更
	m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ABOUT, _T(""), _T("A"), FALSE );	//Dec. 25, 2000 JEPRO F_に変更
	m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	//	Jun. 18, 2001 genta ソフトウェア名改称
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, _T(""), _T("X"), FALSE );

	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );
	po.y -= 4;

	rc.left = 0;
	rc.right = 0;
	rc.top = 0;
	rc.bottom = 0;

	::SetForegroundWindow( GetTrayHwnd() );
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
		GetTrayHwnd(),
		&rc
	);
	::PostMessageAny( GetTrayHwnd(), WM_USER + 1, 0, 0 );
	::DestroyMenu( hMenuTop );
//	MYTRACE( _T("nId=%d\n"), nId );

	m_bUseTrayMenu = false;

	return nId;
}

/*! アクセラレータテーブル作成
	@date 2013.04.20 novice 共通処理を関数化
*/
void CControlTray::CreateAccelTbl( void )
{
	m_pShareData->m_sHandles.m_hAccel = CKeyBind::CreateAccerelator(
		m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
		m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
	);

	if( NULL == m_pShareData->m_sHandles.m_hAccel ){
		ErrorMessage(
			NULL,
			_T("CControlTray::CreateAccelTbl()\n")
			_T("アクセラレータ テーブルが作成できません。\n")
			_T("システムリソースが不足しています。")
		);
	}
}

/*! アクセラレータテーブル破棄
	@date 2013.04.20 novice 共通処理を関数化
*/
void CControlTray::DeleteAccelTbl( void )
{
	if( m_pShareData->m_sHandles.m_hAccel ){
		::DestroyAcceleratorTable( m_pShareData->m_sHandles.m_hAccel );
		m_pShareData->m_sHandles.m_hAccel = NULL;
	}
}

/*!
	@brief WM_DESTROY 処理
	@date 2006.07.09 ryoji 新規作成
*/
void CControlTray::OnDestroy()
{
	HWND hwndExitingDlg = 0;

	if (GetTrayHwnd() == NULL)
		return;	// 既に破棄されている

	// ホットキーの破棄
	::UnregisterHotKey( GetTrayHwnd(), ID_HOTKEY_TRAYMENU );

	// 2006.07.09 ryoji 共有データ保存を CControlProcess::Terminate() から移動
	//
	// 「タスクトレイに常駐しない」設定でエディタ画面（Normal Process）を立ち上げたまま
	// セッション終了するような場合でも共有データ保存が行われなかったり中断されることが
	// 無いよう、ここでウィンドウが破棄される前に保存する
	//

	/* 終了ダイアログを表示する */
	if( m_pShareData->m_Common.m_sGeneral.m_bDispExitingDialog ){
		/* 終了中ダイアログの表示 */
		hwndExitingDlg = ::CreateDialog(
			m_hInstance,
			MAKEINTRESOURCE( IDD_EXITING ),
			GetTrayHwnd()/*::GetDesktopWindow()*/,
			ExitingDlgProc
		);
		::ShowWindow( hwndExitingDlg, SW_SHOW );
	}

	/* 共有データの保存 */
	CShareData_IO::SaveShareData();

	/* 終了ダイアログを表示する */
	if( m_pShareData->m_Common.m_sGeneral.m_bDispExitingDialog ){
		/* 終了中ダイアログの破棄 */
		::DestroyWindow( hwndExitingDlg );
	}

	if( m_bCreatedTrayIcon ){	/* トレイにアイコンを作った */
		TrayMessage( GetTrayHwnd(), NIM_DELETE, 0, NULL, NULL );
	}

	// アクセラレータテーブルの削除
	DeleteAccelTbl();

	m_hWnd = NULL;
}

/*!
	@brief 終了ダイアログ用プロシージャ
	@date 2006.07.02 ryoji CControlProcess から移動
*/
INT_PTR CALLBACK CControlTray::ExitingDlgProc(
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
