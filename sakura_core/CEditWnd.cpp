//	$Id$
/*!	@file
	@brief 編集ウィンドウ（外枠）管理クラス

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro
	Copyright (C) 2001, mik, hor
	Copyright (C) 2002, YAZAKI, genta, aroka, MIK
	Copyright (C) 2003, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include <stdio.h>
#include <windows.h>
#include <winuser.h>
#include <io.h>
#include <mbctype.h>
#include <mbstring.h>

#include "CEditApp.h"
#include "CEditWnd.h"
#include "sakura_rc.h"
#include "CEditDoc.h"
#include "debug.h"
#include "CDlgAbout.h"
#include "mymessage.h"
#include "CShareData.h"
#include "CPrint.h"
#include "etc_uty.h"
#include "charcode.h"
#include "global.h"
#include "CDlgPrintSetting.h"
#include "CDlgPrintPage.h"
#include "funccode.h"		// Stonee, 2001/03/12
#include "CPrintPreview.h" /// 2002/2/3 aroka
#include "CMarkMgr.h" /// 2002/2/3 aroka
#include "CCommandLine.h" /// 2003/1/26 aroka
#include "CSMacroMgr.h" // Jun. 16, 2002 genta
#include "CRunningTimer.h"


#define IDT_TOOLBAR		456
#define ID_TOOLBAR		100


//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたので
//	定義を削除

#ifndef TBSTYLE_ALTDRAG
	#define TBSTYLE_ALTDRAG	0x0400
#endif
#ifndef TBSTYLE_FLAT
	#define TBSTYLE_FLAT	0x0800
#endif
#ifndef TBSTYLE_LIST
	#define TBSTYLE_LIST	0x1000
#endif

#ifndef	WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL	0x020A
#endif

#define		YOHAKU_X		4		/* ウィンドウ内の枠と紙の隙間最小値 */
#define		YOHAKU_Y		4		/* ウィンドウ内の枠と紙の隙間最小値 */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたので
//	定義を削除

//	/* メッセージループ */
//	DWORD MessageLoop_Thread( DWORD pCEditWndObject );

LRESULT CALLBACK CEditWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	CEditWnd* pSEWnd;
	pSEWnd = ( CEditWnd* )::GetWindowLong( hwnd, GWL_USERDATA );
	if( NULL != pSEWnd ){
		return pSEWnd->DispatchEvent( hwnd, uMsg, wParam, lParam );
	}
	return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
}


/*
||  タイマーメッセージのコールバック関数
||
||	ツールバーの状態更新のためにタイマーを使用しています
*/
VOID CALLBACK CEditWndTimerProc(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT idEvent,	// timer identifier
	DWORD dwTime 	// current system time
)
{
	CEditWnd*	pCEdit;
	pCEdit = ( CEditWnd* )::GetWindowLong( hwnd, GWL_USERDATA );
	if( NULL != pCEdit ){
		pCEdit->OnTimer( hwnd, uMsg, idEvent, dwTime );
	}
	return;
}

//by 鬼(2)
void CALLBACK SysMenuTimer(HWND Wnd, UINT Msg, UINT Event, DWORD Time)
{
	CEditWnd *WndObj;
	WndObj = (CEditWnd*)GetWindowLong(Wnd, GWL_USERDATA);
	if(WndObj != NULL)
		WndObj->OnSysMenuTimer();

	KillTimer(Wnd, Event);
}

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CEditWnd::CEditWnd() :
	m_hWnd( NULL ),
	m_bDragMode( FALSE ),
	m_hwndParent( NULL ),
	m_hwndToolBar( NULL ),
	m_hwndStatusBar( NULL ),
	m_hwndProgressBar( NULL ),
	m_hdcCompatDC( NULL ),			/* 再描画用コンパチブルＤＣ */
	m_hbmpCompatBMP( NULL ),		/* 再描画用メモリＢＭＰ */
	m_hbmpCompatBMPOld( NULL ),		/* 再描画用メモリＢＭＰ(OLD) */
// 20020331 aroka 再変換対応 for 95/NT
	m_uMSIMEReconvertMsg( ::RegisterWindowMessage( RWM_RECONVERT ) ),
	m_uATOKReconvertMsg( ::RegisterWindowMessage( MSGNAME_ATOK_RECONVERT ) ),

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	m_pPrintPreview( NULL ),
	m_pszAppName( GSTR_EDITWINDOWNAME ),
	m_hwndSearchBox( NULL ),
	m_fontSearchBox( NULL ),
	m_nCurrentFocus( 0 ),
	m_IconClicked(icNone) //by 鬼(2)
{
	//	Dec. 4, 2002 genta
	InitMenubarMessageFont();

	// Sep. 10, 2002 genta
	m_cEditDoc.m_pcEditWnd = this;
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();

//	MYTRACE( "CEditWnd::CEditWnd()おわり\n" );
	return;
}




CEditWnd::~CEditWnd()
{
	//	Dec. 4, 2002 genta
	/* キャレットの行桁位置表示用フォント */
	::DeleteObject( m_hFontCaretPosInfo );

	/* 再描画用メモリＢＭＰ */
	if( m_hbmpCompatBMP != NULL ){
		/* 再描画用メモリＢＭＰ(OLD) */
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );
		::DeleteObject( m_hbmpCompatBMP );
	}
	/* 再描画用コンパチブルＤＣ */
	if( m_hdcCompatDC != NULL ){
		::DeleteDC( m_hdcCompatDC );
	}

	if( NULL != m_hWnd ){
		m_hWnd = NULL;
	}
	return;
}





/*!	作成

	@param hInstance [in] Instance Handle
	@param hwndParent [in] 親ウィンドウのハンドル
	@param pszPath [in] 最初に開くファイルのパス．NULLのとき開くファイル無し．
	@param nCharCode [in] 漢字コード
	@param bReadOnly [in] 読みとり専用で開くかどうか
	@param nDocumentType [in] 文書タイプ．-1のとき強制指定無し．
	
	@date 2002.03.07 genta nDocumentType追加
*/
HWND CEditWnd::Create(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	const char*	pszPath,
	int			nCharCode,
	BOOL		bReadOnly,
	int			nDocumentType
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditWnd::Create" );

	//	Dec. 6, 2002 genta
	//	small icon指定のため RegisterClassExに変更
	WNDCLASSEX	wc;
	HWND		hWnd;
	ATOM		atom;
	BOOL		bOpened;
	char szMsg[512];

	if( m_pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
		wsprintf( szMsg, "編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。", MAX_EDITWINDOWS );
		::MessageBox( NULL, szMsg, GSTR_APPNAME, MB_OK );
		return NULL;
	}


	m_hInstance = hInstance;
	m_hwndParent = hwndParent;
	//	Apr. 27, 2000 genta
	//	サイズ変更時のちらつきを抑えるためCS_HREDRAW | CS_VREDRAW を外した
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= CEditWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 32;
	wc.hInstance		= m_hInstance;
	//	Dec, 2, 2002 genta アイコン読み込み方法変更
	wc.hIcon			= GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false );

	wc.hCursor			= NULL/*LoadCursor( NULL, IDC_ARROW )*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_3DSHADOW + 1)*/;
	wc.lpszMenuName		= MAKEINTRESOURCE( IDR_MENU1 );
	wc.lpszClassName	= m_pszAppName;

	//	Dec. 6, 2002 genta
	//	small icon指定のため RegisterClassExに変更
	wc.cbSize			= sizeof( wc );
	wc.hIconSm			= GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
	if( 0 == ( atom = RegisterClassEx( &wc ) ) ){
//		return NULL;
	}

	/* ウィンドウサイズ継承 */
	int	nWinCX, nWinCY;
	if( m_pShareData->m_Common.m_bSaveWindowSize ){
		nWinCX = m_pShareData->m_Common.m_nWinSizeCX;
		nWinCY = m_pShareData->m_Common.m_nWinSizeCY;
	}else{
		nWinCX = CW_USEDEFAULT;
		nWinCY = 0;
	}

	/* ウィンドウサイズ指定 */
	FileInfo fi;
	CCommandLine::Instance()->GetFileInfo(fi);
	if( fi.m_nWindowSizeX >= 0 ){
		nWinCX = fi.m_nWindowSizeX;
	}
	if( fi.m_nWindowSizeY >= 0 ){
		nWinCY = fi.m_nWindowSizeY;
	}

	/* ウィンドウ位置指定 */
	int nWinOX, nWinOY;
	nWinOX = CW_USEDEFAULT;
	nWinOY = 0;
	if( fi.m_nWindowOriginX >= 0 ){
		nWinOX = fi.m_nWindowOriginX;
	}
	if( fi.m_nWindowOriginY >= 0 ){
		nWinOY = fi.m_nWindowOriginY;
	}

	hWnd = ::CreateWindowEx(
		0 	// extended window style
//		| WS_EX_CLIENTEDGE
		,
		m_pszAppName,		// pointer to registered class name
		m_pszAppName,		// pointer to window name
//		WS_VISIBLE |
		WS_OVERLAPPEDWINDOW |
		WS_CLIPCHILDREN	|
//		WS_HSCROLL | WS_VSCROLL	|
		0,	// window style

		nWinOX,				// horizontal position of window
		nWinOY,				// vertical position of window
		nWinCX,				// window width
		nWinCY,				// window height
		NULL,				// handle to parent or owner window
		NULL,				// handle to menu or child-window identifier
		m_hInstance,		// handle to application instance
		NULL				// pointer to window-creation data
	);
	m_hWnd = hWnd;

	m_cIcons.Create( m_hInstance, m_hWnd );	//	CreateImage List

	m_CMenuDrawer.Create( m_hInstance, m_hWnd, &m_cIcons );

	if( NULL != m_hWnd ){
		::SetWindowLong( m_hWnd, GWL_USERDATA, (LONG)this );

		/* 再描画用コンパチブルＤＣ */
		HDC hdc = ::GetDC( m_hWnd );
		m_hdcCompatDC = ::CreateCompatibleDC( hdc );
		::ReleaseDC( m_hWnd, hdc );

	}

	if( m_pShareData->m_Common.m_bDispTOOLBAR ){	/* 次回ウィンドウを開いたときツールバーを表示する */
 		/* ツールバー作成 */
		CreateToolBar();
	}

	/* ステータスバー */
	if( m_pShareData->m_Common.m_bDispSTATUSBAR ){	/* 次回ウィンドウを開いたときステータスバーを表示する */
		/* ステータスバー作成 */
		CreateStatusBar();
	}

	/* ファンクションキー バー */
	if( m_pShareData->m_Common.m_bDispFUNCKEYWND ){	/* 次回ウィンドウを開いたときファンクションキーを表示する */
		BOOL bSizeBox;
		if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 ){	/* ファンクションキー表示位置／0:上 1:下 */
			bSizeBox = FALSE;
		}else{
			bSizeBox = TRUE;
			/* ステータスパーを表示している場合はサイズボックスを表示しない */
			if( NULL != m_hwndStatusBar ){
				bSizeBox = FALSE;
			}
		}
		m_CFuncKeyWnd.Open( hInstance, m_hWnd, &m_cEditDoc, bSizeBox );
	}


	if( FALSE == m_cEditDoc.Create( m_hInstance, m_hWnd, &m_cIcons/*, 1, 1, 0, 0*/ ) ){
		::MessageBox(
			m_hWnd,
			"クライアントウィンドウの作成に失敗しました", GSTR_APPNAME,
			MB_OK
		);
	}
	/* デスクトップからはみ出さないようにする */
	RECT	rcOrg;
	RECT	rcDesktop;
//	int		nWork;
	::SystemParametersInfo( SPI_GETWORKAREA, NULL, &rcDesktop, 0 );
	::GetWindowRect( m_hWnd, &rcOrg );
	/* ウィンドウ位置調整 */
	if( rcOrg.bottom >= rcDesktop.bottom ){
		if( 0 > rcOrg.top - (rcOrg.bottom - rcDesktop.bottom ) ){
			rcOrg.top = 0;
		}else{
			rcOrg.top -= rcOrg.bottom - rcDesktop.bottom;
		}
		rcOrg.bottom = rcDesktop.bottom;	//@@@ 2002.01.08
	}
	if( rcOrg.right >= rcDesktop.right ){
		if( 0 > rcOrg.left - (rcOrg.right - rcDesktop.right ) ){
			rcOrg.left = 0;
		}else{
			rcOrg.left -= rcOrg.right - rcDesktop.right;
		}
		rcOrg.right = rcDesktop.right;	//@@@ 2002.01.08
	}
	/* ウィンドウサイズ調整 */
	if( rcOrg.top < rcDesktop.top ){
		rcOrg.top = rcDesktop.top;
	}
	if( rcOrg.left < rcDesktop.left ){
		rcOrg.left = rcDesktop.left;
	}
	if( rcOrg.bottom >= rcDesktop.bottom ){
		//rcOrg.bottom = rcDesktop.bottom - 1;	//@@@ 2002.01.08
		rcOrg.bottom = rcDesktop.bottom;	//@@@ 2002.01.08
	}
	if( rcOrg.right >= rcDesktop.right ){
		//rcOrg.right = rcDesktop.right - 1;	//@@@ 2002.01.08
		rcOrg.right = rcDesktop.right;	//@@@ 2002.01.08
	}
	::SetWindowPos(
		m_hWnd, 0,
		rcOrg.left, rcOrg.top,
		rcOrg.right - rcOrg.left, rcOrg.bottom - rcOrg.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);

	/* ウィンドウサイズ継承 */
	if( m_pShareData->m_Common.m_bSaveWindowSize &&
		m_pShareData->m_Common.m_nWinSizeType == SIZE_MAXIMIZED ){
		::ShowWindow( m_hWnd, SW_SHOWMAXIMIZED );
	}else{
		::ShowWindow( m_hWnd, SW_SHOW );
	}
	if( NULL != m_hWnd ){
		/* ドロップされたファイルを受け入れる */
		::DragAcceptFiles( m_hWnd, TRUE );
		/* 編集ウィンドウリストへの登録 */
		if( FALSE == CShareData::getInstance()->AddEditWndList( m_hWnd ) ){
			wsprintf( szMsg, "編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。", MAX_EDITWINDOWS );
			::MessageBox( m_hWnd, szMsg, GSTR_APPNAME, MB_OK );
			::DestroyWindow( m_hWnd );
			m_hWnd = hWnd = NULL;
			return hWnd;
		}
		/* タイマーを起動 */
		if( 0 == ::SetTimer( m_hWnd, IDT_TOOLBAR, 300, (TIMERPROC)CEditWndTimerProc ) ){
			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
				"CEditWnd::Create()\nタイマーが起動できません。\nシステムリソースが不足しているのかもしれません。"
			);
		}
	}
	::InvalidateRect( m_hWnd, NULL, TRUE );
	if( NULL != pszPath ){
		char*	pszPathNew = new char[_MAX_PATH];
		strcpy( pszPathNew, pszPath );
		::ShowWindow( m_hWnd, SW_SHOW );
		if( !m_cEditDoc.FileRead( pszPathNew, &bOpened, nCharCode, bReadOnly, TRUE ) ){
			/* ファイルが既に開かれている */
			if( bOpened ){
				::PostMessage( m_hWnd, WM_CLOSE, 0, 0 );
				delete [] pszPathNew;
				return NULL;
			}
			else {
				//	Nov. 20, 2000 genta
				m_cEditDoc.SetImeMode( m_pShareData->m_Types[0].m_nImeState );
			}
		}
		//	Mar. 7, 2002 genta 文書タイプの強制指定
		if( nDocumentType >= 0 ){
			m_cEditDoc.SetDocumentType( nDocumentType, true );
			//	2002/05/07 YAZAKI タイプ別設定一覧の一時適用のコードを流用
			m_cEditDoc.LockDocumentType();
			/* 設定変更を反映させる */
			m_cEditDoc.OnChangeSetting();
		}
		delete [] pszPathNew;
	}
	else {
		//	Nov. 20, 2000 genta
		m_cEditDoc.SetImeMode( m_pShareData->m_Types[0].m_nImeState );
	}
	//	YAZAKI 2002/05/30 IMEウィンドウの位置がおかしいのを修正。
	m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].SetIMECompFormPos();
	return m_hWnd;
}


//	キーワード：ステータスバー順序
/* ステータスバー作成 */
void CEditWnd::CreateStatusBar( void )
{
//	int		nStArr[] = { 300, 400, 500. -1 };
//	int		nStArrNum = sizeof( nStArr ) / sizeof( nStArr[0] );

	/* ステータスバー */
	m_hwndStatusBar = ::CreateStatusWindow(
		WS_CHILD | WS_VISIBLE | WS_EX_RIGHT | SBARS_SIZEGRIP,
		"",
		m_hWnd,
		IDW_STATUSBAR
	);

	/* プログレスバー */
	m_hwndProgressBar = ::CreateWindowEx(
		WS_EX_TOOLWINDOW,
		PROGRESS_CLASS,
		(LPSTR) NULL,
		WS_CHILD /*|  WS_VISIBLE*/,
		3,
		5,
		150,
		13,
		m_hwndStatusBar,
		NULL,
		m_hInstance,
		0
	);
//	::ShowWindow( m_hwndProgressBar, SW_SHOW );

	if( NULL != m_CFuncKeyWnd.m_hWnd ){
		m_CFuncKeyWnd.SizeBox_ONOFF( FALSE );
	}
	//スプリッターの、サイズボックスの位置を変更
	m_cEditDoc.m_cSplitterWnd.DoSplit( -1, -1);
	return;
}


/* ステータスバー破棄 */
void CEditWnd::DestroyStatusBar( void )
{
	if( NULL != m_hwndProgressBar ){
		::DestroyWindow( m_hwndProgressBar );
		m_hwndProgressBar = NULL;
	}
	::DestroyWindow( m_hwndStatusBar );
	m_hwndStatusBar = NULL;

	if( NULL != m_CFuncKeyWnd.m_hWnd ){
		BOOL bSizeBox;
		if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 ){	/* ファンクションキー表示位置／0:上 1:下 */
			/* サイズボックスの表示／非表示切り替え */
			bSizeBox = FALSE;
		}else{
			bSizeBox = TRUE;
			/* ステータスパーを表示している場合はサイズボックスを表示しない */
			if( NULL != m_hwndStatusBar ){
				bSizeBox = FALSE;
			}
		}
		m_CFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
	}
	//スプリッターの、サイズボックスの位置を変更
	m_cEditDoc.m_cSplitterWnd.DoSplit( -1, -1 );

	return;
}

/* ツールバー作成
	@date @@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
*/
void CEditWnd::CreateToolBar( void )
{
	int				nFlag;
	TBBUTTON		tbb;
	int				i;
	int				nIdx;
	UINT			uToolType;
	nFlag = 0;

	/* ツールバーウィンドウの作成 */
	m_hwndToolBar = ::CreateWindowEx(
		0,
		TOOLBARCLASSNAME,
		NULL,
		WS_CHILD | WS_VISIBLE | /*WS_BORDER | */
/*		WS_EX_WINDOWEDGE| */
		TBSTYLE_TOOLTIPS |
//		TBSTYLE_WRAPABLE |
//		TBSTYLE_ALTDRAG |
//		CCS_ADJUSTABLE |
		nFlag,
		0, 0,
		0, 0,
		m_hWnd,
		(HMENU)ID_TOOLBAR,
		m_hInstance,
		NULL
	);
	if( NULL == m_hwndToolBar ){
		if( m_pShareData->m_Common.m_bToolBarIsFlat ){	/* フラットツールバーにする／しない */
			m_pShareData->m_Common.m_bToolBarIsFlat = FALSE;
		}
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
			"ツールバーの作成に失敗しました。"
		);
	}else{
		::SendMessage( m_hwndToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );
		//	Oct. 12, 2000 genta
		//	既に用意されているImage Listをアイコンとして登録
		m_cIcons.SetToolBarImages( m_hwndToolBar );
		/* ツールバーにボタンを追加 */
		int count = 0;	//@@@ 2002.06.15 MIK
		for( i = 0; i < m_pShareData->m_Common.m_nToolBarButtonNum; ++i ){
			nIdx = m_pShareData->m_Common.m_nToolBarButtonIdxArr[i];
			tbb = m_CMenuDrawer.m_tbMyButton[m_pShareData->m_Common.m_nToolBarButtonIdxArr[i]];
			//::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&tbb );

			//@@@ 2002.06.15 MIK start
			switch( tbb.fsStyle )
			{
			case TBSTYLE_DROPDOWN:	//ドロップダウン
				//拡張スタイルに設定
				::SendMessage( m_hwndToolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS );
				::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&tbb );
				count++;
				break;

			case TBSTYLE_COMBOBOX:	//コンボボックス
				{
					RECT			rc;
					TBBUTTONINFO	tbi;
					//HWND			my_hwnd;
					TBBUTTON		my_tbb;
					//int			width;
					LOGFONT			lf;
					//HFONT			my_font;
					int				my_i;

					switch( tbb.idCommand )
					{
					case F_SEARCH_BOX:
						if( m_hwndSearchBox )
						{
							break;
						}
						
						//セパレータ作る
						memset( &my_tbb, 0, sizeof(my_tbb) );
						my_tbb.fsStyle   = TBSTYLE_SEP;
						my_tbb.idCommand = tbb.idCommand;	//同じIDにしておく
						::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&my_tbb );
						count++;

						//サイズを設定する
						tbi.cbSize = sizeof(tbi);
						tbi.dwMask = TBIF_SIZE;
						tbi.cx     = 160;	//ボックスの幅
						::SendMessage( m_hwndToolBar, TB_SETBUTTONINFO, (WPARAM)(tbb.idCommand), (LPARAM)&tbi );

						//サイズを取得する
						rc.right = rc.left = rc.top = rc.bottom = 0;
						::SendMessage( m_hwndToolBar, TB_GETITEMRECT, (WPARAM)(count-1), (LPARAM)&rc );

						//コンボボックスを作る
						//	Mar. 8, 2003 genta 検索ボックスを1ドット下にずらした
						m_hwndSearchBox = CreateWindow( "COMBOBOX", "Combo",
								WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN
								/*| CBS_SORT*/ | CBS_AUTOHSCROLL /*| CBS_DISABLENOSCROLL*/,
								rc.left, rc.top + 1, rc.right - rc.left, (rc.bottom - rc.top) * 10,
								m_hwndToolBar, (HMENU)(INT_PTR)tbb.idCommand, m_hInstance, NULL );
						if( m_hwndSearchBox )
						{
							m_nCurrentFocus = 0;

							memset( &lf, 0, sizeof(LOGFONT) );
							lf.lfHeight			= 12; // Jan. 14, 2003 genta ダイアログにあわせてちょっと小さく
							lf.lfWidth			= 0;
							lf.lfEscapement		= 0;
							lf.lfOrientation	= 0;
							lf.lfWeight			= FW_NORMAL;
							lf.lfItalic			= FALSE;
							lf.lfUnderline		= FALSE;
							lf.lfStrikeOut		= FALSE;
							lf.lfCharSet		= SHIFTJIS_CHARSET;
							lf.lfOutPrecision	= OUT_DEFAULT_PRECIS;
							lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
							lf.lfQuality		= DEFAULT_QUALITY;
							lf.lfPitchAndFamily	= FF_MODERN | DEFAULT_PITCH;
							//strcpy( lf.lfFaceName, "ＭＳ ゴシック" );
							strcpy( lf.lfFaceName, "ＭＳ Ｐゴシック" );
							m_fontSearchBox = ::CreateFontIndirect( &lf );
							if( m_fontSearchBox )
							{
								::SendMessage( m_hwndSearchBox, WM_SETFONT, (UINT_PTR)m_fontSearchBox, MAKELONG (TRUE, 0) );
							}

							//入力長制限
							::SendMessage( m_hwndSearchBox, CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );

							//拡張インタフェース
							//::SendMessage( m_hwndSearchBox, CB_SETEXTENDEDUI, (WPARAM)(BOOL)TRUE, 0 );

							//検索ボックスを更新
							::SendMessage( m_hwndSearchBox, CB_RESETCONTENT, 0, 0 );
							for( my_i = 0; my_i < m_pShareData->m_nSEARCHKEYArrNum; my_i++ )
							{
								::SendMessage( m_hwndSearchBox, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szSEARCHKEYArr[my_i] );
							}
							::SendMessage( m_hwndSearchBox, CB_SETCURSEL, 0, 0 );
						}
						break;

					default:
						//width = 0;
						//my_hwnd = NULL;
						//my_font = NULL;
						break;
					}
				}
				break;

			case TBSTYLE_BUTTON:	//ボタン
			case TBSTYLE_SEP:		//セパレータ
			default:
				::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&tbb );
				count++;
				break;
			}
			//@@@ 2002.06.15 MIK end
		}
		if( m_pShareData->m_Common.m_bToolBarIsFlat ){	/* フラットツールバーにする／しない */
			uToolType = (UINT)::GetWindowLong(m_hwndToolBar, GWL_STYLE);
			uToolType |= (TBSTYLE_FLAT);
			::SetWindowLong(m_hwndToolBar, GWL_STYLE, (LONG)uToolType);
			::InvalidateRect(m_hwndToolBar, NULL, TRUE);
		}
	}
	return;
}

void CEditWnd::DestroyToolBar( void )
{
	if( m_hwndToolBar )
	{
		if( m_hwndSearchBox )
		{
			if( m_fontSearchBox )
			{
				::DeleteObject( m_fontSearchBox );
				m_fontSearchBox = NULL;
			}

			::DestroyWindow( m_hwndSearchBox );
			m_hwndSearchBox = NULL;

			m_nCurrentFocus = 0;
		}

		::DestroyWindow( m_hwndToolBar );
		m_hwndToolBar = NULL;
	}

	return;
}


//複数プロセス版
/* メッセージループ */
void CEditWnd::MessageLoop( void )
{
	MSG	msg;
	while ( NULL != m_hWnd && GetMessage( &msg, NULL, 0, 0 ) ){
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		if( m_pPrintPreview && NULL != m_pPrintPreview->GetPrintPreviewBarHANDLE() && ::IsDialogMessage( m_pPrintPreview->GetPrintPreviewBarHANDLE(), &msg ) ){	/* 印刷プレビュー 操作バー */
		}else
		if( NULL != m_cEditDoc.m_cDlgFind.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cDlgFind.m_hWnd, &msg ) ){	/* 「検索」ダイアログ */
		}else
		if( NULL != m_cEditDoc.m_cDlgFuncList.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cDlgFuncList.m_hWnd, &msg ) ){	/* 「アウトライン」ダイアログ */
		}else
		if( NULL != m_cEditDoc.m_cDlgReplace.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cDlgReplace.m_hWnd, &msg ) ){	/* 「置換」ダイアログ */
		}else
		if( NULL != m_cEditDoc.m_cDlgGrep.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cDlgGrep.m_hWnd, &msg ) ){	/* 「Grep」ダイアログ */
		}else
		if( NULL != m_cEditDoc.m_cHokanMgr.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cHokanMgr.m_hWnd, &msg ) ){	/* 「入力補完」 */
		}else
		if( NULL != m_hwndSearchBox && ::IsDialogMessage( m_hwndSearchBox, &msg ) ){	/* 「検索ボックス」 */
			ProcSearchBox( &msg );
		}else
		{
			if( NULL != m_pShareData->m_hAccel ){
				if( TranslateAccelerator( msg.hwnd, m_pShareData->m_hAccel, &msg ) ){
				}else{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
			}else{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
	}
	return;
}


LRESULT CEditWnd::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	int					nRet;
	int					idCtrl;
	LPNMHDR				pnmh;
	LPTOOLTIPTEXT		lptip;
	int					nPane;
	FileInfo*			pfi;
	WORD				fActive;
	BOOL				fMinimized;
	HWND				hwndTarget;
	HWND				hwndActive;
	BOOL				bIsActive;
	int					nCaretPosX;
	int					nCaretPosY;
	POINT*				ppoCaret;
	LPHELPINFO			lphi;
	const char*			pLine;
	int					nLineLen;

	UINT				idCtl;	/* コントロールのID */
	MEASUREITEMSTRUCT*	lpmis;
	char				szLabel[1024];
	LPDRAWITEMSTRUCT	lpdis;	/* 項目描画情報 */
	int					nItemWidth;
	int					nItemHeight;
	UINT				uItem;
	UINT				fuFlags;
	HMENU				hmenu;
	const char*			pszItemStr;
	LRESULT				lRes;

	switch( uMsg ){
	case WM_PAINTICON:
//		MYTRACE( "WM_PAINTICON\n" );
		return 0;
	case WM_ICONERASEBKGND:
//		MYTRACE( "WM_ICONERASEBKGND\n" );
		return 0;
	case WM_LBUTTONDOWN:
		return OnLButtonDown( wParam, lParam );
	case WM_MOUSEMOVE:
		return OnMouseMove( wParam, lParam );
	case WM_LBUTTONUP:
		return OnLButtonUp( wParam, lParam );
	case WM_MOUSEWHEEL:
		return OnMouseWheel( wParam, lParam );
	case WM_HSCROLL:
		return OnHScroll( wParam, lParam );
	case WM_VSCROLL:
		return OnVScroll( wParam, lParam );


	case WM_MENUCHAR:
		/* メニューアクセスキー押下時の処理(WM_MENUCHAR処理) */
		return m_CMenuDrawer.OnMenuChar( hwnd, uMsg, wParam, lParam );






	case WM_MENUSELECT:
		if( NULL == m_hwndStatusBar ){
			return 1;
		}
		uItem = (UINT) LOWORD(wParam);		// menu item or submenu index
		fuFlags = (UINT) HIWORD(wParam);	// menu flags
		hmenu = (HMENU) lParam;				// handle to menu clicked
		{
			/* メニュー機能のテキストをセット */
			CMemory		cmemWork;
			pszItemStr = "";

			/* 機能に対応するキー名の取得(複数) */
			CMemory**	ppcAssignedKeyList;
			int			nAssignedKeyNum;
			int			j;
			nAssignedKeyNum = CKeyBind::GetKeyStrList(
				m_hInstance, m_pShareData->m_nKeyNameArrNum,
				(KEYDATA*)m_pShareData->m_pKeyNameArr, &ppcAssignedKeyList, uItem
			);
			if( 0 < nAssignedKeyNum ){
				for( j = 0; j < nAssignedKeyNum; ++j ){
					if( j > 0 ){
						cmemWork.AppendSz( " , " );
					}
					cmemWork.Append( ppcAssignedKeyList[j] );
					delete ppcAssignedKeyList[j];
				}
				delete [] ppcAssignedKeyList;
			}
			pszItemStr = cmemWork.GetPtr();


			::SendMessage( m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)pszItemStr );


		}
		return 0;


	case WM_DRAWITEM:
		idCtl = (UINT) wParam;				/* コントロールのID */
		lpdis = (DRAWITEMSTRUCT*) lParam;	/* 項目描画情報 */
		if( IDW_STATUSBAR == idCtl ){
			if( 4 == lpdis->itemID ){
				int	nColor;
				if( m_pShareData->m_bRecordingKeyMacro	/* キーボードマクロの記録中 */
				 && m_pShareData->m_hwndRecordingKeyMacro == m_hWnd	/* キーボードマクロを記録中のウィンドウ */
				){
					nColor = COLOR_BTNTEXT;
				}else{
					nColor = COLOR_3DSHADOW;
				}
				::SetTextColor( lpdis->hDC, ::GetSysColor( nColor ) );
				::SetBkMode( lpdis->hDC, TRANSPARENT );
				::TextOut( lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top + 2, "REC", lstrlen( "REC" ) );
				if( COLOR_BTNTEXT == nColor ){
					::TextOut( lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 2, "REC", lstrlen( "REC" ) );
				}
			}
			return 0;
		}else{
			switch( lpdis->CtlType ){
			case ODT_MENU:	/* オーナー描画メニュー */
				/* メニューアイテム描画 */
				m_CMenuDrawer.DrawItem( lpdis );
				return TRUE;
			}
		}
		return FALSE;
	case WM_MEASUREITEM:
		idCtl = (UINT) wParam;					// control identifier
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




	case WM_PAINT:
		return OnPaint( hwnd, uMsg, wParam, lParam );

	case WM_PASTE:
		return m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_PASTE, TRUE, 0, 0, 0, 0 );

	case WM_COPY:
		return m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_COPY, TRUE, 0, 0, 0, 0 );

	case WM_HELP:
		lphi = (LPHELPINFO) lParam;
		switch( lphi->iContextType ){
		case HELPINFO_MENUITEM:
			OnHelp_MenuItem( hwnd, lphi->iCtrlId );
			break;
		}
		return TRUE;

		//	Jun. 2, 2000 genta
	case WM_ACTIVATEAPP:
		fActive = LOWORD(wParam);				// activation flag
		if( fActive ){
			::SetFocus( m_hWnd );
		}
		return 0;	//	should return zero. / Jun. 23, 2000 genta
	case WM_ACTIVATE:
		fActive = LOWORD( wParam );				// activation flag
		fMinimized = (BOOL) HIWORD( wParam );	// minimized flag
		hwndTarget = (HWND) lParam;				// window handle

//		MYTRACE( "WM_ACTIVATE " );
 		switch( fActive ){
		case WA_ACTIVE:
//			MYTRACE( " WA_ACTIVE\n" );
			bIsActive = TRUE;
			break;
		case WA_CLICKACTIVE:
//			MYTRACE( " WA_CLICKACTIVE\n" );
			bIsActive = TRUE;
			break;
		case WA_INACTIVE:
//			MYTRACE( " WA_INACTIVE\n" );
			bIsActive = FALSE;
			break;
		}
		if( !bIsActive ){
			hwndActive = hwndTarget;
			while( hwndActive != NULL ){
				hwndActive = ::GetParent( hwndActive );
				if( hwndActive == m_hWnd ){
					bIsActive = TRUE;
					break;
				}
			}
		}
		if( !bIsActive ){
			if( m_hWnd == ::GetWindow( hwndTarget, GW_OWNER ) ){
				bIsActive = TRUE;
			}
		}
		m_cEditDoc.SetParentCaption( !bIsActive );


		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_SIZE:
//		MYTRACE( "WM_SIZE\n" );
		/* WM_SIZE 処理 */
		if( SIZE_MINIMIZED == wParam ){
			m_cEditDoc.SetParentCaption();
		}
		return OnSize( wParam, lParam );

	case WM_IME_COMPOSITION:
		if ( lParam & GCS_RESULTSTR ) {
			/* メッセージの配送 */
			return m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );
		}else{
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
	case WM_KILLFOCUS:
	case WM_CHAR:
	case WM_IME_CHAR:
	case WM_KEYUP:
	case WM_ENTERMENULOOP:
	case MYWM_IME_REQUEST:   /*  再変換対応 by minfu 2002.03.27  */ // 20020331 aroka
		/* メッセージの配送 */
		return m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );

	case WM_EXITMENULOOP:
//		MYTRACE( "WM_EXITMENULOOP\n" );
		if( NULL != m_hwndStatusBar ){
			::SendMessage( m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)"" );
		}

		/* メッセージの配送 */
		return m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );
	case WM_SETFOCUS:
//		MYTRACE( "WM_SETFOCUS\n" );

		/* ファイルのタイムスタンプのチェック処理 */
		m_cEditDoc.CheckFileTimeStamp();


		/* 編集ウィンドウリストへの登録 */
		CShareData::getInstance()->AddEditWndList( m_hWnd );
		/* メッセージの配送 */
		lRes = m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		/* 印刷プレビューモードのときは、キー操作は全部PrintPreviewBarへ転送 */
		if( m_pPrintPreview ){
			m_pPrintPreview->SetFocusToPrintPreviewBar();
		}

		//検索ボックスを更新
		if( m_hwndSearchBox )
		{
			int	i;
			::SendMessage( m_hwndSearchBox, CB_RESETCONTENT, 0, 0 );
			for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; i++ )
			{
				::SendMessage( m_hwndSearchBox, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szSEARCHKEYArr[i] );
			}
			::SendMessage( m_hwndSearchBox, CB_SETCURSEL, 0, 0 );
		}
		
		return lRes;


	case WM_NOTIFY:
		idCtrl = (int) wParam;
		pnmh = (LPNMHDR) lParam;
		switch( pnmh->code ){
		case TTN_NEEDTEXT:
			lptip = (LPTOOLTIPTEXT)pnmh;
			{
				/* ツールバーのツールチップのテキストをセット */
				CMemory**	ppcAssignedKeyList;
				int			nAssignedKeyNum;
				int			j;
				char*		pszKey;

				// From Here Oct. 15, 2001 genta
				// 機能文字列の取得にLookupを使うように変更
				if( !m_cEditDoc.m_cFuncLookup.Funccode2Name( lptip->hdr.idFrom, szLabel, 1024 )){
					szLabel[0] = '\0';
				}
				// To Here Oct. 15, 2001 genta
				/* 機能に対応するキー名の取得(複数) */
				nAssignedKeyNum = CKeyBind::GetKeyStrList(
					m_hInstance, m_pShareData->m_nKeyNameArrNum,
					(KEYDATA*)m_pShareData->m_pKeyNameArr, &ppcAssignedKeyList, lptip->hdr.idFrom
				);
				if( 0 < nAssignedKeyNum ){
					for( j = 0; j < nAssignedKeyNum; ++j ){
						strcat( szLabel, "\n        " );
						pszKey = ppcAssignedKeyList[j]->GetPtr();
						strcat( szLabel, pszKey );
						delete ppcAssignedKeyList[j];
					}
					delete [] ppcAssignedKeyList;
				}
				lptip->hinst = NULL;
				lptip->lpszText	= szLabel;
			}
			break;

		case TBN_DROPDOWN:
			{
				int	nId;
				nId = CreateFileDropDownMenu( pnmh->hwndFrom );
				if( nId != 0 ) OnCommand( (WORD)0 /*メニュー*/, (WORD)nId, (HWND)0 );
			}
			return FALSE;
		}
		return 0L;
	case WM_COMMAND:
		OnCommand( HIWORD(wParam), LOWORD(wParam), (HWND) lParam );
		return 0L;
	case WM_INITMENUPOPUP:
		InitMenu( (HMENU)wParam, (UINT)LOWORD( lParam ), (BOOL)HIWORD( lParam ) );
		return 0L;
	case WM_DROPFILES:
		/* ファイルがドロップされた */
		OnDropFiles( (HDROP) wParam );
		return 0L;
	case WM_QUERYENDSESSION:
		if( OnClose() ){
			DestroyWindow( hwnd );
			return TRUE;
		}else{
			return FALSE;
		}
	case WM_CLOSE:
		if( OnClose() ){
			DestroyWindow( hwnd );
		}
		return 0L;
	case WM_DESTROY:
		if( m_pShareData->m_bRecordingKeyMacro ){					/* キーボードマクロの記録中 */
			if( m_pShareData->m_hwndRecordingKeyMacro == m_hWnd ){	/* キーボードマクロを記録中のウィンドウ */
				m_pShareData->m_bRecordingKeyMacro = FALSE;			/* キーボードマクロの記録中 */
				m_pShareData->m_hwndRecordingKeyMacro = NULL;		/* キーボードマクロを記録中のウィンドウ */
			}
		}

		/* タイマーを削除 */
		::KillTimer( m_hWnd, IDT_TOOLBAR );

		/* ドロップされたファイルを受け入れるのを解除 */
		::DragAcceptFiles( hwnd, FALSE );
		/* 編集ウィンドウリストからの削除 */
		CShareData::getInstance()->DeleteEditWndList( m_hWnd );

		if( m_pShareData->m_hwndDebug == m_hWnd ){
			m_pShareData->m_hwndDebug = NULL;
		}
		m_hWnd = NULL;


		/* 編集ウィンドウオブジェクトからのオブジェクト削除要求 */
		::PostMessage( m_pShareData->m_hwndTray, MYWM_DELETE_ME, 0, 0 );

		/* Windows にスレッドの終了を要求します */
		::PostQuitMessage( 0 );

		return 0L;
	case MYWM_CLOSE:
		/* エディタへの全終了要求 */
		if( FALSE != ( nRet = OnClose()) ){	// Jan. 23, 2002 genta 警告抑制
			DestroyWindow( hwnd );
		}
		return nRet;


	case MYWM_GETFILEINFO:
		/* トレイからエディタへの編集ファイル名要求通知 */
		pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

		/* 編集ファイル情報を格納 */
		m_cEditDoc.SetFileInfo( pfi );
		return 0L;
	case MYWM_CHANGESETTING:
		/* 設定変更の通知 */
// Oct 10, 2000 ao
/* 設定変更時、ツールバーを再作成するようにする */
		if( NULL != m_hwndToolBar ){
			DestroyToolBar();
			CreateToolBar();
		}
// Oct 10, 2000 ao ここまで

		if( NULL != m_CFuncKeyWnd.m_hWnd ){
			BOOL bSizeBox;
			if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 ){	/* ファンクションキー表示位置／0:上 1:下 */
				/* サイズボックスの表示／非表示切り替え */
				bSizeBox = FALSE;
			}else{
				bSizeBox = TRUE;
				/* ステータスパーを表示している場合はサイズボックスを表示しない */
				if( NULL != m_hwndStatusBar ){
					bSizeBox = FALSE;
				}
			}
			m_CFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
		}

		//	Aug, 21, 2000 genta
		m_cEditDoc.ReloadAutoSaveParam();

//1999.09.03 なーんかうまく動かんので、やめた。運用でカバーしてほしい。
//
//ファンクションキー窓位置変更したとき、サイズボックスの位置を変更
//		m_cEditDoc.m_cSplitterWnd.DoSplit( -1, -1);
//ファンクションキー窓位置変更したとき、ファンクションキー窓位置変更
//		::GetWindowRect( hwnd, &rc );
//		::SetWindowPos( hwnd, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top + 1, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
//		::SetWindowPos( hwnd, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );

		m_cEditDoc.SetDocumentIcon();	// Sep. 10, 2002 genta 文書アイコンの再設定
		m_cEditDoc.OnChangeSetting();	/* ビューに設定変更を反映させる */
		return 0L;
	case MYWM_SETACTIVEPANE:
		if( -1 == (int)wParam ){
			if( 0 == lParam ){
				nPane = m_cEditDoc.m_cSplitterWnd.GetFirstPane();
			}else{
				nPane = m_cEditDoc.m_cSplitterWnd.GetLastPane();
			}
		}
		m_cEditDoc.SetActivePane( nPane );
		return 0L;


	case MYWM_SETCARETPOS:	/* カーソル位置変更通知 */
		ppoCaret = (POINT*)m_pShareData->m_szWork;
		/* 範囲選択中か */
		if( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].DisableSelectArea( TRUE );
		}
		/*
		カーソル位置変換
		 物理位置(行頭からのバイト数、折り返し無し行位置)
		→
		 レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		*/
		m_cEditDoc.m_cLayoutMgr.CaretPos_Phys2Log(
			ppoCaret->x,
			ppoCaret->y,
			&nCaretPosX,
			&nCaretPosY
		);
		/* カーソル移動 */
		if( nCaretPosY >= m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
			/*ファイルの最後に移動 */
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_GOFILEEND, FALSE, 0, 0, 0, 0 );
		}else{
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].MoveCursor( nCaretPosX, nCaretPosY, TRUE, _CARETMARGINRATE / 3 );
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosX_Prev =
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosX;
		}
		return 0L;


	case MYWM_GETCARETPOS:	/* カーソル位置取得要求 */
		ppoCaret = (POINT*)m_pShareData->m_szWork;
		/*
		カーソル位置変換
		 レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		→
		物理位置(行頭からのバイト数、折り返し無し行位置)
		*/
		m_cEditDoc.m_cLayoutMgr.CaretPos_Log2Phys(
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosX,
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosY,
			(int*)&ppoCaret->x,
			(int*)&ppoCaret->y
		);
		return 0L;

	case MYWM_GETLINEDATA:	/* 行(改行単位)データの要求 */
		pLine = m_cEditDoc.m_cDocLineMgr.GetLineStr( (int)wParam, &nLineLen );
		if( NULL == pLine ){
			return 0;
		}
		if( nLineLen > sizeof( m_pShareData->m_szWork ) ){
			memcpy( m_pShareData->m_szWork, pLine, sizeof( m_pShareData->m_szWork ) );
		}else{
			memcpy( m_pShareData->m_szWork, pLine, nLineLen );
		}
		return nLineLen;


	case MYWM_ADDSTRING:
//		MYTRACE( "MYWM_ADDSTRING[%s]\n", m_pShareData->m_szWork );
		m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_ADDTAIL, TRUE, (LPARAM)m_pShareData->m_szWork, (LPARAM)lstrlen( m_pShareData->m_szWork ), 0, 0 );
		m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_GOFILEEND, TRUE, 0, 0, 0, 0 );
		return 0L;
	case MYWM_SETREFERER:
		ppoCaret = (POINT*)m_pShareData->m_szWork;

		m_cEditDoc.SetReferer( (HWND)wParam, ppoCaret->x, ppoCaret->y );
		return 0L;

	//by 鬼 (2) MYWM_CHECKSYSMENUDBLCLKは不要に, WM_LBUTTONDBLCLK追加
	case WM_NCLBUTTONDOWN:
		return OnNcLButtonDown(wParam, lParam);

	case WM_NCLBUTTONUP:
		return OnNcLButtonUp(wParam, lParam);

	case WM_LBUTTONDBLCLK:
		return OnLButtonDblClk(wParam, lParam);

	default:
// << 20020331 aroka 再変換対応 for 95/NT
		if( uMsg == m_uMSIMEReconvertMsg || uMsg == m_uATOKReconvertMsg){
			return m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );
		}
// >> by aroka
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}

/*! 終了時の処理

	@retval TRUE: 終了して良い / FALSE: 終了しない
*/
int	CEditWnd::OnClose( void )
{
	/* ウィンドウをアクティブにする */
	/* アクティブにする */
	ActivateFrameWindow( m_hWnd );
//	if( ::IsIconic( m_hWnd ) ){
//		::ShowWindow( m_hWnd, SW_RESTORE );
//	}else{
//		::ShowWindow( m_hWnd, SW_SHOW );
//	}
//	::SetForegroundWindow( m_hWnd );
//	::SetActiveWindow( m_hWnd );
	/* ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行 */
	return m_cEditDoc.OnFileClose();
}






void CEditWnd::OnCommand( WORD wNotifyCode, WORD wID , HWND hwndCtl )
{
//	MYTRACE( "CEditWnd::OnCommand()\n" );

	static			CHOOSEFONT cf;
	CMemory			cMemKeyList;
	int				nFuncCode;
	HWND			hwndWork;
	BOOL			bOpened;
	FileInfo*		pfi;
	HWND			hWndOwner;
	static char		szURL[1024];

	switch( wNotifyCode ){
	/* メニューからのメッセージ */
	case 0:
		switch( wID ){
		case F_EXITALL:	//Dec. 26, 2000 JEPRO F_に変更
			/* サクラエディタの全終了 */
			CEditApp::TerminateApplication();
			break;
//Sept. 15, 2000→Nov. 25, 2000 JEPRO //ショートカットキーがうまく働かないので殺してあった下の2行を修正・復活
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
		case F_ABOUT:	//Dec. 25, 2000 JEPRO F_に変更
			/* バージョン情報 */
			{
				CDlgAbout cDlgAbout;
				cDlgAbout.DoModal( m_hInstance, m_hWnd );

#ifdef _DEBUG
				HRESULT hres;
				char szPath[_MAX_PATH + 1];
				/* ショートカット(.lnk)の解決 */
				hres = ::ResolveShortcutLink( m_hWnd, "C:\\WINDOWS\\All Users\\ﾃﾞｽｸﾄｯﾌﾟ\\Outlook Express.LNK", szPath );
				MYTRACE( "hres=%xh, szPath=[%s]\n", hres, szPath );

				/* ショートカット(.lnk)の解決 */
				hres = ::ResolveShortcutLink( m_hWnd, "C:\\My Documents\\develop\\EDIT\\sakura\\CDlgCompare.cpp", szPath );
				MYTRACE( "hres=%xh, szPath=[%s]\n", hres, szPath );

				/* パス名に対するアイテムＩＤリストを作成する */
				BOOL bRes;
				ITEMIDLIST* pIDL;
				pIDL = CreateItemIDList( "C:\\WINDOWS\\ALLUSE~1\\ﾃﾞｽｸﾄｯﾌﾟ\\OUTLOO~1.LNK" );
//				pIDL = CreateItemIDList( "\\\\NPSV-NT5\\newpat-src\\nwmsc4\\ZENBUN~1\\ZENBUN~1.RC" );
//				pIDL = CreateItemIDList( "C:\\MYDOCU~1\\DEVELOP\\ZENBUN~1\\CDLGSE~2.CPP" );

				// SHGetPathFromIDList()関数はアイテムＩＤリストの物理パスを探してくれる
				bRes = ::SHGetPathFromIDList( pIDL, szPath );
				MYTRACE( "szPath=[%s]\n", szPath );

				/* アイテムＩＤリストを削除する */
				bRes = DeleteItemIDList( pIDL );
				MYTRACE( "bRes=%d\n", bRes );
#endif
			}
			break;
		default:
			if( wID - IDM_SELWINDOW >= 0 &&
				wID - IDM_SELWINDOW < m_pShareData->m_nEditArrNum ){
				hwndWork = m_pShareData->m_pEditArr[wID - IDM_SELWINDOW].m_hWnd;
				/* アクティブにする */
				ActivateFrameWindow( hwndWork );
			}else
			if( wID - IDM_SELMRU >= 0 &&
				wID - IDM_SELMRU < 999
			){
				/* 指定ファイルが開かれているか調べる */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
				CMRU cMRU;
				FileInfo checkFileInfo;
				cMRU.GetFileInfo(wID - IDM_SELMRU, &checkFileInfo);
				if( CShareData::getInstance()->IsPathOpened( checkFileInfo.m_szPath, &hWndOwner ) ){

					::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
					pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

					/* アクティブにする */
					ActivateFrameWindow( hWndOwner );
					/* MRUリストへの登録 */
					cMRU.Add( pfi );
				}else{
					/* 変更フラグがオフで、ファイルを読み込んでいない場合 */
//@@@ 2002.01.08 YAZAKI Grep結果で無い場合も含める。
					if( !m_cEditDoc.IsModified() &&
						!m_cEditDoc.IsFilePathAvailable() && 	/* 現在編集中のファイルのパス */
						!m_cEditDoc.m_bGrepMode	//	さらに、Grepモードじゃない。
					){
						/* ファイル読み込み */
						m_cEditDoc.FileRead(
							checkFileInfo.m_szPath,
							&bOpened,
							checkFileInfo.m_nCharCode,
							FALSE,	/* 読み取り専用か */
							TRUE	/* 文字コード変更時の確認をするかどうか */
						);
					}else{
						/* 新たな編集ウィンドウを起動 */
						//	From Here Oct. 27, 2000 genta	カーソル位置を復元しない機能
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
						CMRU cMRU;
						FileInfo openFileInfo;
						cMRU.GetFileInfo(wID - IDM_SELMRU, &openFileInfo);
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
					}
				}
			}else
			if( wID - IDM_SELOPENFOLDER >= 0 &&
				wID - IDM_SELOPENFOLDER < 999
			){
				{
					char*		pszPath = new char[_MAX_PATH];
					BOOL		bOpened;
					int			nCharCode;
					BOOL		bReadOnly;
					FileInfo*	pfi;
					HWND		hWndOwner;

					strcpy( pszPath, "" );

					//Stonee, 2001/12/21 UNCであれば接続を試みる
//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
					CMRUFolder cMRUFolder;
					NetConnect( cMRUFolder.GetPath( wID - IDM_SELOPENFOLDER ) );

					/* 「ファイルを開く」ダイアログ */
					nCharCode = CODE_AUTODETECT;	/* 文字コード自動判別 */
					bReadOnly = FALSE;
					if( !m_cEditDoc.OpenFileDialog( m_hWnd, cMRUFolder.GetPath(wID - IDM_SELOPENFOLDER), pszPath, &nCharCode, &bReadOnly ) ){
						return;
					}
					/* 指定ファイルが開かれているか調べる */
					if( CShareData::getInstance()->IsPathOpened( pszPath, &hWndOwner ) ){
						::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
						pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

						if( CODE_AUTODETECT == nCharCode ){	/* 文字コード自動判別 */
							int nCharCodeNew; 
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
							nCharCodeNew = CMemory::CheckKanjiCodeOfFile( pszPath );
							if( -1 == nCharCodeNew ){

							}else{
								nCharCode = nCharCodeNew;
							}
						}
						if( nCharCode != pfi->m_nCharCode ){	/* 文字コード種別 */
							char*	pszCodeNameCur;
							char*	pszCodeNameNew;
							// gm_pszCodeNameArr_1 を使うように変更 Moca. 2002/05/26
							if( -1 < pfi->m_nCharCode && pfi->m_nCharCode < CODE_CODEMAX ){
								pszCodeNameCur = (char*)gm_pszCodeNameArr_1[pfi->m_nCharCode];
							}
							if( -1 < nCharCode && nCharCode < CODE_CODEMAX ){
								pszCodeNameNew = (char*)gm_pszCodeNameArr_1[nCharCode];
							}
							::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
								"%s\n\n\n既に開いているファイルを違う文字コードで開く場合は、\n一旦閉じてから開いてください。\n\n現在の文字コードセット=[%s]\n新しい文字コードセット=[%s]",
								pszPath, pszCodeNameCur, pszCodeNameNew
							);
						}
						/* 自分が開いているか */
						if( 0 == strcmp( m_cEditDoc.GetFilePath(), pszPath ) ){
							/* 何もしない */
						}else{
							/* 開いているウィンドウをアクティブにする */
							/* アクティブにする */
							ActivateFrameWindow( hWndOwner );
						}
					}else{
						/* ファイルが開かれていない */
						/* 変更フラグがオフで、ファイルを読み込んでいない場合 */
						if( !m_cEditDoc.IsModified() &&
							!m_cEditDoc.IsFilePathAvailable() &&		/* 現在編集中のファイルのパス */
							//	Jun. 13, 2003 Moca GrepウィンドウへDropしたときにGrepウィンドウのまま
							//	同じウィンドウで開かれてしまう問題を修正
							!m_cEditDoc.m_bGrepMode					/* Grep結果ではない */
						){
							/* ファイル読み込み */
							m_cEditDoc.FileRead( pszPath, &bOpened, nCharCode, bReadOnly,
								TRUE	/* 文字コード変更時の確認をするかどうか */
							);
						}else{
							if( strchr( pszPath, ' ' ) ){
								char	szFile2[_MAX_PATH + 3];
								wsprintf( szFile2, "\"%s\"", pszPath );
								strcpy( pszPath, szFile2 );
							}
							/* 新たな編集ウィンドウを起動 */
							CEditApp::OpenNewEditor( m_hInstance, m_hWnd, pszPath, nCharCode, bReadOnly );
						}
					}
					delete [] pszPath;
				}
			}else{
				//ビューにフォーカスを移動しておく
				if( wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX )
					::SetFocus( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_hWnd );

				/* コマンドコードによる処理振り分け */
				m_cEditDoc.HandleCommand( wID );
			}
			break;
		}
		break;
	/* アクセラレータからのメッセージ */
	case 1:
		//ビューにフォーカスを移動しておく
		if( wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX )
			::SetFocus( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_hWnd );

		nFuncCode = CKeyBind::GetFuncCode(
			wID,
			m_pShareData->m_nKeyNameArrNum,
			m_pShareData->m_pKeyNameArr
		);
//		MYTRACE( "CEditWnd::OnCommand()  nFuncCode=%d\n", nFuncCode );
		m_cEditDoc.HandleCommand( nFuncCode );
		break;

	case CBN_SETFOCUS:
		if( NULL != m_hwndSearchBox && hwndCtl == m_hwndSearchBox )
		{
			m_nCurrentFocus = F_SEARCH_BOX;
		}
		break;

	case CBN_KILLFOCUS:
		if( NULL != m_hwndSearchBox && hwndCtl == m_hwndSearchBox )
		{
			m_nCurrentFocus = 0;

			//フォーカスがはずれたときに検索キーにしてしまう。
			//検索キーワードを取得
			char	szText[_MAX_PATH];
			memset( szText, 0, sizeof(szText) );
			::SendMessage( m_hwndSearchBox, WM_GETTEXT, _MAX_PATH - 1, (LPARAM)szText );
			if( szText[0] )	//キー文字列がある
			{
				//検索キーを登録
				CShareData::getInstance()->AddToSearchKeyArr( (const char*)szText );
			}

		}
		break;

	/* コントロールからのメッセージには通知コード */
	default:
		break;
	}
	return;
}



//	キーワード：メニューバー順序
//	Sept.14, 2000 Jepro note: メニューバーの項目のキャプションや順番設定などは以下で行っているらしい
//	Sept.16, 2000 Jepro note: アイコンとの関連付けはCShareData_new2.cppファイルで行っている
void CEditWnd::InitMenu( HMENU hMenu, UINT uPos, BOOL fSystemMenu )
{
	int			cMenuItems;
	int			nPos;
	UINT		id;
	UINT		fuFlags;
	int			i;
	BOOL		bRet;
	char		szMemu[280];
	int			nRowNum;
	EditNode*	pEditNodeArr;
	FileInfo*	pfi;

	HMENU		hMenuPopUp;
	HMENU		hMenuPopUp_2;
	const char*	pszLabel;


	if( hMenu != ::GetSubMenu( ::GetMenu( m_hWnd ), uPos ) ){
		goto end_of_func_IsEnable;
	}


	if( fSystemMenu ){
	}else{
//		MYTRACE( "hMenu=%08xh uPos=%d, fSystemMenu=%s\n", hMenu, uPos, fSystemMenu ? "TRUE":"FALSE" );
		switch( uPos ){
		case 0:
			/* 「ファイル」メニュー */
			m_CMenuDrawer.ResetContents();
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW			, "新規作成(&N)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN		, "開く(&O)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVE		, "上書き保存(&S)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEAS_DIALOG	, "名前を付けて保存(&A)..." );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILECLOSE		, "閉じて(無題) (&B)" );	//Oct. 17, 2000 jepro キャプションを「閉じる」から変更	//Feb. 18, 2001 JEPRO アクセスキー変更(C→B; Blankの意味)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILECLOSE_OPEN	, "閉じて開く(&L)..." );
			//	Dec. 4, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN	, "開き直す(&W)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINCLOSE		, "ウィンドウを閉じる(&C)" );	//Feb. 18, 2001	JEPRO 追加

			// 「文字コードセット」ポップアップメニュー
			hMenuPopUp_2 = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_SJIS, "&SJISで開き直す" );		//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_JIS, "&JISで開き直す" );			//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_EUC, "&EUCで開き直す" );			//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UNICODE, "&Unicodeで開き直す" );	//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UNICODEBE, "U&nicodeBEで開き直す" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UTF8, "UTF-&8で開き直す" );		//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UTF7, "UTF-&7で開き直す" );		//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp_2 , "文字コードセット(&H)" );//Oct. 11, 2000 JEPRO アクセスキー変更(M→H)

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT				, "印刷(&P)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT_PREVIEW		, "印刷プレビュー(&V)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT_PAGESETUP		, "ページ設定(&U)..." );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			// 「ファイル操作」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_OPEN_HfromtoC				, "同名のC/C++ヘッダ(ソース)を開く(&C)" );	//Feb. 7, 2001 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ACTIVATE_SQLPLUS			, "SQL*Plusをアクティブ表示(&A)" );	//Sept. 11, 2000 JEPRO アクセスキー付与	説明の「アクティブ化」を「アクティブ表示」に統一
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_PLSQL_COMPILE_ON_SQLPLUS	, "SQL*Plusで実行(&X)" );			//Sept. 11, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BROWSE						, "ブラウズ(&B)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_READONLY					, "読み取り専用(&O)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_PROPERTY_FILE				, "ファイルのプロパティ(&R)" );		//Nov. 7, 2000 jepro キャプションに'ファイルの'を追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "ファイル操作(&R)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			/* MRUリストのファイルのリストをメニューにする */
			{
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
			}

			/* 最近使ったフォルダのメニューを作成 */
			{
				//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
				CMRUFolder cMRUFolder;
				hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
				if (cMRUFolder.Length() > 0){
					//	アクティブ
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "最近使ったフォルダ(&D)" );
				}
				else {
					//	非アクティブ
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , "最近使ったフォルダ(&D)" );
				}
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_CLOSEALL	, "すべてのウィンドウを閉じる(&Q)" );	//Feb/ 19, 2001 JEPRO 追加
			//	Jun. 9, 2001 genta ソフトウェア名改称
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL			, "サクラエディタの全終了(&X)" );	//Sept. 11, 2000 jepro キャプションを「アプリケーション終了」から変更	//Dec. 26, 2000 JEPRO F_に変更
			break;

		case 1:
			/* 「編集」メニュー */
			m_CMenuDrawer.ResetContents();
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UNDO		, "元に戻す(&Undo)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REDO		, "やり直し(&Redo)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CUT			, "切り取り(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY		, "コピー(&C)" );
			//	Jul, 3, 2000 genta
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE, "全行引用コピー(&N)" );
			//	Sept. 14, 2000 JEPRO	キャプションに「記号付き」を追加、アクセスキー変更(N→.)
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE, "選択範囲内全行引用符付きコピー(&.)" );
//			Sept. 30, 2000 JEPRO	引用符付きコピーのアイコンを作成したので上記メニューは重複を避けて「高度な操作」内におくだけにする
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PASTE		, "貼り付け(&P)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DELETE		, "削除(&D)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SELECTALL	, "すべて選択(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );							/* メニューからの再変換対応 minfu 2002.04.09 */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECONVERT	, "再変換(&R)" );		/* メニューからの再変換対応 minfu 2002.04.09 */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CHGMOD_INS	, "挿入／上書きモード切り替え(&M)" );	//Nov. 9, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY_CRLF	, "CR&LF改行でコピー" );				//Nov. 9, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY_ADDCRLF	, "折り返し位置に改行をつけてコピー(&H)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PASTEBOX	, "矩形貼り付け(&X)" );					//Sept. 13, 2000 JEPRO 移動に伴いアクセスキー付与	//Oct. 22, 2000 JEPRO アクセスキー変更(P→X)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DELETE_BACK	, "カーソル前を削除(&B)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			// 「挿入」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INS_DATE, "日付(&D)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INS_TIME, "時刻(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CTRL_CODE_DIALOG, "コントロールコード(&C)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "挿入(&I)" );

			// 「高度な操作」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDeleteToStart	,	"単語の左端まで削除(&L)" );			//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDeleteToEnd	,	"単語の右端まで削除(&R)" );			//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTWORD			,	"現在位置の単語選択(&W)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordCut			,	"単語切り取り(&T)" );				//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDelete			,	"単語削除(&D)" );					//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineCutToStart		,	"行頭まで切り取り(改行単位) (&U)" );//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineCutToEnd		,	"行末まで切り取り(改行単位) (&K)" );//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineDeleteToStart	,	"行頭まで削除(改行単位) (&H)" );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineDeleteToEnd	,	"行末まで削除(改行単位) (&E)" );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CUT_LINE			,	"行切り取り(折り返し単位) (&X)" );	//Jan. 16, 2001 JEPRO 行(頭・末)関係の順序を入れ替えた
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DELETE_LINE		,	"行削除(折り返し単位) (&Y)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DUPLICATELINE		,	"行の二重化(折り返し単位) (&2)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INDENT_TAB			,	"TABインデント(&A)" );				//Oct. 22, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UNINDENT_TAB		,	"逆TABインデント(&B)" );			//Oct. 22, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INDENT_SPACE		,	"SPACEインデント(&S)" );			//Oct. 22, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UNINDENT_SPACE		,	"逆SPACEインデント(&P)" );			//Oct. 22, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINES				, "選択範囲内全行コピー(&@)" );		//Sept. 14, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE		, "選択範囲内全行引用符付きコピー(&.)" );//Sept. 13, 2000 JEPRO キャプションから「記号付き」を追加、アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINESWITHLINENUMBER, "選択範囲内全行行番号付きコピー(&:)" );//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYFNAME			,	"このファイル名をコピー(&-)" );	// 2002/2/3 aroka
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYPATH			,	"このファイルのパス名をコピー(&\\)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYTAG			,	"このファイルのパス名とカーソル位置をコピー(&^)" );
//			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_TEST_CREATEKEYBINDLIST	, "キー割り当て一覧をコピー(&Q)" );	//Sept. 15, 2000 JEPRO キャプションの「...リスト」、アクセスキー変更(K→Q)
//Sept. 16, 2000 JEPRO ショートカットキーがうまく働かないので次行は殺して元に戻してある		//Dec. 25, 2000 復活
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CREATEKEYBINDLIST	, "キー割り当て一覧をコピー(&Q)" );			//Sept. 15, 2000 JEPRO キャプションの「...リスト」、アクセスキー変更(K→Q) IDM_TEST→Fに変更
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDSREFERENCE, "単語リファレンス(&W)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "高度な操作(&V)" );

		//From Here Feb. 19, 2001 JEPRO [移動(M)], [選択(R)]メニューを[編集]のサブメニューとして移動
			// 「移動」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UP2		, "カーソル上移動(２行ごと) (&Q)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DOWN2		, "カーソル下移動(２行ごと) (&K)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDLEFT	, "単語の左端に移動(&L)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDRIGHT	, "単語の右端に移動(&R)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOPREVPARAGRAPH	, "前の段落に移動(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GONEXTPARAGRAPH	, "次の段落に移動(&Z)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINETOP	, "行頭に移動(折り返し単位) (&H)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINEEND	, "行末に移動(折り返し単位) (&E)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageUp	, "１ページアップ(&U)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageDown	, "１ページダウン(&D)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILETOP	, "ファイルの先頭に移動(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILEEND	, "ファイルの最後に移動(&B)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CURLINECENTER, "カーソル行をウィンドウ中央へ(&C)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMP_DIALOG, "指定行へジャンプ(&J)..." );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMP_SRCHSTARTPOS, "検索開始位置へ戻る(&I)" );	// 検索開始位置へ戻る 02/06/26 ai
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_PREV	, "移動履歴: 前へ(&P)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_NEXT	, "移動履歴: 次へ(&N)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_SET	, "現在位置を移動履歴に登録(&S)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "移動(&O)" );

			// 「選択」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTWORD		, "現在位置の単語選択(&W)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTALL		, "すべて選択(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BEGIN_SEL		, "範囲選択開始(&S)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UP2_SEL		, "(選択)カーソル上移動(２行ごと) (&Q)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DOWN2_SEL		, "(選択)カーソル下移動(２行ごと) (&K)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDLEFT_SEL	, "(選択)単語の左端に移動(&L)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDRIGHT_SEL	, "(選択)単語の右端に移動(&R)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOPREVPARAGRAPH_SEL	, "(選択)前の段落に移動(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GONEXTPARAGRAPH_SEL	, "(選択)次の段落に移動(&Z)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINETOP_SEL	, "(選択)行頭に移動(折り返し単位) (&H)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINEEND_SEL	, "(選択)行末に移動(折り返し単位) (&E)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageUp_Sel	, "(選択)１ページアップ(&U)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageDown_Sel	, "(選択)１ページダウン(&D)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILETOP_SEL	, "(選択)ファイルの先頭に移動(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILEEND_SEL	, "(選択)ファイルの最後に移動(&B)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "選択(&S)" );

			// 「矩形選択」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BEGIN_BOX	, "矩形範囲選択開始(&S)" );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UP_BOX			, "(矩選)カーソル上移動(&)" );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DOWN_BOX		, "(矩選)カーソル下移動(&)" );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LEFT_BOX		, "(矩選)カーソル左移動(&)" );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RIGHT_BOX		, "(矩選)カーソル右移動(&)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UP2_BOX			, "(矩選)カーソル上移動(２行ごと) (&Q)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DOWN2_BOX		, "(矩選)カーソル下移動(２行ごと) (&K)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDLEFT_BOX	, "(矩選)単語の左端に移動(&L)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDRIGHT_BOX	, "(矩選)単語の右端に移動(&R)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOLINETOP_BOX	, "(矩選)行頭に移動(折り返し単位) (&H)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOLINEEND_BOX	, "(矩選)行末に移動(折り返し単位) (&E)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HalfPageUp_Box	, "(選択)半ページアップ(&)" );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HalfPageDown_Box, "(選択)半ページダウン(&)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_1PageUp_Box		, "(矩選)１ページアップ(&U)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_1PageDown_Box	, "(矩選)１ページダウン(&D)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOFILETOP_BOX	, "(矩選)ファイルの先頭に移動(&T)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOFILEEND_BOX	, "(矩選)ファイルの最後に移動(&B)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "矩形選択(&E)" );

			// 「整形」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LTRIM, "左(先頭)の空白を削除(&L)" );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_RTRIM, "右(末尾)の空白を削除(&R)" );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SORT_ASC, "選択行の昇順ソート(&A)" );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SORT_DESC, "選択行の降順ソート(&D)" );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_MERGE, "選択行のマージ(&M)" );			// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "整形(&K)" );

			break;
		//Feb. 19, 2001 JEPRO [移動][移動], [選択]を[編集]配下に移したので削除

//		case 4://case 2: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
		case 2://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
			m_CMenuDrawer.ResetContents();
			/* 「変換」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOLOWER				, "英大文字→英小文字(&L)" );			//Sept. 10, 2000 jepro キャプションを英語から変更
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOUPPER				, "英小文字→英大文字(&U)" );			//Sept. 10, 2000 jepro キャプションを英語から変更
//	From Here Sept. 18, 2000 JEPRO
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKAKU			, "全角→半角" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUKATA		, "半角→全角カタカナ" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUHIRA		, "半角→全角ひらがな" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKAKU			, "全角→半角(&F)" );					//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKATA			, "全角カタカナ→半角カタカナ" );		//Aug. 29, 2002 ai
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANEI				, "全角英数→半角英数" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENEI				, "半角英数→全角英数" );				//July. 29, 2001 Misaka アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUKATA		, "半角＋全ひら→全角・カタカナ(&Z)" );	//Sept. 13, 2000 JEPRO キャプション変更 & アクセスキー付与 //Oct. 11, 2000 JEPRO キャプション変更
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUHIRA		, "半角＋全カタ→全角・ひらがな(&N)" );	//Sept. 13, 2000 JEPRO キャプション変更 & アクセスキー付与 //Oct. 11, 2000 JEPRO キャプション変更
//	To Here Sept. 18, 2000
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HANKATATOZENKAKUKATA, "半角カタカナ→全角カタカナ(&K)" );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HANKATATOZENKAKUHIRA, "半角カタカナ→全角ひらがな(&H)" );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TABTOSPACE			, "TAB→空白(&S)" );	//Feb. 19, 2001 JEPRO 下から移動した
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPACETOTAB			, "空白→TAB(&T)" );	//#### Stonee, 2001/05/27
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//「文字コード変換」ポップアップ
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_AUTO2SJIS		, "自動判別→SJISコード変換(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_EMAIL			, "E-Mail(JIS→SJIS)コード変換(&M)" );//Sept. 11, 2000 JEPRO キャプションに「E-Mail」を追加しアクセスキー変更(V→M:Mail)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_EUC2SJIS		, "EUC→SJISコード変換(&W)" );		//Sept. 11, 2000 JEPRO アクセスキー変更(E→W:Work Station)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UNICODE2SJIS	, "Unicode→SJISコード変換(&U)" );	//Sept. 11, 2000 JEPRO アクセスキー変更候補はI:shIft
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UNICODEBE2SJIS	, "UnicodeBE→SJISコード変換(&N)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UTF82SJIS		, "UTF-8→SJISコード変換(&T)" );	//Sept. 11, 2000 JEPRO アクセスキー付与(T:uTF/shifT)	//Oct. 6, 2000 簡潔表示にした
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UTF72SJIS		, "UTF-7→SJISコード変換(&F)" );	//Sept. 11, 2000 JEPRO アクセスキー付与(F:utF/shiFt)	//Oct. 6, 2000 簡潔表示にした
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2JIS		, "SJIS→JISコード変換(&J)" );		//Sept. 11, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2EUC		, "SJIS→EUCコード変換(&E)" );		//Sept. 11, 2000 JEPRO アクセスキー付与
//			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UNICODE	, "SJIS→&Unicodeコード変換" );		//Sept. 11, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UTF8		, "SJIS→UTF-8コード変換(&8)" );	//Sept. 11, 2000 JEPRO アクセスキー付与 //Oct. 6, 2000 簡潔表示にした
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UTF7		, "SJIS→UTF-7コード変換(&7)" );	//Sept. 11, 2000 JEPRO アクセスキー付与 //Oct. 6, 2000 簡潔表示にした
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BASE64DECODE			, "Base64デコードして保存(&B)" );	//Oct. 6, 2000 JEPRO アクセスキー変更(6→B)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UUDECODE				, "uudecodeして保存(&D)" );			//Sept. 11, 2000 JEPRO アクセスキー変更(U→D)

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, "文字コード変換(&C)" );

			break;

//		case 5://case 3: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
		case 3://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
			m_CMenuDrawer.ResetContents();
			/* 「検索」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_DIALOG	, "検索(&F)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_NEXT		, "次を検索(&N)" );				//Sept. 11, 2000 JEPRO "次"を"前"の前に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_PREV		, "前を検索(&P)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REPLACE_DIALOG	, "置換(&R)..." );				//Oct. 7, 2000 JEPRO 下のセクションからここに移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_CLEARMARK, "検索マークの切替え(&C)" );	// "検索マークのクリア(&C)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_JUMP_SRCHSTARTPOS, "検索開始位置へ戻る(&I)" );	// 検索開始位置へ戻る 02/06/26 ai
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GREP_DIALOG		, "&Grep..." );					//Oct. 7, 2000 JEPRO 下からここに移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_JUMP_DIALOG		, "指定行へジャンプ(&J)..." );	//Sept. 11, 2000 jepro キャプションに「 ジャンプ」を追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OUTLINE			, "アウトライン解析(&L)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMP			, "タグジャンプ(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMPBACK		, "タグジャンプバック(&B)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGS_MAKE		, "タグファイルの作成" );	//@@@ 2003.04.13 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIRECT_TAGJUMP	, "ダイレクトタグジャンプ" );	//@@@ 2003.04.13 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HOKAN			, "入力補完(&/)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COMPARE			, "ファイル内容比較(&@)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_DIALOG		, "DIFF差分表示(&D)..." );	//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_NEXT		, "次の差分へ" );		//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_PREV		, "前の差分へ" );		//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_RESET		, "差分表示の全解除" );		//@@@ 2002.05.25 MIK
//	From Here Sept. 1, 2000 JEPRO	対括弧の検索をメニューに追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BRACKETPAIR		, "対括弧の検索(&[)" );
//	To Here Sept. 1, 2000
// From Here 2001.12.03 hor
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BOOKMARK_SET	, "ブックマーク設定・解除(&S)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BOOKMARK_NEXT	, "次のブックマークへ(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BOOKMARK_PREV	, "前のブックマークへ(&Z)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BOOKMARK_RESET	, "ブックマークの全解除(&X)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BOOKMARK_VIEW	, "ブックマークの一覧(&V)" );
// To Here 2001.12.03 hor

			break;

//		case 6://case 4: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
		case 4://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
			m_CMenuDrawer.ResetContents();
			/* 「オプション」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
//	From Here Sept. 17, 2000 JEPRO
//	やはりWin標準に合わせてチェックマークだけで表示／非表示を判断するようにした方がいいので変更
			if ( FALSE == m_pShareData->m_Common.m_bMenuIcon ){
				pszLabel = "ツールバーを表示(&T)";				//これのみ表示
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTOOLBAR, pszLabel );	//これのみ
				pszLabel = "ファンクションキーを表示(&K)";		//これのみ表示
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWFUNCKEY, pszLabel );	//これのみ
				pszLabel = "ステータスバーを表示(&S)";			//これのみ表示
//	To Here Sept.17, 2000 JEPRO
//	From Here Oct. 28, 2000 JEPRO
//	3つボタンのアイコンができたことに伴い表示／非表示のメッセージを変えるように再び変更
			}else{
				if( m_hwndToolBar == NULL ){
					pszLabel = "ツールバーを表示(&T)";			//これのみ表示
				}else{
					pszLabel = "表示中のツールバーを隠す(&T)";			//Sept. 9, 2000 jepro キャプションに「表示中の」を追加
				}
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTOOLBAR, pszLabel );	//これのみ
				if( NULL == m_CFuncKeyWnd.m_hWnd ){
					pszLabel = "ファンクションキーを表示(&K)";	//これのみ表示
				}else{
					pszLabel = "表示中のファンクションキーを隠す(&K)";	//Sept. 9, 2000 jepro キャプションに「表示中の」を追加
				}
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWFUNCKEY, pszLabel );	//これのみ
				if( m_hwndStatusBar == NULL ){
					pszLabel = "ステータスバーを表示(&S)";		//これのみ表示
				}else{
					pszLabel = "表示中のステータスバーを隠す(&S)";		//Sept. 9, 2000 jepro キャプションに「表示中の」を追加
				}
			}
//	To Here Oct. 28, 2000

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWSTATUSBAR, pszLabel );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TYPE_LIST		, "タイプ別設定一覧(&L)..." );	//Sept. 13, 2000 JEPRO 設定より上に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION_TYPE		, "タイプ別設定(&Y)..." );		//Sept. 13, 2000 JEPRO アクセスキー変更(S→Y)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION			, "共通設定(&C)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FONT			, "フォント設定(&F)..." );		//Sept. 17, 2000 jepro キャプションに「設定」を追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FAVORITE		, "お気に入りの設定(&O)..." );	//お気に入り	//@@@ 2003.04.08 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
//@@@ 2002.01.14 YAZAKI 折り返さないコマンド追加
			if( m_cEditDoc.GetDocumentAttribute().m_nMaxLineSize == m_cEditDoc.ActiveView().m_nViewColNum ){
				pszLabel = "折り返さない(&W)";
			}else{
				pszLabel = "現在のウィンドウ幅で折り返し(&W)";
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , pszLabel );	//Sept. 13, 2000 JEPRO アクセスキー付与	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , "現在のウィンドウ幅で折り返し(&W)" );	//Sept. 13, 2000 JEPRO アクセスキー付与	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			if( !m_pShareData->m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECKEYMACRO	, "キーマクロの記録開始(&R)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SAVEKEYMACRO, "キーマクロの保存(&M)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LOADKEYMACRO, "キーマクロの読み込み(&A)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECKEYMACRO, "キーマクロの実行(&D)" );
			}else{
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECKEYMACRO	, "キーマクロの記録終了(&R)" );
				::CheckMenuItem( hMenu, F_RECKEYMACRO, MF_BYCOMMAND | MF_CHECKED );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SAVEKEYMACRO, "キーマクロの記録終了&&保存(&M)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LOADKEYMACRO, "キーマクロの記録終了&&読み込み(&A)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECKEYMACRO, "キーマクロの記録終了&&実行(&D)" );
			}
			
			//	From Here Sep. 14, 2001 genta
			//「登録済みマクロ」ポップアップ
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			
			for( i = 0; i < MAX_CUSTMACRO; ++i ){
				MacroRec *mp = &m_pShareData->m_MacroTable[i];
				if( mp->IsEnabled() ){
					if(  mp->m_szName[0] ){
						m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + i, mp->m_szName );
					}
					else {
						m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + i, mp->m_szFile );
					}
				}
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "登録済みマクロ(&B)" );
			//	To Here Sep. 14, 2001 genta

			if( m_pShareData->m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
				::CheckMenuItem( hMenu, F_RECKEYMACRO, MF_BYCOMMAND | MF_CHECKED );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECCMMAND, "外部コマンド実行(&X)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECCOMMAND_DIALOG, "外部コマンド実行(&X)" );	//Mar. 10, 2001 JEPRO 機能しないのでメニューから隠した	//Mar.21, 2001 JEPRO 標準出力しないで復活
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			//To Here Sept. 20, 2000

			//「カスタムメニュー」ポップアップ
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//	右クリックメニュー
			if( m_pShareData->m_Common.m_nCustMenuItemNumArr[0] > 0 ){
				 m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING,
				 	F_MENU_RBUTTON, m_pShareData->m_Common.m_szCustMenuNameArr[0] );
			}
			//	カスタムメニュー
			for( i = 1; i < MAX_CUSTOM_MENU; ++i ){
				if( m_pShareData->m_Common.m_nCustMenuItemNumArr[i] > 0 ){
					 m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING,
					 	F_CUSTMENU_BASE + i, m_pShareData->m_Common.m_szCustMenuNameArr[i] );
				}
			}

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "カスタムメニュー(&U)" );

//		m_pShareData->m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

			break;

//		case 7://case 5: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
		case 5://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
			m_CMenuDrawer.ResetContents();
			/* 「ウィンドウ」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

			if( 1 == m_cEditDoc.m_cSplitterWnd.GetAllSplitRows() ){ // 2002/2/8 aroka メソッドを通じてアクセス
				pszLabel = "上下に分割(&-)";	//Oct. 7, 2000 JEPRO アクセスキーを変更(T→-)
			}else{
				pszLabel = "上下分割の解除(&-)";
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_V , pszLabel );

			if( 1 == m_cEditDoc.m_cSplitterWnd.GetAllSplitCols() ){ // 2002/2/8 aroka メソッドを通じてアクセス
				pszLabel = "左右に分割(&I)";	//Oct. 7, 2000 JEPRO アクセスキーを変更(Y→I)
			}else{
				pszLabel = "左右分割の解除(&I)";
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_H , pszLabel );
//	From Here Sept. 17, 2000 JEPRO	縦横分割の場合も状態によってメニューメッセージが変わるように変更
			if( (1 < m_cEditDoc.m_cSplitterWnd.GetAllSplitRows()) && (1 < m_cEditDoc.m_cSplitterWnd.GetAllSplitCols()) ){ // 2002/2/8 aroka メソッドを通じてアクセス
				pszLabel = "縦横分割の解除(&S)";	//Feb. 18, 2001 JEPRO アクセスキー変更(Q→S)
			}else{
				pszLabel = "縦横に分割(&S)";	//Sept. 17, 2000 jepro 説明に「に」を追加	//Oct. 7, 2000 JEPRO アクセスキーを変更(S→Q)	//Feb. 18, 2001 JEPRO アクセスキーを元に戻した(Q→S)
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_VH , pszLabel );
//	To Here Sept. 17, 2000
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINCLOSE		, "閉じる(&C)" );			//Feb. 18, 2001 JEPRO アクセスキー変更(O→C)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_CLOSEALL	, "すべて閉じる(&Q)" );		//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)	//Feb. 18, 2001 JEPRO アクセスキー変更(L→Q)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_NEXTWINDOW		, "次のウィンドウ(&N)" );	//Sept. 11, 2000 JEPRO "次"を"前"の前に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PREVWINDOW		, "前のウィンドウ(&P)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CASCADE			, "重ねて表示(&E)" );		//Oct. 7, 2000 JEPRO アクセスキー変更(C→E)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TILE_V			, "上下に並べて表示(&H)" );	//Sept. 13, 2000 JEPRO 分割に合わせてメニューの左右と上下を入れ替えた //Oct. 7, 2000 JEPRO アクセスキー変更(V→H)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TILE_H			, "左右に並べて表示(&T)" );	//Oct. 7, 2000 JEPRO アクセスキー変更(H→T)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MAXIMIZE_V		, "縦方向に最大化(&X)" );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MAXIMIZE_H		, "横方向に最大化(&Y)" );	//2001.02.10 by MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MINIMIZE_ALL	, "すべて最小化(&M)" );		//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* セパレータ */				//Oct. 22, 2000 JEPRO 下の「再描画」復活に伴いセパレータを追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REDRAW			, "再描画(&R)" );			//Oct. 22, 2000 JEPRO コメントアウトされていたのを復活させた
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_OUTPUT		, "アウトプット(&U)" );		//Sept. 13, 2000 JEPRO アクセスキー変更(O→U)

			/* 現在開いている編集窓のリストをメニューにする */
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if( nRowNum > 0 ){
				/* セパレータ */
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
				for( i = 0; i < nRowNum; ++i ){
					/* トレイからエディタへの編集ファイル名要求通知 */
					::SendMessage( pEditNodeArr[i].m_hWnd, MYWM_GETFILEINFO, 0, 0 );
					pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;
					if( pfi->m_bIsGrep ){
						/* データを指定バイト数以内に切り詰める */
						CMemory		cmemDes;
						int			nDesLen;
						const char*	pszDes;
						LimitStringLengthB( pfi->m_szGrepKey, lstrlen( pfi->m_szGrepKey ), 64, cmemDes );
						pszDes = cmemDes.GetPtr();
						nDesLen = lstrlen( pszDes );
//	From Here Oct. 4, 2000 JEPRO commented out & modified	開いているファイル数がわかるように履歴とは違って1から数える
//		i >= 10 + 26 の時の考慮を省いた(に近い)が開くファイル数が36個を越えることはまずないので事実上OKでしょう
						wsprintf( szMemu, "&%c 【Grep】\"%s%s\"", ((1 + i) <= 9)?('1' + i):('A' + i - 9),
							pszDes, ( (int)lstrlen( pfi->m_szGrepKey ) > nDesLen ) ? "・・・":""
						);
					}else
					if( pEditNodeArr[i].m_hWnd == m_pShareData->m_hwndDebug ){
//		i >= 10 + 26 の時の考慮を省いた(に近い)が出力ファイル数が36個を越えることはまずないので事実上OKでしょう
						wsprintf( szMemu, "&%c アウトプット", ((1 + i) <= 9)?('1' + i):('A' + i - 9) );

					}else{
//		From Here Jan. 23, 2001 JEPRO
//		ファイル名やパス名に'&'が使われているときに履歴等でキチンと表示されない問題を修正(&を&&に置換するだけ)
//<----- From Here Added
						char	szFile2[_MAX_PATH * 2];
						if( '\0' == pfi->m_szPath[0] ){
							strcpy( szFile2, "(無題)" );
						}else{
							char buf[_MAX_PATH];
							CShareData::getInstance()->GetTransformFileName( pfi->m_szPath, buf, _MAX_PATH );
							dupamp( buf, szFile2 );
						}
						wsprintf( szMemu, "&%c %s %s", ((1 + i) <= 9)?('1' + i):('A' + i - 9),
							szFile2,
							pfi->m_bIsModified ? "*":" "
						);
//-----> To Here Added
//		To Here Jan. 23, 2001

//	To Here Oct. 4, 2000
						// SJIS以外の文字コードの種別を表示する
						// gm_pszCodeNameArr_3 からコピーするように変更
						if( 0 < pfi->m_nCharCode && pfi->m_nCharCode < CODE_CODEMAX ){
							strcat( szMemu, gm_pszCodeNameArr_3[pfi->m_nCharCode] );
						}
					}
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, szMemu );
					if( m_hWnd == pEditNodeArr[i].m_hWnd ){
						::CheckMenuItem( hMenu, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, MF_BYCOMMAND | MF_CHECKED );
					}
				}
				delete [] pEditNodeArr;
			}

			break;

//		case 8://case 6: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
		case 6://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
			m_CMenuDrawer.ResetContents();
			/* 「ヘルプ」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
//Sept. 15, 2000→Nov. 25, 2000 JEPRO //ショートカットキーがうまく働かないので殺してあった下の2行を修正・復活
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , "目次(&O)" );				//Sept. 7, 2000 jepro キャプションを「ヘルプ目次」から変更	Oct. 13, 2000 JEPRO アクセスキーを「トレイ右ボタン」のために変更(C→O)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH	,	 "キーワード検索(&S)" );	//Sept. 7, 2000 jepro キャプションを「ヘルプトピックの検索」から変更 //Nov. 25, 2000 jepro「トピックの」→「キーワード」に変更
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MENU_ALLFUNC	, "コマンド一覧(&M)" );		//Oct. 13, 2000 JEPRO アクセスキーを「トレイ右ボタン」のために変更(L→M)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHELP1		, "外部ヘルプ１(&E)" );		//Sept. 7, 2000 JEPRO このメニューの順番をトップから下に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHTMLHELP		, "外部HTMLヘルプ(&H)" );	//Sept. 7, 2000 JEPRO このメニューの順番を２番目から下に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ABOUT			, "バージョン情報(&A)" );	//Dec. 25, 2000 JEPRO F_に変更
			break;
		}
	}

end_of_func_IsEnable:;
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	if (m_pPrintPreview)	return;	//	印刷プレビューモードなら排除。（おそらく排除しなくてもいいと思うんだけど、念のため）

	/* 機能が利用可能かどうか、チェック状態かどうかを一括チェック */
	cMenuItems = ::GetMenuItemCount( hMenu );
	for (nPos = 0; nPos < cMenuItems; nPos++) {
		id = ::GetMenuItemID(hMenu, nPos);
		/* 機能が利用可能か調べる */
		if( IsFuncEnable( &m_cEditDoc, m_pShareData, id ) ){
			fuFlags = MF_BYCOMMAND | MF_ENABLED;
		}else{
			fuFlags = MF_BYCOMMAND | MF_GRAYED;
		}
		::EnableMenuItem(hMenu, id, fuFlags);

		/* 機能がチェック状態か調べる */
		if( IsFuncChecked( &m_cEditDoc, m_pShareData, id ) ){
			fuFlags = MF_BYCOMMAND | MF_CHECKED;
			::CheckMenuItem(hMenu, id, fuFlags);
		}
		/* else{
			fuFlags = MF_BYCOMMAND | MF_UNCHECKED;
		}
		*/
	}
	return;
}




/* ファイルがドロップされた */
void CEditWnd::OnDropFiles( HDROP hDrop )
{
		POINT		pt;
		WORD		cFiles, i;
		char		szFile[_MAX_PATH + 1];
		char		szWork[_MAX_PATH + 1];
		BOOL		bOpened;
		FileInfo*	pfi;
		HWND		hWndOwner;

		::DragQueryPoint( hDrop, &pt );
		cFiles = ::DragQueryFile( hDrop, 0xFFFFFFFF, (LPSTR) NULL, 0);
		/* ファイルをドロップしたときは閉じて開く */
		if( m_pShareData->m_Common.m_bDropFileAndClose ){
			cFiles = 1;
		}
		/* 一度にドロップ可能なファイル数 */
		if( cFiles > m_pShareData->m_Common.m_nDropFileNumMax ){
				cFiles = m_pShareData->m_Common.m_nDropFileNumMax;
		}
		for( i = 0; i < cFiles; i++ ) {
				::DragQueryFile( hDrop, i, szFile, sizeof(szFile) );
				/* ショートカット(.lnk)の解決 */
				if( TRUE == ResolveShortcutLink( NULL, szFile, szWork ) ){
					strcpy( szFile, szWork );
				}
				/* ロングファイル名を取得する */
				if( TRUE == ::GetLongFileName( szFile, szWork ) ){
						strcpy( szFile, szWork );
				}

				/* 指定ファイルが開かれているか調べる */
				if( CShareData::getInstance()->IsPathOpened( szFile, &hWndOwner ) ){
					::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
					pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;
					/* アクティブにする */
					ActivateFrameWindow( hWndOwner );
					/* MRUリストへの登録 */
					CMRU cMRU;
					cMRU.Add( pfi );
				}else{
						/* 変更フラグがオフで、ファイルを読み込んでいない場合 */
						if( !m_cEditDoc.IsModified() &&	//	Jan. 22, 2002 genta
							!m_cEditDoc.IsFilePathAvailable() &&	/* 現在編集中のファイルのパス */
							//	Jun. 13, 2003 Moca GrepウィンドウへDropしたときにGrepウィンドウのまま
							//	同じウィンドウで開かれてしまう問題を修正
							!m_cEditDoc.m_bGrepMode					/* Grep結果ではない */  // 2003.06.13 Add
						){
								/* ファイル読み込み */
								m_cEditDoc.FileRead(
										szFile,
										&bOpened,
										CODE_AUTODETECT,	/* 文字コード自動判別 */
										FALSE,				/* 読み取り専用か */
										TRUE				/* 文字コード変更時の確認をするかどうか */
								);
								hWndOwner = m_hWnd;
								/* アクティブにする */
								ActivateFrameWindow( hWndOwner );
						}else{
								/* ファイルをドロップしたときは閉じて開く */
								if( m_pShareData->m_Common.m_bDropFileAndClose ){
										/* ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行 */
										if( m_cEditDoc.OnFileClose() ){
												/* 既存データのクリア */
												m_cEditDoc.Init();

												/* 全ビューの初期化 */
												m_cEditDoc.InitAllView();

												/* 親ウィンドウのタイトルを更新 */
												m_cEditDoc.SetParentCaption();

												/* ファイル読み込み */
														m_cEditDoc.FileRead(
														szFile,
														&bOpened,
														CODE_AUTODETECT,	/* 文字コード自動判別 */
														FALSE,				/* 読み取り専用か */
														TRUE				/* 文字コード変更時の確認をするかどうか */
												);
												hWndOwner = m_hWnd;
												/* アクティブにする */
												ActivateFrameWindow( hWndOwner );
										}
										goto end_of_drop_query;
								}else{
										/* 編集ウィンドウの上限チェック */
										if( m_pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
												char szMsg[512];
												wsprintf( szMsg, "編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。", MAX_EDITWINDOWS );
												::MessageBox( NULL, szMsg, GSTR_APPNAME, MB_OK );
												::DragFinish( hDrop );
												return;
										}
										char	szFile2[_MAX_PATH + 3];
										if( strchr( szFile, ' ' ) ){
												wsprintf( szFile2, "\"%s\"", szFile );
												strcpy( szFile, szFile2 );
										}
										/* 新たな編集ウィンドウを起動 */
										CEditApp::OpenNewEditor(
												m_hInstance,
												m_hWnd,
												szFile,
												CODE_AUTODETECT,	/* 文字コード自動判別 */
												FALSE				/* 読み取り専用か */
										);
								}
						}
				}
		}
end_of_drop_query:;
		::DragFinish( hDrop );
		return;
}


/* タイマーの処理
	@datey 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
*/
void CEditWnd::OnTimer(
	HWND		hwnd,		// handle of window for timer messages
	UINT		uMsg,		// WM_TIMER message
	UINT		idEvent,	// timer identifier
	DWORD		dwTime 		// current system time
)
{
	static	int	nLoopCount = 0;
	int			i;
	TBBUTTON	tbb;
	nLoopCount++;
	if( 10 < nLoopCount ){
		nLoopCount = 0;
	}
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	/* 印刷プレビューなら、何もしない。そうでなければ、ツールバーの状態更新 */
	if( !m_pPrintPreview && NULL != m_hwndToolBar ){
		for( i = 0; i < m_pShareData->m_Common.m_nToolBarButtonNum; ++i ){
			tbb = m_CMenuDrawer.m_tbMyButton[m_pShareData->m_Common.m_nToolBarButtonIdxArr[i]];
			/* 機能が利用可能か調べる */
			::PostMessage(
				m_hwndToolBar, TB_ENABLEBUTTON, tbb.idCommand,
				(LPARAM) MAKELONG( (IsFuncEnable( &m_cEditDoc, m_pShareData, tbb.idCommand ) ) , 0 )
			);
			/* 機能がチェック状態か調べる */
			::PostMessage(
				m_hwndToolBar, TB_CHECKBUTTON, tbb.idCommand,
				(LPARAM) MAKELONG( IsFuncChecked( &m_cEditDoc, m_pShareData, tbb.idCommand ), 0 )
			);
		}
	}

	if( nLoopCount == 0 ){
		/* ファイルのタイムスタンプのチェック処理 */
		m_cEditDoc.CheckFileTimeStamp() ;
	}

	m_cEditDoc.CheckAutoSave();
	return;
}

/* 機能がチェック状態か調べる */
int CEditWnd::IsFuncChecked( CEditDoc* pcEditDoc, DLLSHAREDATA*	pShareData, int nId )
{
	CEditWnd* pCEditWnd;
	pCEditWnd = ( CEditWnd* )::GetWindowLong( pcEditDoc->m_hwndParent, GWL_USERDATA );
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことにより、プレビュー判定削除
	switch( nId ){
	case F_FILE_REOPEN_SJIS:
		if( CODE_SJIS == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_FILE_REOPEN_JIS:
		if( CODE_JIS == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_FILE_REOPEN_EUC:
		if( CODE_EUC == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_FILE_REOPEN_UNICODE:
		if( CODE_UNICODE == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_FILE_REOPEN_UNICODEBE:
		if( CODE_UNICODEBE == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_FILE_REOPEN_UTF8:
		if( CODE_UTF8 == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_FILE_REOPEN_UTF7:
		if( CODE_UTF7 == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_RECKEYMACRO:	/* キーマクロの記録開始／終了 */
		if( pShareData->m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			if( pShareData->m_hwndRecordingKeyMacro == pcEditDoc->m_hwndParent ){	/* キーボードマクロを記録中のウィンドウ */
				return TRUE;
			}else{
				return FALSE;
			}
		}else{
			return FALSE;
		}
	case F_SHOWTOOLBAR:
		if( pCEditWnd->m_hwndToolBar != NULL ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_SHOWFUNCKEY:
		if( pCEditWnd->m_CFuncKeyWnd.m_hWnd != NULL ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_SHOWSTATUSBAR:
		if( pCEditWnd->m_hwndStatusBar != NULL ){
			return TRUE;
		}else{
			return FALSE;
		}
	// Mar. 6, 2002 genta
	case F_READONLY://読み取り専用
		if( pcEditDoc->m_bReadOnly ){ /* 変更フラグ */
			return TRUE;
		}else{
			return FALSE;
		}
	}
	return FALSE;
}




/* 機能が利用可能か調べる */
int CEditWnd::IsFuncEnable( CEditDoc* pcEditDoc, DLLSHAREDATA* pShareData, int nId )
{
	/* 書き換え禁止のときを一括チェック */
	if( pcEditDoc->IsModificationForbidden( nId ) )
		return FALSE;

	switch( nId ){
	case F_RECKEYMACRO:	/* キーマクロの記録開始／終了 */
		if( pShareData->m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			if( pShareData->m_hwndRecordingKeyMacro == pcEditDoc->m_hwndParent ){	/* キーボードマクロを記録中のウィンドウ */
				return TRUE;
			}else{
				return FALSE;
			}
		}else{
			return TRUE;
		}
	case F_SAVEKEYMACRO:	/* キーマクロの保存 */
		//	Jun. 16, 2002 genta
		//	キーマクロエンジン以外のマクロを読み込んでいるときは
		//	実行はできるが保存はできない．
		if( pShareData->m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			if( pShareData->m_hwndRecordingKeyMacro == pcEditDoc->m_hwndParent ){	/* キーボードマクロを記録中のウィンドウ */
				return TRUE;
			}else{
				return FALSE;
			}
		}else{
			return pcEditDoc->m_pcSMacroMgr->IsSaveOk();
		}
	case F_EXECKEYMACRO:	/* キーマクロの実行 */
		if( pShareData->m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			if( pShareData->m_hwndRecordingKeyMacro == pcEditDoc->m_hwndParent ){	/* キーボードマクロを記録中のウィンドウ */
				return TRUE;
			}else{
				return FALSE;
			}
		}else{
			//@@@ 2002.1.24 YAZAKI m_szKeyMacroFileNameにファイル名がコピーされているかどうか。
			if (pShareData->m_szKeyMacroFileName[0] ) {
				return TRUE;
			}else{
				return FALSE;
			}
		}
	case F_LOADKEYMACRO:	/* キーマクロの読み込み */
		if( pShareData->m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			if( pShareData->m_hwndRecordingKeyMacro == pcEditDoc->m_hwndParent ){	/* キーボードマクロを記録中のウィンドウ */
				return TRUE;
			}else{
				return FALSE;
			}
		}else{
			return TRUE;
		}

	case F_SEARCH_CLEARMARK:	//検索マークのクリア
		return TRUE;

	// 02/06/26 ai Start
	case F_JUMP_SRCHSTARTPOS:	// 検索開始位置へ戻る
		if( 0 <= pcEditDoc->m_cEditViewArr[pcEditDoc->m_nActivePaneIndex].m_nSrchStartPosX_PHY &&
			0 <= pcEditDoc->m_cEditViewArr[pcEditDoc->m_nActivePaneIndex].m_nSrchStartPosY_PHY) {
			return TRUE;
		}else{
			return FALSE;
		}
	// 02/06/26 ai End

	case F_COMPARE:	/* ファイル内容比較 */
		if( 2 <= pShareData->m_nEditArrNum ){
			return TRUE;
		}else{
			return FALSE;
		}

	case F_DIFF_NEXT:	/* 次の差分へ */	//@@@ 2002.05.25 MIK
	case F_DIFF_PREV:	/* 前の差分へ */	//@@@ 2002.05.25 MIK
	case F_DIFF_RESET:	/* 差分の全解除 */	//@@@ 2002.05.25 MIK
		if( ! pcEditDoc->m_cDocLineMgr.IsDiffUse() ) return FALSE;
		return TRUE;
	case F_DIFF_DIALOG:	/* DIFF差分表示 */	//@@@ 2002.05.25 MIK
		//if( pcEditDoc->IsModified() ) return FALSE;
		//if( ! pcEditDoc->IsFilePathAvailable() ) return FALSE;
		return TRUE;

	case F_BEGIN_BOX:	//矩形範囲選択開始
		if( TRUE == pShareData->m_Common.m_bFontIs_FIXED_PITCH ){	/* 現在のフォントは固定幅フォントである */
			return TRUE;
		}else{
			return FALSE;
		}
	case F_PASTEBOX:
		/* クリップボードから貼り付け可能か？ */
		if( pcEditDoc->IsEnablePaste() &&
			TRUE == pShareData->m_Common.m_bFontIs_FIXED_PITCH
		){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_PASTE:
		/* クリップボードから貼り付け可能か？ */
		if( pcEditDoc->IsEnablePaste() ){
			return TRUE;
		}else{
			return FALSE;
		}

	case F_FILENEW:	/* 新規作成 */
	case F_GREP_DIALOG:	/* Grep */
		/* 編集ウィンドウの上限チェック */
		if( pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
			return FALSE;
		}else{
			return TRUE;
		}

	case F_FILESAVE:	/* 上書き保存 */
		if( !pcEditDoc->m_bReadOnly ){	/* 読み取り専用モード */
			if( TRUE == pcEditDoc->IsModified() ){	/* 変更フラグ */
				return TRUE;
			}else{
				/* 無変更でも上書きするか */
				if( FALSE == pShareData->m_Common.m_bEnableUnmodifiedOverwrite ){
					return FALSE;
				}else{
					return TRUE;
				}
			}
		}else{
			return FALSE;
		}
	case F_COPYLINES:				//選択範囲内全行コピー
	case F_COPYLINESASPASSAGE:		//選択範囲内全行引用符付きコピー
	case F_COPYLINESWITHLINENUMBER:	//選択範囲内全行行番号付きコピー
		if( pcEditDoc->IsTextSelected( ) ){/* テキストが選択されているか */
			return TRUE;
		}else{
			return FALSE;
		}

	case F_TOLOWER:					/* 英大文字→英小文字 */
	case F_TOUPPER:					/* 英小文字→英大文字 */
	case F_TOHANKAKU:				/* 全角→半角 */
	case F_TOHANKATA:				/* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	case F_TOZENEI:					/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	case F_TOHANEI:					/* 全角英数→半角英数 */
	case F_TOZENKAKUKATA:			/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	case F_TOZENKAKUHIRA:			/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	case F_HANKATATOZENKAKUKATA:	/* 半角カタカナ→全角カタカナ */
	case F_HANKATATOZENKAKUHIRA:	/* 半角カタカナ→全角ひらがな */
	case F_TABTOSPACE:				/* TAB→空白 */
	case F_SPACETOTAB:				/* 空白→TAB */  //#### Stonee, 2001/05/27
	case F_CODECNV_AUTO2SJIS:		/* 自動判別→SJISコード変換 */
	case F_CODECNV_EMAIL:			/* E-Mail(JIS→SJIS)コード変換 */
	case F_CODECNV_EUC2SJIS:		/* EUC→SJISコード変換 */
	case F_CODECNV_UNICODE2SJIS:	/* Unicode→SJISコード変換 */
	case F_CODECNV_UNICODEBE2SJIS:	/* UnicodeBE→SJISコード変換 */
	case F_CODECNV_UTF82SJIS:		/* UTF-8→SJISコード変換 */
	case F_CODECNV_UTF72SJIS:		/* UTF-7→SJISコード変換 */
	case F_CODECNV_SJIS2JIS:		/* SJIS→JISコード変換 */
	case F_CODECNV_SJIS2EUC:		/* SJIS→EUCコード変換 */
	case F_CODECNV_SJIS2UTF8:		/* SJIS→UTF-8コード変換 */
	case F_CODECNV_SJIS2UTF7:		/* SJIS→UTF-7コード変換 */
	case F_BASE64DECODE:			/* Base64デコードして保存 */
	case F_UUDECODE:				//uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更

		/* テキストが選択されているか */
		if( pcEditDoc->IsTextSelected( ) ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_SELECTWORD:	/* 現在位置の単語選択 */
	case F_CUT_LINE:	//行切り取り(折り返し単位)
	case F_DELETE_LINE:	//行削除(折り返し単位)
		/* テキストが選択されているか */
		if( pcEditDoc->IsTextSelected( ) ){
			return FALSE;
		}else{
			return TRUE;
		}
	case F_UNDO:
		/* Undo(元に戻す)可能な状態か？ */
		if( pcEditDoc->IsEnableUndo() ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_REDO:
		/* Redo(やり直し)可能な状態か？ */
		if( pcEditDoc->IsEnableRedo() ){
			return TRUE;
		}else{
			return FALSE;
		}

	case F_COPYPATH:
	case F_COPYTAG:
	case F_COPYFNAME:					// 2002/2/3 aroka
	case F_OPEN_HfromtoC:				//同名のC/C++ヘッダ(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
	case F_OPEN_HHPP:					//同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
	case F_OPEN_CCPP:					//同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
	case F_PLSQL_COMPILE_ON_SQLPLUS:	/* Oracle SQL*Plusで実行 */
	case F_BROWSE:						//ブラウズ
	//case F_READONLY:					//読み取り専用	//	Sep. 10, 2002 genta 常に使えるように
	case F_PROPERTY_FILE:
		/* 現在編集中のファイルのパス名をクリップボードにコピーできるか */
		if( pcEditDoc->IsFilePathAvailable() ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_JUMPHIST_PREV:	//	移動履歴: 前へ
		if( pcEditDoc->ActiveView().m_cHistory->CheckPrev() )
			return TRUE;
		else
			return FALSE;
	case F_JUMPHIST_NEXT:	//	移動履歴: 次へ
		if( pcEditDoc->ActiveView().m_cHistory->CheckNext() )
			return TRUE;
		else
			return FALSE;
	case F_JUMPHIST_SET:	//	現在位置を移動履歴に登録
		return TRUE;
	case F_DIRECT_TAGJUMP:	//ダイレクトタグジャンプ	//@@@ 2003.04.15 MIK
	//	2003.05.12 MIK タグファイル作成先を選べるようにしたので、常に作成可能とする
//	case F_TAGS_MAKE:	//タグファイルの作成	//@@@ 2003.04.13 MIK
		if( pcEditDoc->IsFilePathAvailable() ){
			return TRUE;
		}else{
			return FALSE;
		}
		
	}
	return TRUE;
}

//#ifdef _DEBUG
	/* デバッグモニタモードに設定 */
	void CEditWnd::SetDebugModeON( void )
	{
		if( NULL != m_pShareData->m_hwndDebug ){
			if( CShareData::IsEditWnd( m_pShareData->m_hwndDebug ) ){
				return;
			}
		}
		m_pShareData->m_hwndDebug = m_hWnd;
		m_cEditDoc.m_bDebugMode = TRUE;

// 2001/06/23 N.Nakatani アウトプット窓への出力テキストの追加F_ADDTAILが抑止されるのでとりあえず読み取り専用モードは辞めました
		m_cEditDoc.m_bReadOnly = FALSE;		/* 読み取り専用モード */
		/* 親ウィンドウのタイトルを更新 */
		m_cEditDoc.SetParentCaption();
		return;
	}
//#endif


/* 機能IDに対応するヘルプコンテキスト番号を返す */
// Modified by Stonee, 2001/02/23
// etc_uty.cpp内に中身を移動
/*	2002/04/17 YAZAKI コメントアウト
int CEditWnd::FuncID_To_HelpContextID( int nFuncID )
{
	return ::FuncID_To_HelpContextID(nFuncID);
}
*/

/* メニューアイテムに対応するヘルプを表示 */
void CEditWnd::OnHelp_MenuItem( HWND hwndParent, int nFuncID )
{
	char	szHelpFile[_MAX_PATH + 1];
	int		nHelpContextID;

	/* ヘルプファイルのフルパスを返す */
	::GetHelpFilePath( szHelpFile );

	/* 機能IDに対応するヘルプコンテキスト番号を返す */
	nHelpContextID = FuncID_To_HelpContextID( nFuncID );
	if( 0 != nHelpContextID ){
		::WinHelp( hwndParent, szHelpFile, HELP_CONTEXT, nHelpContextID );
	}
	return;
}


//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更

/* 印刷プレビューモードのオン/オフ */
void CEditWnd::PrintPreviewModeONOFF( void )
{
	HMENU	hMenu;

	/* 印刷プレビューモードか */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	if( m_pPrintPreview ){
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		/*	印刷プレビューモードを解除します。	*/
		delete m_pPrintPreview;	//	削除。
		m_pPrintPreview = NULL;	//	NULLか否かで、プリントプレビューモードか判断するため。

		/*	通常モードに戻す	*/
		::ShowWindow( m_cEditDoc.m_hWnd, SW_SHOW );
		::ShowWindow( m_hwndToolBar, SW_SHOW );
		::ShowWindow( m_hwndStatusBar, SW_SHOW );
		::ShowWindow( m_CFuncKeyWnd.m_hWnd, SW_SHOW );

		::SetFocus( m_hWnd );

		hMenu = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_MENU1 ) );
		::SetMenu( m_hWnd, hMenu );
		::DrawMenuBar( m_hWnd );

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		::InvalidateRect( m_hWnd, NULL, TRUE );
	}else{
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		/*	通常モードを隠す	*/
		hMenu = ::GetMenu( m_hWnd );
		//	Jun. 18, 2001 genta Print Previewではメニューを削除
		::SetMenu( m_hWnd, NULL );
		::DestroyMenu( hMenu );
		::DrawMenuBar( m_hWnd );

		::ShowWindow( m_cEditDoc.m_hWnd, SW_HIDE );
		::ShowWindow( m_hwndToolBar, SW_HIDE );
		::ShowWindow( m_hwndStatusBar, SW_HIDE );
		::ShowWindow( m_CFuncKeyWnd.m_hWnd, SW_HIDE );

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		m_pPrintPreview = new CPrintPreview( this );
		/* 現在の印刷設定 */
		m_pPrintPreview->SetPrintSetting(
			&m_pShareData->m_PrintSettingArr[
				m_cEditDoc.GetDocumentAttribute().m_nCurrentPrintSetting]
		);

		//	プリンタの情報を取得。

		/* 現在のデフォルトプリンタの情報を取得 */
		BOOL bRes;
		bRes = m_pPrintPreview->GetDefaultPrinterInfo();
		if( FALSE == bRes ){
			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, GSTR_APPNAME,
				"印刷プレビューを実行する前に、プリンタをインストールしてください。\n"
			);
			return;
		}

		/* 印刷設定の反映 */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		m_pPrintPreview->OnChangePrintSetting();
		::InvalidateRect( m_hWnd, NULL, TRUE );
		::UpdateWindow( m_hWnd /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);

	}
	return;

}




/* WM_SIZE 処理 */
LRESULT CEditWnd::OnSize( WPARAM wParam, LPARAM lParam )
{
	int			cx;
	int			cy;
	int			nToolBarHeight;
	int			nStatusBarHeight;
	int			nFuncKeyWndHeight;
	RECT		rc, rcClient;
	int			nCxHScroll;
	int			nCyHScroll;
	int			nCxVScroll;
	int			nCyVScroll;
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる
//	変数削除

	RECT		rcWin;



	cx = LOWORD( lParam );
	cy = HIWORD( lParam );

	/* ウィンドウサイズ継承 */
	if( wParam != SIZE_MINIMIZED &&						/* 最小化は継承しない */
		m_pShareData->m_Common.m_bSaveWindowSize		/* ウィンドウサイズ継承をするか */
	){
		if( wParam == SIZE_MAXIMIZED ){					/* 最大化はサイズを記録しない */
			if( m_pShareData->m_Common.m_nWinSizeType != (int)wParam ){
				m_pShareData->m_Common.m_nWinSizeType = wParam;
			}
		}else{
			::GetWindowRect( m_hWnd, &rcWin );
			/* ウィンドウサイズに関するデータが変更されたか */
			if( m_pShareData->m_Common.m_nWinSizeType != (int)wParam ||
				m_pShareData->m_Common.m_nWinSizeCX != rcWin.right - rcWin.left ||
				m_pShareData->m_Common.m_nWinSizeCY != rcWin.bottom - rcWin.top
			){
				m_pShareData->m_Common.m_nWinSizeType = wParam;
				m_pShareData->m_Common.m_nWinSizeCX = rcWin.right - rcWin.left;
				m_pShareData->m_Common.m_nWinSizeCY = rcWin.bottom - rcWin.top;
			}
		}
	}

	m_nWinSizeType = wParam;	/* サイズ変更のタイプ */
	nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );


	nToolBarHeight = 0;
	if( NULL != m_hwndToolBar ){
		::SendMessage( m_hwndToolBar, WM_SIZE, wParam, lParam );
		::GetWindowRect( m_hwndToolBar, &rc );
		nToolBarHeight = rc.bottom - rc.top;
	}
	nFuncKeyWndHeight = 0;
	if( NULL != m_CFuncKeyWnd.m_hWnd ){
		::SendMessage( m_CFuncKeyWnd.m_hWnd, WM_SIZE, wParam, lParam );
		::GetWindowRect( m_CFuncKeyWnd.m_hWnd, &rc );
		nFuncKeyWndHeight = rc.bottom - rc.top;
	}
	nStatusBarHeight = 0;
	if( NULL != m_hwndStatusBar ){
		::SendMessage( m_hwndStatusBar, WM_SIZE, wParam, lParam );
		::GetClientRect( m_hwndStatusBar, &rc );
		//	May 12, 2000 genta
		//	2カラム目に改行コードの表示を挿入
		//	From Here
		int			nStArr[8];
		const char*	pszLabel[7] = { "", "999999行 99999列", "RR0RR0","FFFFFFFF", "SJIS  ", "REC", "上書" };	//Oct. 30, 2000 JEPRO 千万行も要らん
		int			nStArrNum = 7;
		//	To Here
		int			nAllWidth;
		SIZE		sz;
		HDC			hdc;
		int			i;
		nAllWidth = rc.right - rc.left;
		hdc = ::GetDC( m_hwndStatusBar );
		nStArr[nStArrNum - 1] = nAllWidth;
		if( wParam != SIZE_MAXIMIZED ){
			nStArr[nStArrNum - 1] -= 16;
		}
		for( i = nStArrNum - 1; i > 0; i-- ){
			::GetTextExtentPoint32( hdc, pszLabel[i], lstrlen( pszLabel[i] ), &sz );
			nStArr[i - 1] = nStArr[i] - ( sz.cx + ::GetSystemMetrics( SM_CXEDGE ) );
		}
		nStArr[0] += 16;	//Nov. 2, 2000 JEPRO よくわからないがともかくここを増やしてみる

		::SendMessage( m_hwndStatusBar, SB_SETPARTS, nStArrNum, (LPARAM) (LPINT)nStArr );
		::ReleaseDC( m_hwndStatusBar, hdc );


		::GetWindowRect( m_hwndStatusBar, &rc );
		nStatusBarHeight = rc.bottom - rc.top;
	}
	::GetClientRect( m_hWnd, &rcClient );

	if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 ){	/* ファンクションキー表示位置／0:上 1:下 */
		m_cEditDoc.OnMove(
			0,
			nToolBarHeight + nFuncKeyWndHeight,
			cx,
			cy - nToolBarHeight - nFuncKeyWndHeight - nStatusBarHeight
		);
		::MoveWindow( m_CFuncKeyWnd.m_hWnd, 0, nToolBarHeight, cx, nFuncKeyWndHeight, TRUE );
	}else
	if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 1 ){	/* ファンクションキー表示位置／0:上 1:下 */
		m_cEditDoc.OnMove( 0, nToolBarHeight, cx, cy - nToolBarHeight - nFuncKeyWndHeight - nStatusBarHeight );
		::MoveWindow(
			m_CFuncKeyWnd.m_hWnd,
			0,
			cy - nFuncKeyWndHeight - nStatusBarHeight,
			cx,
			nFuncKeyWndHeight, TRUE
		);
		BOOL	bSizeBox = TRUE;
		if( NULL != m_hwndStatusBar ){
			bSizeBox = FALSE;
		}
		if( wParam == SIZE_MAXIMIZED ){
			bSizeBox = FALSE;
		}
		m_CFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
	}
	/* 印刷プレビューモードか */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	if( !m_pPrintPreview ){
		return 0L;
	}
	return m_pPrintPreview->OnSize(wParam, lParam);
}




/* WM_PAINT 描画処理 */
LRESULT CEditWnd::OnPaint(
	HWND			hwnd,	// handle of window
	UINT			uMsg,	// message identifier
	WPARAM			wParam,	// first message parameter
	LPARAM			lParam 	// second message parameter
)
{
	HDC				hdc;
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	/* 印刷プレビューモードか */
	if( !m_pPrintPreview ){
		PAINTSTRUCT		ps;
		hdc = ::BeginPaint( hwnd, &ps );
		::EndPaint( hwnd, &ps );
		return 0L;
	}
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	return m_pPrintPreview->OnPaint(hwnd, uMsg, wParam, lParam);
}

/* 印刷プレビュー 垂直スクロールバーメッセージ処理 WM_VSCROLL */
LRESULT CEditWnd::OnVScroll( WPARAM wParam, LPARAM lParam )
{
	/* 印刷プレビューモードか */
	if( !m_pPrintPreview ){
		return 0;
	}
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	return m_pPrintPreview->OnVScroll(wParam, lParam);
}




/* 印刷プレビュー 水平スクロールバーメッセージ処理 */
LRESULT CEditWnd::OnHScroll( WPARAM wParam, LPARAM lParam )
{
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	/* 印刷プレビューモードか */
	if( !m_pPrintPreview ){
		return 0;
	}
	return m_pPrintPreview->OnHScroll( wParam, lParam );
}

LRESULT CEditWnd::OnLButtonDown( WPARAM wParam, LPARAM lParam )
{
	//by 鬼(2) キャプチャーして押されたら非クライアントでもこっちに来る
	if(m_IconClicked != icNone)
		return 0;

	WPARAM		fwKeys;
	int			xPos;
	int			yPos;
	fwKeys = wParam;		// key flags
	xPos = LOWORD(lParam);	// horizontal position of cursor
	yPos = HIWORD(lParam);	// vertical position of cursor
	m_nDragPosOrgX = xPos;
	m_nDragPosOrgY = yPos;
	SetCapture( m_hWnd );
	m_bDragMode = TRUE;

	return 0;
}
#define IDT_SYSMENU 1357
LRESULT CEditWnd::OnLButtonUp( WPARAM wParam, LPARAM lParam )
{
	//by 鬼 2002/04/18
	if(m_IconClicked != icNone)
	{
		if(m_IconClicked == icDown)
		{
			m_IconClicked = icClicked;
			//by 鬼(2) タイマー(IDは適当です)
			SetTimer(m_hWnd, IDT_SYSMENU, GetDoubleClickTime(), SysMenuTimer);
		}
		return 0;
	}

	m_bDragMode = FALSE;
	ReleaseCapture();
	::InvalidateRect( m_hWnd, NULL, TRUE );
	return 0;
}


LRESULT CEditWnd::OnMouseMove( WPARAM wParam, LPARAM lParam )
{
	//by 鬼
	if(m_IconClicked != icNone)
	{
		//by 鬼(2) 一回押された時だけ
		if(m_IconClicked == icDown)
		{
			POINT P;
			GetCursorPos(&P); //スクリーン座標
			if(SendMessage(m_hWnd, WM_NCHITTEST, 0, P.x | (P.y << 16)) != HTSYSMENU)
			{
				ReleaseCapture();
				m_IconClicked = icNone;

				if(m_cEditDoc.IsFilePathAvailable())
				{
					const char *PathEnd = m_cEditDoc.GetFilePath();
					for(const char* I = m_cEditDoc.GetFilePath(); *I != 0; ++I)
					{
						//by 鬼(2): DBCS処理
						if(IsDBCSLeadByte(*I))
							++I;
						else if(*I == '\\')
							PathEnd = I;
					}

					wchar_t WPath[MAX_PATH];
					int c = MultiByteToWideChar(CP_ACP, 0, m_cEditDoc.GetFilePath(), PathEnd - m_cEditDoc.GetFilePath(), WPath, MAX_PATH);
					WPath[c] = 0;
					wchar_t WFile[MAX_PATH];
					MultiByteToWideChar(CP_ACP, 0, PathEnd + 1, -1, WFile, MAX_PATH);

					IDataObject *DataObject;
					IMalloc *Malloc;
					IShellFolder *Desktop, *Folder;
					LPITEMIDLIST PathID, ItemID;
					SHGetMalloc(&Malloc);
					SHGetDesktopFolder(&Desktop);
					DWORD Eaten, Attribs;
					if(SUCCEEDED(Desktop->ParseDisplayName(0, NULL, WPath, &Eaten, &PathID, &Attribs)))
					{
						Desktop->BindToObject(PathID, NULL, IID_IShellFolder, (void**)&Folder);
						Malloc->Free(PathID);
						if(SUCCEEDED(Folder->ParseDisplayName(0, NULL, WFile, &Eaten, &ItemID, &Attribs)))
						{
							LPCITEMIDLIST List[1];
							List[0] = ItemID;
							Folder->GetUIObjectOf(0, 1, List, IID_IDataObject, NULL, (void**)&DataObject);
							Malloc->Free(ItemID);
#define DDASTEXT
#ifdef  DDASTEXT
							//テキストでも持たせる…便利
							int Len = lstrlen(m_cEditDoc.GetFilePath());
							FORMATETC F;
							STGMEDIUM M;

							F.cfFormat = CF_TEXT;
							F.ptd      = NULL;
							F.dwAspect = DVASPECT_CONTENT;
							F.lindex   = -1;
							F.tymed    = TYMED_HGLOBAL;

							M.tymed = TYMED_HGLOBAL;
							M.pUnkForRelease = NULL;
							M.hGlobal = GlobalAlloc(GMEM_MOVEABLE, Len+1);

							void* P = GlobalLock(M.hGlobal);
							CopyMemory(P, m_cEditDoc.GetFilePath(), Len+1);
							GlobalUnlock(M.hGlobal);

							DataObject->SetData(&F, &M, TRUE);
#endif
							//移動は禁止
							DWORD R;
							DoDragDrop(DataObject, new CDropSource(TRUE), DROPEFFECT_COPY | DROPEFFECT_LINK, &R);
							DataObject->Release();
						}
						Folder->Release();
					}
					Desktop->Release();
					Malloc->Release();
				}
			}
		}
		return 0;
	}

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	if (!m_pPrintPreview){
		return 0;
	}
	else {
		return m_pPrintPreview->OnMouseMove( wParam, lParam );
	}
}




LRESULT CEditWnd::OnMouseWheel( WPARAM wParam, LPARAM lParam )
{
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	/* 印刷プレビューモードか */
	if( !m_pPrintPreview ){
		/* メッセージの配送 */
		return m_cEditDoc.DispatchEvent( m_hWnd, WM_MOUSEWHEEL, wParam, lParam );
	}
	return m_pPrintPreview->OnMouseWheel( wParam, lParam );
}

/* 印刷ページ設定
	印刷プレビュー時にも、そうでないときでも呼ばれる可能性がある。
*/
BOOL CEditWnd::OnPrintPageSetting( void )
{
	/* 印刷設定（CANCEL押したときに破棄するための領域） */
	CDlgPrintSetting	cDlgPrintSetting;
	BOOL				bRes;
	PRINTSETTING		PrintSettingArr[MAX_PRINTSETTINGARR];
	int					i;
	int					nCurrentPrintSetting;
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		PrintSettingArr[i] = m_pShareData->m_PrintSettingArr[i];
	}

//	cDlgPrintSetting.Create( m_hInstance, m_hWnd );
	nCurrentPrintSetting = m_cEditDoc.GetDocumentAttribute().m_nCurrentPrintSetting;
	bRes = cDlgPrintSetting.DoModal(
		m_hInstance,
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		m_hWnd,
		&nCurrentPrintSetting, /* 現在選択している印刷設定 */
		PrintSettingArr
	);

	if( TRUE == bRes ){
		/* 現在選択されているページ設定の番号が変更されたか */
		if( nCurrentPrintSetting !=
			m_pShareData->m_Types[m_cEditDoc.GetDocumentType()].m_nCurrentPrintSetting
		){
//			/* 変更フラグ(タイプ別設定) */
			m_cEditDoc.GetDocumentAttribute().m_nCurrentPrintSetting = nCurrentPrintSetting;
		}

//		/* それぞれのページ設定の内容が変更されたか */
//		if( 0 != memcmp(
//			&m_pShareData->m_PrintSettingArr,
//			&PrintSettingArr,
//			sizeof( m_pShareData->m_PrintSettingArr ) )
//		){
//			m_pShareData->m_bPrintSettingModify = TRUE;	/* 変更フラグ(印刷の全体) */
			for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
//				if( 0 != memcmp(
//					&m_pShareData->m_PrintSettingArr[i],
//					&PrintSettingArr[i],
//					sizeof( m_pShareData->m_PrintSettingArr[i] ) )
//				){
//					/* 変更フラグ(印刷設定ごと) */
//					m_pShareData->m_bPrintSettingModifyArr[i] = TRUE;
					m_pShareData->m_PrintSettingArr[i] = PrintSettingArr[i];
//				}
			}
//		}

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		//	印刷プレビュー時のみ。
		if ( m_pPrintPreview ){
			/* 現在の印刷設定 */
			m_pPrintPreview->SetPrintSetting( &m_pShareData->m_PrintSettingArr[m_cEditDoc.GetDocumentAttribute().m_nCurrentPrintSetting] );

			/* 印刷プレビュー スクロールバー初期化 */
			m_pPrintPreview->InitPreviewScrollBar();

			/* 印刷設定の反映 */
			m_pPrintPreview->OnChangePrintSetting( );

			::InvalidateRect( m_hWnd, NULL, TRUE );
		}
	}
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	::UpdateWindow( m_hWnd /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);
	return bRes;
}

///////////////////////////// by 鬼

LRESULT CEditWnd::OnNcLButtonDown(WPARAM wp, LPARAM lp)
{
	LRESULT Result;
	if(wp == HTSYSMENU)
	{
		SetCapture(m_hWnd);
		m_IconClicked = icDown;
		Result = 0;
	}
	else
		Result = DefWindowProc(m_hWnd, WM_NCLBUTTONDOWN, wp, lp);

	return Result;
}

LRESULT CEditWnd::OnNcLButtonUp(WPARAM wp, LPARAM lp)
{
	LRESULT Result;
	if(m_IconClicked != icNone)
	{
		//念のため
		ReleaseCapture();
		m_IconClicked = icNone;
		Result = 0;
	}
	else if(wp == HTSYSMENU)
		Result = 0;
	else
		Result = DefWindowProc(m_hWnd, WM_NCLBUTTONDOWN, wp, lp);

	return Result;
}

LRESULT CEditWnd::OnLButtonDblClk(WPARAM wp, LPARAM lp) //by 鬼(2)
{
	LRESULT Result;
	if(m_IconClicked != icNone)
	{
		ReleaseCapture();
		m_IconClicked = icDoubleClicked;

		SendMessage(m_hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);

		Result = 0;
	}
	else
		Result = DefWindowProc(m_hWnd, WM_NCLBUTTONDOWN, wp, lp);

	return Result;
}

void CEditWnd::OnSysMenuTimer() //by 鬼(2)
{
	if(m_IconClicked == icClicked)
	{
		ReleaseCapture();

		//システムメニュー表示
		HMENU SysMenu = GetSystemMenu(m_hWnd, FALSE);
		RECT R;
		GetWindowRect(m_hWnd, &R);
		DWORD Cmd = TrackPopupMenu(SysMenu, TPM_RETURNCMD | TPM_LEFTBUTTON |
						TPM_LEFTALIGN | TPM_TOPALIGN,
						(R.left + GetSystemMetrics(SM_CXFRAME) > 0)? R.left + GetSystemMetrics(SM_CXFRAME) : 0,
						R.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME),
						0, m_hWnd, NULL);
		if(Cmd != 0)
			SendMessage(m_hWnd, WM_SYSCOMMAND, Cmd, 0);
	}
	m_IconClicked = icNone;
}

/*! ドロップダウンメニュー(開く) */	//@@@ 2002.06.15 MIK
int	CEditWnd::CreateFileDropDownMenu( HWND hwnd )
{
	int			nId;
	HMENU		hMenu;
	HMENU		hMenuPopUp;
	POINT		po;
	RECT		rc;

	m_CMenuDrawer.ResetContents();

	/* MRUリストのファイルのリストをメニューにする */
	CMRU cMRU;
	hMenu = cMRU.CreateMenu( &m_CMenuDrawer );
	if( cMRU.Length() > 0 )
	{
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	}

	/* 最近使ったフォルダのメニューを作成 */
	CMRUFolder cMRUFolder;
	hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
	if ( cMRUFolder.Length() > 0 )
	{
		//	アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, "最近使ったフォルダ(&D)" );
	}
	else 
	{
		//	非アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp, "最近使ったフォルダ(&D)" );
	}

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, "新規作成(&N)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, "開く(&O)...", FALSE );

	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );

	rc.left   = 0;
	rc.right  = 0;
	rc.top    = 0;
	rc.bottom = 0;
	::GetWindowRect( hwnd, &rc );
	po.x = po.x /*rc.left*/;
	po.y = rc.bottom - 2;

	rc.left   = 0;
	rc.right  = 0;
	rc.top    = 0;
	rc.bottom = 0;

	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		,
		po.x,
		po.y,
		0,
		hwnd,
		&rc
	);

	::DestroyMenu( hMenu );

	return nId;
}

/*! 検索ボックスでの処理 */
void CEditWnd::ProcSearchBox( MSG *msg )
{
	if( msg->message == WM_KEYDOWN /* && ::GetParent( msg->hwnd ) == m_hwndSearchBox */ )
	{
		if( (TCHAR)msg->wParam == VK_RETURN )  //リターンキー
		{
			//検索キーワードを取得
			char	szText[_MAX_PATH];
			memset( szText, 0, sizeof(szText) );
			::SendMessage( m_hwndSearchBox, WM_GETTEXT, _MAX_PATH - 1, (LPARAM)szText );
			if( szText[0] )	//キー文字列がある
			{
				//検索キーを登録
				CShareData::getInstance()->AddToSearchKeyArr( (const char*)szText );

				//::SetFocus( m_hWnd );	//先にフォーカスを移動しておかないとキャレットが消える
				::SetFocus( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_hWnd );

				// 検索開始時のカーソル位置登録条件を変更 02/07/28 ai start
				m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nSrchStartPosX_PHY = m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosX_PHY;
				m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nSrchStartPosY_PHY = m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosY_PHY;
				// 02/07/28 ai end

				//次を検索
				OnCommand( (WORD)0 /*メニュー*/, (WORD)F_SEARCH_NEXT, (HWND)0 );
			}
		}
		else if( (TCHAR)msg->wParam == VK_TAB )	//タブキー
		{
			//フォーカスを移動
			::SetFocus( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_hWnd );
		}
	}

	return;
}

/*!
	@brief ウィンドウのアイコン設定

	指定されたアイコンをウィンドウに設定する．
	以前のアイコンは破棄する．

	@param hIcon [in] 設定するアイコン
	@param flag [in] アイコン種別．ICON_BIGまたはICON_SMALL.
	@author genta
	@date 2002.09.10
*/
void CEditWnd::SetWindowIcon(HICON hIcon, int flag)
{
	HICON hOld = (HICON)::SendMessage( m_hWnd, WM_SETICON, flag, (LPARAM)hIcon );
	if( hOld != NULL ){
		::DestroyIcon( hOld );
	}
}

/*!
	標準アイコンの取得

	@param hIconBig   [out] 大きいアイコンのハンドル
	@param hIconSmall [out] 小さいアイコンのハンドル

	@author genta
	@date 2002.09.10
	@date 2002.12.02 genta 新設した共通関数を使うように
*/
void CEditWnd::GetDefaultIcon( HICON& hIconBig, HICON& hIconSmall ) const
{
	hIconBig = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false );
	hIconSmall = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
}

/*!
	アイコンの取得
	
	指定されたファイル名に対応するアイコン(大・小)を取得して返す．
	
	@param szFile     [in] ファイル名
	@param hIconBig   [out] 大きいアイコンのハンドル
	@param hIconSmall [out] 小さいアイコンのハンドル
	
	@retval true 関連づけられたアイコンが見つかった
	@retval false 関連づけられたアイコンが見つからなかった
	
	@author genta
	@date 2002.09.10
*/
bool CEditWnd::GetRelatedIcon(const char* szFile, HICON& hIconBig, HICON& hIconSmall) const
{
	if( NULL != szFile && szFile[0] != '\0' ){
		char szExt[_MAX_EXT];
		char FileType[1024];

		// (.で始まる)拡張子の取得
		_splitpath( szFile, NULL, NULL, NULL, szExt );
		
		if( ReadRegistry(HKEY_CLASSES_ROOT, szExt, NULL, FileType, sizeof(FileType) - 13)){
			lstrcat( FileType, "\\DefaultIcon" );
			if( ReadRegistry(HKEY_CLASSES_ROOT, FileType, NULL, NULL, 0)){
				// 関連づけられたアイコンを取得する
				SHFILEINFO shfi;
				SHGetFileInfo( szFile, 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON );
				hIconBig = shfi.hIcon;
				SHGetFileInfo( szFile, 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON );
				hIconSmall = shfi.hIcon;
				return true;
			}
		}
	}

	//	標準のアイコンを返す
	GetDefaultIcon( hIconBig, hIconSmall );
	return false;
}

/*
	@brief メニューバー表示用フォントの初期化
	
	メニューバー表示用フォントの初期化を行う．
	
	@date 2002.12.04 CEditViewのコンストラクタから移動
*/
void CEditWnd::InitMenubarMessageFont(void)
{
	TEXTMETRIC	tm;
	LOGFONT		lf;
	HDC			hdc;
	HFONT		hFontOld;
	int			i;

	/* LOGFONTの初期化 */
	memset( &lf, 0, sizeof( LOGFONT ) );
	lf.lfHeight			= -12;
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0x0;
	lf.lfUnderline		= 0x0;
	lf.lfStrikeOut		= 0x0;
	lf.lfCharSet		= 0x80;
	lf.lfOutPrecision	= 0x3;
	lf.lfClipPrecision	= 0x2;
	lf.lfQuality		= 0x1;
	lf.lfPitchAndFamily	= 0x31;
	strcpy( lf.lfFaceName, "ＭＳ ゴシック" );
	m_hFontCaretPosInfo = ::CreateFontIndirect( &lf );

	hdc = ::GetDC( ::GetDesktopWindow() );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontCaretPosInfo );
	::GetTextMetrics( hdc, &tm );
	m_nCaretPosInfoCharWidth = tm.tmAveCharWidth;
	m_nCaretPosInfoCharHeight = tm.tmHeight;
	for( i = 0; i < ( sizeof( m_pnCaretPosInfoDx ) / sizeof( m_pnCaretPosInfoDx[0] ) ); ++i ){
		m_pnCaretPosInfoDx[i] = ( m_nCaretPosInfoCharWidth );
	}
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( ::GetDesktopWindow(), hdc );
}

/*
	@brief メニューバーにメッセージを表示する
	
	事前にメニューバー表示用フォントが初期化されていなくてはならない．
	指定できる文字数は最大30バイト．それ以上の場合はうち切って表示する．
	
	@author genta
	@date 2002.12.04
*/
void CEditWnd::PrintMenubarMessage( const char* msg )
{
	HDC		hdc;
	POINT	po,poFrame;
	RECT	rc,rcFrame;
	HFONT	hFontOld;
	int		nStrLen;
	const int MAX_LEN = 30;
	char	szText[MAX_LEN + 1];
	
	int len = strlen( msg );
	strncpy( szText, msg, MAX_LEN );
	
	if( len < MAX_LEN ){
		memset( szText + len, ' ', MAX_LEN - len );
	}
	szText[MAX_LEN] = '\0';
	
	hdc = ::GetWindowDC( m_hWnd );
	poFrame.x = 0;
	poFrame.y = 0;
	::ClientToScreen( m_hWnd, &poFrame );
	::GetWindowRect( m_hWnd, &rcFrame );
	po.x = rcFrame.right - rcFrame.left;
	po.y = poFrame.y - rcFrame.top;
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontCaretPosInfo );
	nStrLen = MAX_LEN;
	rc.left = po.x - nStrLen * m_nCaretPosInfoCharWidth - 5;
	rc.right = rc.left + nStrLen * m_nCaretPosInfoCharWidth;
	rc.top = po.y - m_nCaretPosInfoCharHeight - 2;
	rc.bottom = rc.top + m_nCaretPosInfoCharHeight;
	::SetTextColor( hdc, ::GetSysColor( COLOR_MENUTEXT ) );
	::SetBkColor( hdc, ::GetSysColor( COLOR_MENU ) );
	::ExtTextOut( hdc,rc.left,rc.top,ETO_OPAQUE,&rc,szText,nStrLen,m_pnCaretPosInfoDx);
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( m_hWnd, hdc );
}

/*!
	@brief メッセージの表示
	
	指定されたメッセージをステータスバーに表示する．
	ステータスバーが非表示の場合はメニューバーの右端に表示する．
	
	@param msg [in] 表示するメッセージ
	@date 2002.01.26 hor 新規作成
	@date 2002.12.04 genta CEditViewより移動
*/
void CEditWnd::SendStatusMessage( const char* msg )
{
	if( NULL == m_hwndStatusBar ){
		// メニューバーへ
		PrintMenubarMessage( msg );
	}else{
		// ステータスバーへ
		::SendMessage( m_hwndStatusBar,SB_SETTEXT,0 | SBT_NOBORDERS,(LPARAM) (LPINT)msg );
	}
}
/*[EOF]*/
