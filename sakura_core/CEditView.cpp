/*!	@file
	@brief 文書ウィンドウの管理

	@author Norio Nakatani
	@date	1998/03/13 作成
	@date   2005/09/02 D.S.Koba GetSizeOfCharで書き換え
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, かろと, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta
	Copyright (C) 2007, ryoji, じゅうじ, maru

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include "CEditView.h"
#include "debug.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include "CEditWnd.h"
#include "CDlgCancel.h"
#include "CLayout.h"/// 2002/2/3 aroka
#include "COpe.h"///
#include "COpeBlk.h"///
#include "CDropTarget.h"///
#include "CSplitBoxWnd.h"///
#include "CRegexKeyword.h"///	//@@@ 2001.11.17 add MIK
#include "CMarkMgr.h"///
#include "COsVersionInfo.h"
#include "io/CFileLoad.h" // 2002/08/30 Moca
#include "CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
#include "CClipboard.h"
#include "CTypeSupport.h"
#include "parse/CWordParse.h"
#include "convert/CConvert.h"
#include "charset/CCodeMediator.h"
#include "charset/CShiftJis.h"
#include "util/input.h"
#include "util/os.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "util/module.h"
#include "util/tchar_template.h"


#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL	0x020A
#endif

// novice 2004/10/10 マウスサイドボタン対応
#ifndef WM_XBUTTONDOWN
	#define WM_XBUTTONDOWN   0x020B
	#define WM_XBUTTONUP     0x020C
	#define WM_XBUTTONDBLCLK 0x020D
#endif
#ifndef XBUTTON1
	#define XBUTTON1 0x0001
	#define XBUTTON2 0x0002
#endif

#ifndef IMR_RECONVERTSTRING
#define IMR_RECONVERTSTRING             0x0004
#endif // IMR_RECONVERTSTRING

/* 2002.04.09 minfu 再変換調整 */
#ifndef IMR_CONFIRMRECONVERTSTRING
#define IMR_CONFIRMRECONVERTSTRING             0x0005
#endif // IMR_CONFIRMRECONVERTSTRING

const int STRNCMP_MAX = 100;	/* MAXキーワード長：strnicmp文字列比較最大値(CEditView::KeySearchCore) */	// 2006.04.10 fon

CEditView*	g_m_pcEditView;
LRESULT CALLBACK EditViewWndProc( HWND, UINT, WPARAM, LPARAM );
VOID CALLBACK EditViewTimerProc( HWND, UINT, UINT, DWORD );

#define IDT_ROLLMOUSE	1

/* リソースヘッダー */
#define	 BFT_BITMAP		0x4d42	  /* 'BM' */

/* リソースがDIBかどうかを判断するマクロ */
#define	 ISDIB(bft)		((bft) == BFT_BITMAP)

/* 指定された値を最も近いバイト境界に整列させるマクロ */
#define	 WIDTHBYTES(i)	((i+31)/32*4)


/*
	@date 2006.01.16 Moca 他のTYMEDが利用可能でも、取得できるように変更。
	@note IDataObject::GetData() で tymed = TYMED_HGLOBAL を指定すること。
*/
BOOL IsDataAvailable( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC	fe;

	// 2006.01.16 Moca 他のTYMEDが利用可能でも、IDataObject::GetData()で
	//  tymed = TYMED_HGLOBALを指定すれば問題ない
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = TYMED_HGLOBAL;
	// 2006.03.16 Moca S_FALSEでも受け入れてしまうバグを修正(ファイルのドロップ等)
	return S_OK == pDataObject->QueryGetData( &fe );
}

HGLOBAL GetGlobalData( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC fe;
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	// 2006.01.16 Moca fe.tymed = -1からTYMED_HGLOBALに変更。
	fe.tymed = TYMED_HGLOBAL;

	HGLOBAL hDest = NULL;
	STGMEDIUM stgMedium;
	// 2006.03.16 Moca SUCCEEDEDマクロではS_FALSEのとき困るので、S_OKに変更
	if( S_OK == pDataObject->GetData( &fe, &stgMedium ) ){
		if( stgMedium.pUnkForRelease == NULL ){
			if( stgMedium.tymed == TYMED_HGLOBAL )
				hDest = stgMedium.hGlobal;
		}else{
			if( stgMedium.tymed == TYMED_HGLOBAL ){
				DWORD nSize = ::GlobalSize( stgMedium.hGlobal );
				hDest = ::GlobalAlloc( GMEM_SHARE|GMEM_MOVEABLE, nSize );
				if( hDest != NULL ){
					// copy the bits
					LPVOID lpSource = ::GlobalLock( stgMedium.hGlobal );
					LPVOID lpDest = ::GlobalLock( hDest );
					memcpy_raw( lpDest, lpSource, nSize );
					::GlobalUnlock( hDest );
					::GlobalUnlock( stgMedium.hGlobal );
				}
			}
			::ReleaseStgMedium( &stgMedium );
		}
	}
	return hDest;
}


/*
|| ウィンドウプロシージャ
||
*/

LRESULT CALLBACK EditViewWndProc(
	HWND		hwnd,	// handle of window
	UINT		uMsg,	// message identifier
	WPARAM		wParam,	// first message parameter
	LPARAM		lParam 	// second message parameter
)
{
//	DBPRINT_W(L"EditViewWndProc(0x%08X): %ls\n", hwnd, GetWindowsMessageName(uMsg));

	CEditView*	pCEdit;
	switch( uMsg ){
	case WM_CREATE:
		pCEdit = ( CEditView* )g_m_pcEditView;
		return pCEdit->DispatchEvent( hwnd, uMsg, wParam, lParam );
	default:
		pCEdit = ( CEditView* )::GetWindowLongPtr( hwnd, 0 );
		if( NULL != pCEdit ){
			//	May 16, 2000 genta
			//	From Here
			if( uMsg == WM_COMMAND ){
				::SendMessageCmd( ::GetParent( pCEdit->m_hwndParent ), WM_COMMAND, wParam,  lParam );
			}
			else{
				return pCEdit->DispatchEvent( hwnd, uMsg, wParam, lParam );
			}
			//	To Here
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}


/*
||  タイマーメッセージのコールバック関数
||
||	現在は、マウスによる領域選択時のスクロール処理のためにタイマーを使用しています。
*/
VOID CALLBACK EditViewTimerProc(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT idEvent,	// timer identifier
	DWORD dwTime 	// current system time
)
{
	CEditView*	pCEditView;
	pCEditView = ( CEditView* )::GetWindowLongPtr( hwnd, 0 );
	if( NULL != pCEditView ){
		pCEditView->OnTimer( hwnd, uMsg, idEvent, dwTime );
	}
	return;
}


#pragma warning(disable:4355) //「thisポインタが初期化リストで使用されました」の警告を無効化

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CEditView::CEditView(CEditWnd* pcEditWnd)
: CViewCalc(this)
, m_pcEditWnd(pcEditWnd)
, m_cParser(this)
, m_cTextDrawer(this)
, m_cCommander(this)
, m_cViewSelect(this)
, AT_ImmSetReconvertString(NULL)
, m_bActivateByMouse( FALSE )	// 2007.10.02 nasukoji
{
}


// 2007.10.23 kobake コンストラクタ内の処理をすべてCreateに移しました。(初期化処理が不必要に分散していたため)
BOOL CEditView::Create(
	HINSTANCE	hInstance,	//!< アプリケーションのインスタンスハンドル
	HWND		hwndParent,	//!< 親
	CEditDoc*	pcEditDoc,	//!< 参照するドキュメント
	int			nMyIndex,	//!< ビューのインデックス
	BOOL		bShow		//!< 作成時に表示するかどうか
)
{
	m_pcTextArea = new CTextArea(this);
	m_pcCaret = new CCaret(this, pcEditDoc);
	m_pcRuler = new CRuler(this, pcEditDoc);
	m_pcFontset = new CViewFont();

	m_cHistory = new CAutoMarkMgr;
	m_cRegexKeyword = NULL;				// 2007.04.08 ryoji

	SetDrawSwitch(true);
	m_pcDropTarget = new CDropTarget( this );
	m_bDragSource = FALSE;
	_SetDragMode(FALSE);					/* 選択テキストのドラッグ中か */
	m_bCurSrchKeyMark = false;				/* 検索文字列 */
	//	Jun. 27, 2001 genta
	m_szCurSrchKey[0] = L'\0';

	m_sCurSearchOption.Reset();				// 検索／置換 オプション

	m_bExecutingKeyMacro = false;			/* キーボードマクロの実行中 */
	m_nMyIndex = 0;

	//	Dec. 4, 2002 genta
	//	メニューバーへのメッセージ表示機能はCEditWndへ移管

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();
	m_bCommandRunning = FALSE;	/* コマンドの実行中 */
	m_pcOpeBlk = NULL;			/* 操作ブロック */
	m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */
	m_pcsbwVSplitBox = NULL;	/* 垂直分割ボックス */
	m_pcsbwHSplitBox = NULL;	/* 水平分割ボックス */
	m_pszAppName = _T("EditorClient");
	m_hInstance = NULL;
	m_hwndVScrollBar = NULL;
	m_nVScrollRate = 1;			/* 垂直スクロールバーの縮尺 */
	m_hwndHScrollBar = NULL;
	m_hwndSizeBox = NULL;

	m_ptSrchStartPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));	//検索/置換開始時のカーソル位置  (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	m_bSearch = FALSE;					// 検索/置換開始位置を登録するか */											// 02/06/26 ai
	
	m_ptBracketPairPos_PHY.Set(CLogicInt(-1), CLogicInt(-1)); // 対括弧の位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	m_ptBracketCaretPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));

	m_bDrawBracketPairFlag = FALSE;	/* 03/02/18 ai */
	GetSelectionInfo().m_bDrawSelectArea = false;	/* 選択範囲を描画したか */	// 02/12/13 ai

	m_crBack = -1;				/* テキストの背景色 */			// 2006.12.16 ryoji



	/* ルーラー表示 */
	GetTextArea().SetAreaTop(GetTextArea().GetAreaTop()+m_pShareData->m_Common.m_sWindow.m_nRulerHeight);	/* ルーラー高さ */
	GetRuler().SetRedrawFlag();	// ルーラー全体を描き直す時=true   2002.02.25 Add By KK
	m_hdcCompatDC = NULL;		/* 再描画用コンパチブルＤＣ */
	m_hbmpCompatBMP = NULL;		/* 再描画用メモリＢＭＰ */
	m_hbmpCompatBMPOld = NULL;	/* 再描画用メモリＢＭＰ(OLD) */

	//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
	//	2007.08.12 genta 初期化にShareDataの値が必要になった
	m_CurRegexp.Init(m_pShareData->m_Common.m_sSearch.m_szRegexpLib );

	// 2004.02.08 m_hFont_ZENは未使用により削除
	m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
	m_bInMenuLoop = FALSE;				/* メニュー モーダル ループに入っています */
//	MYTRACE_A( "CEditView::CEditView()おわり\n" );
	m_bHokan = FALSE;

	m_hFontOld = NULL;

	//	Aug. 31, 2000 genta
	m_cHistory->SetMax( 30 );

	// from here  2002.04.09 minfu OSによって再変換の方式を変える
	//	YAZAKI COsVersionInfoのカプセル化は守りましょ。
	COsVersionInfo	cOs;
	if( cOs.OsDoesNOTSupportReconvert() ){
		// 95 or NTならば
		m_uMSIMEReconvertMsg = ::RegisterWindowMessage( RWM_RECONVERT );
		m_uATOKReconvertMsg = ::RegisterWindowMessage( MSGNAME_ATOK_RECONVERT ) ;
		m_uWM_MSIME_RECONVERTREQUEST = ::RegisterWindowMessage(_T("MSIMEReconvertRequest"));
		
		m_hAtokModule = LoadLibrary(_T("ATOK10WC.DLL"));
		AT_ImmSetReconvertString = NULL;
		if ( NULL != m_hAtokModule ) {
			AT_ImmSetReconvertString =(BOOL (WINAPI *)( HIMC , int ,PRECONVERTSTRING , DWORD  ) ) GetProcAddress(m_hAtokModule,"AT_ImmSetReconvertString");
		}
	}
	else{ 
		// それ以外のOSのときはOS標準を使用する
		m_uMSIMEReconvertMsg = 0;
		m_uATOKReconvertMsg = 0 ;
		m_hAtokModule = 0;	//@@@ 2002.04.14 MIK
	}
	// to here  2002.04.10 minfu
	
	//2004.10.23 isearch
	m_nISearchMode = 0;
	m_pcmigemo = NULL;

	// 2007.10.02 nasukoji
	m_dwTripleClickCheck = 0;		// トリプルクリックチェック用時刻初期化



	//↑今までコンストラクタでやってたこと
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//↓今までCreateでやってたこと

	WNDCLASS	wc;
	HDC			hdc;
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;
	m_pcEditDoc = pcEditDoc;
	m_nMyIndex = nMyIndex;

	//	2007.08.18 genta 初期化にShareDataの値が必要になった
	m_cRegexKeyword = new CRegexKeyword( m_pShareData->m_Common.m_sSearch.m_szRegexpLib );	//@@@ 2001.11.17 add MIK
	m_cRegexKeyword->RegexKeySetTypes(&(m_pcEditDoc->m_cDocType.GetDocumentAttribute()));	//@@@ 2001.11.17 add MIK

	GetTextArea().SetTopYohaku( m_pShareData->m_Common.m_sWindow.m_nRulerBottomSpace ); 	/* ルーラーとテキストの隙間 */
	GetTextArea().SetAreaTop( GetTextArea().GetTopYohaku() );								/* 表示域の上端座標 */
	/* ルーラー表示 */
	if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		GetTextArea().SetAreaTop( GetTextArea().GetAreaTop() + m_pShareData->m_Common.m_sWindow.m_nRulerHeight);	/* ルーラー高さ */
	}


	/* ウィンドウクラスの登録 */
	//	Apr. 27, 2000 genta
	//	サイズ変更時のちらつきを抑えるためCS_HREDRAW | CS_VREDRAW を外した
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= (WNDPROC)EditViewWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= sizeof( LONG_PTR );
	wc.hInstance		= m_hInstance;
	wc.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
	wc.hCursor			= NULL/*LoadCursor( NULL, IDC_IBEAM )*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_WINDOW + 1)*/;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= m_pszAppName;
	if( 0 == ::RegisterClass( &wc ) ){
	}

	/* エディタウィンドウの作成 */
	g_m_pcEditView = this;
	SetHwnd(
		::CreateWindowEx(
			WS_EX_STATICEDGE,		// extended window style
			m_pszAppName,			// pointer to registered class name
			m_pszAppName,			// pointer to window name
			0						// window style
			| WS_VISIBLE
			| WS_CHILD
			| WS_CLIPCHILDREN
			,
			CW_USEDEFAULT,			// horizontal position of window
			0,						// vertical position of window
			CW_USEDEFAULT,			// window width
			0,						// window height
			hwndParent,				// handle to parent or owner window
			NULL,					// handle to menu or child-window identifier
			m_hInstance,			// handle to application instance
			(LPVOID)this			// pointer to window-creation data
		)
	);
	if( NULL == GetHwnd() ){
		return FALSE;
	}

	m_pcDropTarget->Register_DropTarget( GetHwnd() );

	/* 辞書Tip表示ウィンドウ作成 */
	m_cTipWnd.Create( m_hInstance, GetHwnd()/*m_pShareData->m_hwndTray*/ );

	/* 再描画用コンパチブルＤＣ */
	hdc = ::GetDC( GetHwnd() );
	m_hdcCompatDC = ::CreateCompatibleDC( hdc );
	::ReleaseDC( GetHwnd(), hdc );

	/* 垂直分割ボックス */
	m_pcsbwVSplitBox = new CSplitBoxWnd;
	m_pcsbwVSplitBox->Create( m_hInstance, GetHwnd(), TRUE );
	/* 水平分割ボックス */
	m_pcsbwHSplitBox = new CSplitBoxWnd;
	m_pcsbwHSplitBox->Create( m_hInstance, GetHwnd(), FALSE );

	/* スクロールバー作成 */
	CreateScrollBar();		// 2006.12.19 ryoji

	SetFont();

	if( bShow ){
		ShowWindow( GetHwnd(), SW_SHOW );
	}

	/* 親ウィンドウのタイトルを更新 */
	m_pcEditWnd->UpdateCaption();

	/* キーボードの現在のリピート間隔を取得 */
	int nKeyBoardSpeed;
	SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0 );

	/* タイマー起動 */
	if( 0 == ::SetTimer( GetHwnd(), IDT_ROLLMOUSE, nKeyBoardSpeed, (TIMERPROC)EditViewTimerProc ) ){
		WarningMessage( GetHwnd(), _T("CEditView::Create()\nタイマーが起動できません。\nシステムリソースが不足しているのかもしれません。") );
	}

	return TRUE;
}


CEditView::~CEditView()
{
	if( GetHwnd() != NULL ){
		DestroyWindow( GetHwnd() );
	}

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

	delete m_pcDropTarget;
	m_pcDropTarget = NULL;

	delete m_cHistory;

	delete m_cRegexKeyword;	//@@@ 2001.11.17 add MIK
	
	//再変換 2002.04.10 minfu
	if(m_hAtokModule)
		FreeLibrary(m_hAtokModule);

	delete m_pcTextArea;
	delete m_pcCaret;
	delete m_pcRuler;
	delete m_pcFontset;
}





/*! スクロールバー作成
	@date 2006.12.19 ryoji 新規作成（CEditView::Createから分離）
*/
BOOL CEditView::CreateScrollBar( void )
{
	SCROLLINFO	si;

	/* スクロールバーの作成 */
	m_hwndVScrollBar = ::CreateWindowEx(
		0L,									/* no extended styles */
		_T("SCROLLBAR"),						/* scroll bar control class */
		NULL,								/* text for window title bar */
		WS_VISIBLE | WS_CHILD | SBS_VERT,	/* scroll bar styles */
		0,									/* horizontal position */
		0,									/* vertical position */
		200,								/* width of the scroll bar */
		CW_USEDEFAULT,						/* default height */
		GetHwnd(),								/* handle of main window */
		(HMENU) NULL,						/* no menu for a scroll bar */
		m_hInstance,						/* instance owning this window */
		(LPVOID) NULL						/* pointer not needed */
	);
	si.cbSize = sizeof( si );
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin  = 0;
	si.nMax  = 29;
	si.nPage = 10;
	si.nPos  = 0;
	si.nTrackPos = 1;
	::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
	::ShowScrollBar( m_hwndVScrollBar, SB_CTL, TRUE );

	/* スクロールバーの作成 */
	m_hwndHScrollBar = NULL;
	if( m_pShareData->m_Common.m_sWindow.m_bScrollBarHorz ){	/* 水平スクロールバーを使う */
		m_hwndHScrollBar = ::CreateWindowEx(
			0L,									/* no extended styles */
			_T("SCROLLBAR"),						/* scroll bar control class */
			NULL,								/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_HORZ,	/* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			GetHwnd(),								/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = 29;
		si.nPage = 10;
		si.nPos  = 0;
		si.nTrackPos = 1;
		::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );
		::ShowScrollBar( m_hwndHScrollBar, SB_CTL, TRUE );
	}


	/* サイズボックス */
	if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 ){	/* ファンクションキー表示位置／0:上 1:下 */
		m_hwndSizeBox = ::CreateWindowEx(
			WS_EX_CONTROLPARENT/*0L*/, 			/* no extended styles */
			_T("SCROLLBAR"),						/* scroll bar control class */
			NULL,								/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			GetHwnd(), 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}else{
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 								/* no extended styles */
			_T("STATIC"),						/* scroll bar control class */
			NULL,								/* text for window title bar */
			WS_VISIBLE | WS_CHILD/* | SBS_SIZEBOX | SBS_SIZEGRIP*/, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			GetHwnd(), 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}
	return TRUE;
}



/*! スクロールバー破棄
	@date 2006.12.19 ryoji 新規作成
*/
void CEditView::DestroyScrollBar( void )
{
	if( m_hwndVScrollBar )
	{
		::DestroyWindow( m_hwndVScrollBar );
		m_hwndVScrollBar = NULL;
	}

	if( m_hwndHScrollBar )
	{
		::DestroyWindow( m_hwndHScrollBar );
		m_hwndHScrollBar = NULL;
	}

	if( m_hwndSizeBox )
	{
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = NULL;
	}
}



//TCHAR→WCHAR変換。
inline wchar_t tchar_to_wchar(TCHAR tc)
{
#ifdef _UNICODE
	return tc;
#else
	WCHAR wc=0;
	mbtowc(&wc,&tc,sizeof(tc));
	return wc;
#endif
}

/*
|| メッセージディスパッチャ
*/
LRESULT CEditView::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	HDC			hdc;
	int			nPosX;
	int			nPosY;

	switch ( uMsg ){
	case WM_MOUSEWHEEL:
		if( m_pcEditDoc->m_pcEditWnd->DoMouseWheel( wParam, lParam ) ){
			return 0L;
		}
		return OnMOUSEWHEEL( wParam, lParam );

	case WM_CREATE:
		::SetWindowLongPtr( hwnd, 0, (LONG_PTR) this );

		return 0L;

	case WM_SIZE:
		OnSize( LOWORD( lParam ), HIWORD( lParam ) );
		return 0L;

	case WM_SETFOCUS:
		OnSetFocus();

		/* 親ウィンドウのタイトルを更新 */
		m_pcEditWnd->UpdateCaption();

		return 0L;
	case WM_KILLFOCUS:
		OnKillFocus();

		return 0L;
	case WM_CHAR:
		GetCommander().HandleCommand( F_WCHAR, TRUE, tchar_to_wchar((TCHAR)wParam), 0, 0, 0 );
		return 0L;

	case WM_IME_NOTIFY:	// Nov. 26, 2006 genta
		if( wParam == IMN_SETCONVERSIONMODE || wParam == IMN_SETOPENSTATUS){
			GetCaret().ShowEditCaret();
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_IME_COMPOSITION:
		if( IsInsMode() && (lParam & GCS_RESULTSTR)){
			HIMC hIMC;
			DWORD dwSize;
			HGLOBAL hstr;
			hIMC = ImmGetContext( hwnd );

			if( !hIMC ){
				return 0;
//				MyError( ERROR_NULLCONTEXT );
			}

			// Get the size of the result string.
			dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);

			// increase buffer size for NULL terminator,
			//	maybe it is in Unicode
			dwSize += sizeof( WCHAR );

			hstr = GlobalAlloc( GHND, dwSize );
			if( hstr == NULL ){
				return 0;
//				 MyError( ERROR_GLOBALALLOC );
			}

			LPTSTR lptstr;
			lptstr = (LPTSTR)GlobalLock( hstr );
			if( lptstr == NULL ){
				return 0;
//				 MyError( ERROR_GLOBALLOCK );
			}

			// Get the result strings that is generated by IME into lptstr.
			ImmGetCompositionString(hIMC, GCS_RESULTSTR, lptstr, dwSize);

			/* テキストを貼り付け */
			GetCommander().HandleCommand( F_INSTEXT_W, TRUE, (LPARAM)to_wchar(lptstr), TRUE, 0, 0 );

			ImmReleaseContext( hwnd, hIMC );

			// add this string into text buffer of application

			GlobalUnlock( hstr );
			GlobalFree( hstr );
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
//			return 0;
		}else{
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
	case WM_IME_CHAR:
		if( ! IsInsMode() /* Oct. 2, 2005 genta */ ){ /* 上書きモードか？ */
			GetCommander().HandleCommand( F_IME_CHAR, TRUE, wParam, 0, 0, 0 );
		}
		return 0L;

	case WM_KEYUP:
		/* キーリピート状態 */
		GetCommander().m_bPrevCommand = 0;
		return 0L;

	// 2004.04.27 Moca From Here ALT+xでALTを押したままだとキーリピートがOFFにならない対策
	case WM_SYSKEYUP:
		GetCommander().m_bPrevCommand = 0;
		// 念のため呼ぶ
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	// 2004.04.27 To Here

	case WM_LBUTTONDBLCLK:

		// 2007.10.02 nasukoji	非アクティブウィンドウのダブルクリック時はここでカーソルを移動する
		// 2007.10.12 genta フォーカス移動のため，OnLBUTTONDBLCLKより移動
		if(m_bActivateByMouse){
			/* アクティブなペインを設定 */
			m_pcEditDoc->m_pcEditWnd->SetActivePane( m_nMyIndex );
			// カーソルをクリック位置へ移動する
			OnLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );	
			// 2007.10.02 nasukoji
			m_bActivateByMouse = FALSE;		// マウスによるアクティベートを示すフラグをOFF
		}
		//		MYTRACE_A( " WM_LBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONDBLCLK( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

// novice 2004/10/11 マウス中ボタン対応
	case WM_MBUTTONDOWN:
		OnMBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );

		return 0L;

	case WM_LBUTTONDOWN:
		// 2007.10.02 nasukoji
		m_bActivateByMouse = FALSE;		// マウスによるアクティベートを示すフラグをOFF
//		MYTRACE_A( " WM_LBUTTONDOWN wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;
//	case WM_MBUTTONUP:
	case WM_LBUTTONUP:

//		MYTRACE_A( " WM_LBUTTONUP wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;
	case WM_MOUSEMOVE:
		OnMOUSEMOVE( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

	case WM_RBUTTONDBLCLK:
//		MYTRACE_A( " WM_RBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		return 0L;
//	case WM_RBUTTONDOWN:
//		MYTRACE_A( " WM_RBUTTONDOWN wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
//		OnRBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
//		if( m_nMyIndex != m_pcEditDoc->m_pcEditWnd->GetActivePane() ){
//			/* アクティブなペインを設定 */
//			m_pcEditDoc->m_pcEditWnd->SetActivePane( m_nMyIndex );
//		}
//		return 0L;
	case WM_RBUTTONUP:
//		MYTRACE_A( " WM_RBUTTONUP wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnRBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

// novice 2004/10/10 マウスサイドボタン対応
	case WM_XBUTTONDOWN:
		switch ( HIWORD(wParam) ){
		case XBUTTON1:
			OnXLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
			break;
		case XBUTTON2:
			OnXRBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
			break;
		}

		return 0L;

	case WM_VSCROLL:
//		MYTRACE_A( "	WM_VSCROLL nPos=%d\n", GetScrollPos( m_hwndVScrollBar, SB_CTL ) );
		//	Sep. 11, 2004 genta 同期スクロールの関数化
		{
			CLayoutInt Scroll = OnVScroll(
				(int) LOWORD( wParam ), ((int) HIWORD( wParam )) * m_nVScrollRate );

			//	シフトキーが押されていないときだけ同期スクロール
			if(!GetKeyState_Shift()){
				SyncScrollV( Scroll );
			}
		}

		return 0L;

	case WM_HSCROLL:
//		MYTRACE_A( "	WM_HSCROLL nPos=%d\n", GetScrollPos( m_hwndHScrollBar, SB_CTL ) );
		//	Sep. 11, 2004 genta 同期スクロールの関数化
		{
			CLayoutInt Scroll = OnHScroll(
				(int) LOWORD( wParam ), ((int) HIWORD( wParam )) );

			//	シフトキーが押されていないときだけ同期スクロール
			if(!GetKeyState_Shift()){
				SyncScrollH( Scroll );
			}
		}

		return 0L;

	case WM_ENTERMENULOOP:
		m_bInMenuLoop = TRUE;	/* メニュー モーダル ループに入っています */

		/* 辞書Tipが起動されている */
		if( 0 == m_dwTipTimer ){
			/* 辞書Tipを消す */
			m_cTipWnd.Hide();
			m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
		}
		if( m_bHokan ){
			m_pcEditDoc->m_pcEditWnd->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
		}
		return 0L;

	case WM_EXITMENULOOP:
		m_bInMenuLoop = FALSE;	/* メニュー モーダル ループに入っています */
		return 0L;


	case WM_PAINT:
		{
			PAINTSTRUCT	ps;
			hdc = ::BeginPaint( hwnd, &ps );
			OnPaint( hdc, &ps, FALSE );
			::EndPaint(hwnd, &ps);
		}
		return 0L;

	case WM_CLOSE:
//		MYTRACE_A( "	WM_CLOSE\n" );
		DestroyWindow( hwnd );
		return 0L;
	case WM_DESTROY:
//		CDropTarget::Revoke_DropTarget();
		m_pcDropTarget->Revoke_DropTarget();
//		::RevokeDragDrop( GetHwnd() );
//		::OleUninitialize();

		/* タイマー終了 */
		::KillTimer( GetHwnd(), IDT_ROLLMOUSE );


//		MYTRACE_A( "	WM_DESTROY\n" );
		/*
		||子ウィンドウの破棄
		*/
		if( NULL != m_hwndVScrollBar ){	// Aug. 20, 2005 Aroka
			DestroyWindow( m_hwndVScrollBar );
			m_hwndVScrollBar = NULL;
		}
		if( NULL != m_hwndHScrollBar ){
			DestroyWindow( m_hwndHScrollBar );
			m_hwndHScrollBar = NULL;
		}
		if( NULL != m_hwndSizeBox ){
			DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		if( NULL != m_pcsbwVSplitBox ){	/* 垂直分割ボックス */
			delete m_pcsbwVSplitBox;
			m_pcsbwVSplitBox = NULL;
		}

		if( NULL != m_pcsbwHSplitBox ){	/* 垂直分割ボックス */
			delete m_pcsbwHSplitBox;
			m_pcsbwHSplitBox = NULL;
		}


		SetHwnd(NULL);
		return 0L;

	case MYWM_DOSPLIT:
		nPosX = (int)wParam;
		nPosY = (int)lParam;
//		MYTRACE_A( "MYWM_DOSPLIT nPosX=%d nPosY=%d\n", nPosX, nPosY );
		::SendMessageAny( m_hwndParent, MYWM_DOSPLIT, wParam, lParam );
		return 0L;

	case MYWM_SETACTIVEPANE:
		m_pcEditDoc->m_pcEditWnd->SetActivePane( m_nMyIndex );
		::PostMessageAny( m_hwndParent, MYWM_SETACTIVEPANE, (WPARAM)m_nMyIndex, 0 );
		return 0L;

	case MYWM_IME_REQUEST:  /* 再変換  by minfu 2002.03.27 */ // 20020331 aroka
		
		// 2002.04.09 switch case に変更  minfu 
		switch ( wParam ){
		case IMR_RECONVERTSTRING:
			return SetReconvertStruct((PRECONVERTSTRING)lParam, UNICODE_BOOL);
			
		case IMR_CONFIRMRECONVERTSTRING:
			return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, UNICODE_BOOL);
			
		}
		
		return 0L;
	
	// 2007.10.02 nasukoji	マウスクリックにてアクティベートされた時はカーソル位置を移動しない
	case WM_MOUSEACTIVATE:
		LRESULT nRes;
		nRes = ::DefWindowProc( hwnd, uMsg, wParam, lParam );	// 親に先に処理させる
		if( nRes == MA_NOACTIVATE || nRes == MA_NOACTIVATEANDEAT ){
			return nRes;
		}

		// マウスクリックによりバックグラウンドウィンドウがアクティベートされた
		//	2007.10.08 genta オプション追加
		if( m_pShareData->m_Common.m_sGeneral.m_bNoCaretMoveByActivation &&
		   (! m_pcEditDoc->m_pcEditWnd->IsActiveApp()))
		{
			m_bActivateByMouse = TRUE;		// マウスによるアクティベート
			return MA_ACTIVATEANDEAT;		// アクティベート後イベントを破棄
		}

		/* アクティブなペインを設定 */
		m_pcEditDoc->m_pcEditWnd->SetActivePane( m_nMyIndex );

		return nRes;

	default:
// << 20020331 aroka 再変換対応 for 95/NT
		if( (m_uMSIMEReconvertMsg && (uMsg == m_uMSIMEReconvertMsg)) 
			|| (m_uATOKReconvertMsg && (uMsg == m_uATOKReconvertMsg))){
		// 2002.04.08 switch case に変更 minfu 
			switch ( wParam ){
			case IMR_RECONVERTSTRING:
				return SetReconvertStruct((PRECONVERTSTRING)lParam, true);
				
			case IMR_CONFIRMRECONVERTSTRING:
				return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, true);
				
			}
			return 0L;
		}
// >> by aroka

		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}




void CEditView::OnMove( int x, int y, int nWidth, int nHeight )
{
	MoveWindow( GetHwnd(), x, y, nWidth, nHeight, TRUE );
	return;
}




/* ウィンドウサイズの変更処理 */
void CEditView::OnSize( int cx, int cy )
{
	if( NULL == GetHwnd() ){
		return;
	}
	if( cx == 0 && cy == 0 ){
		return;
	}

	int	nVSplitHeight = 0;	/* 垂直分割ボックスの高さ */
	int	nHSplitWidth  = 0;	/* 水平分割ボックスの幅 */

	//スクロールバーのサイズ基準値を取得
	int nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	int nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	int nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	int nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );

	/* 垂直分割ボックス */
	if( NULL != m_pcsbwVSplitBox ){
		nVSplitHeight = 7;
		::MoveWindow( m_pcsbwVSplitBox->GetHwnd(), cx - nCxVScroll , 0, nCxVScroll, nVSplitHeight, TRUE );
	}
	/* 水平分割ボックス */
	if( NULL != m_pcsbwHSplitBox ){
		nHSplitWidth = 7;
		::MoveWindow( m_pcsbwHSplitBox->GetHwnd(),0, cy - nCyHScroll, nHSplitWidth, nCyHScroll, TRUE );
	}
	/* 垂直スクロールバー */
	if( NULL != m_hwndVScrollBar ){
		::MoveWindow( m_hwndVScrollBar, cx - nCxVScroll , 0 + nVSplitHeight, nCxVScroll, cy - nCyVScroll - nVSplitHeight, TRUE );
	}
	/* 水平スクロールバー */
	if( NULL != m_hwndHScrollBar ){
		::MoveWindow( m_hwndHScrollBar, 0 + nHSplitWidth, cy - nCyHScroll, cx - nCxVScroll - nHSplitWidth, nCyHScroll, TRUE );
	}

	/* サイズボックス */
	if( NULL != m_hwndSizeBox ){
		::MoveWindow( m_hwndSizeBox, cx - nCxVScroll, cy - nCyHScroll, nCxHScroll, nCyVScroll, TRUE );
	}

	// エリア情報更新
	GetTextArea().TextArea_OnSize(
		CMySize(cx,cy),
		nCxVScroll,
		m_hwndHScrollBar?nCyHScroll:0
	);

	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	/* 再描画用メモリＢＭＰ */
	if( m_hbmpCompatBMP != NULL ){
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );	/* 再描画用メモリＢＭＰ(OLD) */
		::DeleteObject( m_hbmpCompatBMP );
	}
	HDC	hdc = ::GetDC( GetHwnd() );
	m_hbmpCompatBMP = ::CreateCompatibleBitmap( hdc, cx, cy );
	m_hbmpCompatBMPOld = (HBITMAP)::SelectObject( m_hdcCompatDC, m_hbmpCompatBMP );
	::ReleaseDC( GetHwnd(), hdc );

	/* 親ウィンドウのタイトルを更新 */
	m_pcEditWnd->UpdateCaption(); // [Q] genta 本当に必要？

	return;
}


/*!	IME ONか

	@date  2006.12.04 ryoji 新規作成（関数化）
*/
bool CEditView::IsImeON( void )
{
	bool bRet;
	HIMC	hIme;
	DWORD	conv, sent;

	//	From here Nov. 26, 2006 genta
	hIme = ImmGetContext( m_hwndParent );
	if( ImmGetOpenStatus( hIme ) != FALSE ){
		ImmGetConversionStatus( hIme, &conv, &sent );
		if(( conv & IME_CMODE_NOCONVERSION ) == 0 ){
			bRet = true;
		}
		else {
			bRet = false;
		}
	}
	else {
		bRet = false;
	}
	ImmReleaseContext( m_hwndParent, hIme );
	//	To here Nov. 26, 2006 genta

	return bRet;
}




/* 入力フォーカスを受け取ったときの処理 */
void CEditView::OnSetFocus( void )
{
	// 2004.04.02 Moca EOFのみのレイアウト行は、0桁目のみ有効.EOFより下の行のある場合は、EOF位置にする
	{
		CLayoutPoint ptPos = GetCaret().GetCaretLayoutPos();
		if( GetCaret().GetAdjustCursorPos( &ptPos ) ){
			GetCaret().MoveCursor( ptPos, FALSE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
	}

	GetCaret().ShowEditCaret();

	SetIMECompFormFont();

	/* ルーラのカーソルをグレーから黒に変更する */
	HDC hdc = ::GetDC( GetHwnd() );
	GetRuler().DispRuler( hdc );
	::ReleaseDC( GetHwnd(), hdc );

	// 03/02/18 対括弧の強調表示(描画) ai
	m_bDrawBracketPairFlag = TRUE;
	DrawBracketPair( true );

	return;
}





/* 入力フォーカスを失ったときの処理 */
void CEditView::OnKillFocus( void )
{
	// 03/02/18 対括弧の強調表示(消去) ai
	DrawBracketPair( false );
	m_bDrawBracketPairFlag = FALSE;

	GetCaret().DestroyCaret();

	/* ルーラー描画 */
	/* ルーラのカーソルを黒からグレーに変更する */
	HDC	hdc = ::GetDC( GetHwnd() );
	GetRuler().DispRuler( hdc );
	::ReleaseDC( GetHwnd(), hdc );

	/* 辞書Tipが起動されている */
	if( 0 == m_dwTipTimer ){
		/* 辞書Tipを消す */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
	}

	if( m_bHokan ){
		m_pcEditDoc->m_pcEditWnd->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

	return;
}





/*! 垂直スクロールバーメッセージ処理

	@param nScrollCode [in]	スクロール種別 (Windowsから渡されるもの)
	@param nPos [in]		スクロール位置(THUMBTRACK用)
	@retval	実際にスクロールした行数

	@date 2004.09.11 genta スクロール行数を返すように．
		未使用のhwndScrollBar引数削除．
*/
CLayoutInt CEditView::OnVScroll( int nScrollCode, int nPos )
{
	CLayoutInt nScrollVal = CLayoutInt(0);

	switch( nScrollCode ){
	case SB_LINEDOWN:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( GetTextArea().GetViewTopLine() + 1 );
//		}
		nScrollVal = ScrollAtV( GetTextArea().GetViewTopLine() + m_pShareData->m_Common.m_sGeneral.m_nRepeatedScrollLineNum );
		break;
	case SB_LINEUP:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( GetTextArea().GetViewTopLine() - 1 );
//		}
		nScrollVal = ScrollAtV( GetTextArea().GetViewTopLine() - m_pShareData->m_Common.m_sGeneral.m_nRepeatedScrollLineNum );
		break;
	case SB_PAGEDOWN:
		nScrollVal = ScrollAtV( GetTextArea().GetBottomLine() );
		break;
	case SB_PAGEUP:
		nScrollVal = ScrollAtV( GetTextArea().GetViewTopLine() - GetTextArea().m_nViewRowNum );
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtV( CLayoutInt(nPos) );
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtV( CLayoutInt(nPos) );
		break;
	case SB_TOP:
		nScrollVal = ScrollAtV( CLayoutInt(0) );
		break;
	case SB_BOTTOM:
		nScrollVal = ScrollAtV(( m_pcEditDoc->m_cLayoutMgr.GetLineCount() ) - GetTextArea().m_nViewRowNum );
		break;
	default:
		break;
	}
	return nScrollVal;
}

/*! 水平スクロールバーメッセージ処理

	@param nScrollCode [in]	スクロール種別 (Windowsから渡されるもの)
	@param nPos [in]		スクロール位置(THUMBTRACK用)
	@retval	実際にスクロールした桁数

	@date 2004.09.11 genta スクロール桁数を返すように．
		未使用のhwndScrollBar引数削除．
*/
CLayoutInt CEditView::OnHScroll( int nScrollCode, int nPos )
{
	CLayoutInt nScrollVal = CLayoutInt(0);

	GetRuler().SetRedrawFlag(); // YAZAKI
	switch( nScrollCode ){
	case SB_LINELEFT:
		nScrollVal = ScrollAtH( GetTextArea().GetViewLeftCol() - CLayoutInt(4) );
		break;
	case SB_LINERIGHT:
		nScrollVal = ScrollAtH( GetTextArea().GetViewLeftCol() + CLayoutInt(4) );
		break;
	case SB_PAGELEFT:
		nScrollVal = ScrollAtH( GetTextArea().GetViewLeftCol() - GetTextArea().m_nViewColNum );
		break;
	case SB_PAGERIGHT:
		nScrollVal = ScrollAtH( GetTextArea().GetRightCol() );
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtH( CLayoutInt(nPos) );
//		MYTRACE_A( "nPos=%d\n", nPos );
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtH( CLayoutInt(nPos) );
//		MYTRACE_A( "nPos=%d\n", nPos );
		break;
	case SB_LEFT:
		nScrollVal = ScrollAtH( CLayoutInt(0) );
		break;
	case SB_RIGHT:
		//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
		nScrollVal = ScrollAtH( m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() - GetTextArea().m_nViewColNum );
		break;
	}
	return nScrollVal;
}



//void CEditView::Draw3dRect( HDC hdc, int x, int y, int cx, int cy,
//	COLORREF clrTopLeft, COLORREF clrBottomRight )
//{
//	return;
//}
//
//void CEditView::FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr )
//{
//	RECT	rc;
//	::SetBkColor( hdc, clr );
//	::SetRect( &rc, x, y, x + cx, y + cy );
//	::ExtTextOutW_AnyBuild( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );
//	return;
//}






/* フォントの変更 */
void CEditView::SetFont( void )
{
	// メトリクス更新
	this->GetTextMetrics().Update(GetFontset().GetFontHan());

	// エリア情報を更新
	HDC hdc = ::GetDC( GetHwnd() );
	GetTextArea().UpdateAreaMetrics(hdc);
	::ReleaseDC( GetHwnd(), hdc );

	// 行番号表示に必要な幅を設定
	GetTextArea().DetectWidthOfLineNumberArea( false );

	// ぜんぶ再描画
	::InvalidateRect( GetHwnd(), NULL, TRUE );

	//	Oct. 11, 2002 genta IMEのフォントも変更
	SetIMECompFormFont();
}









/* スクロールバーの状態を更新する */
void CEditView::AdjustScrollBars( void )
{
	if( !GetDrawSwitch() ){
		return;
	}


	CLayoutInt	nAllLines;
	int			nVScrollRate;
	SCROLLINFO	si;

	if( NULL != m_hwndVScrollBar ){
		/* 垂直スクロールバー */
		/* nAllLines / nVScrollRate < 65535 となる整数nVScrollRateを求める */
		nAllLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
		nAllLines+=2;
		nVScrollRate = 1;
		while( nAllLines / nVScrollRate > 65535 ){
			++nVScrollRate;
		}
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = (Int)nAllLines / nVScrollRate - 1;	/* 全行数 */
		si.nPage = (Int)GetTextArea().m_nViewRowNum / nVScrollRate;	/* 表示域の行数 */
		si.nPos  = (Int)GetTextArea().GetViewTopLine() / nVScrollRate;	/* 表示域の一番上の行(0開始) */
		si.nTrackPos = nVScrollRate;
		::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
		m_nVScrollRate = nVScrollRate;				/* 垂直スクロールバーの縮尺 */
		
		//	Nov. 16, 2002 genta
		//	縦スクロールバーがDisableになったときは必ず全体が画面内に収まるように
		//	スクロールさせる
		//	2005.11.01 aroka 判定条件誤り修正 (バーが消えてもスクロールしない)
		if( GetTextArea().m_nViewRowNum >= nAllLines ){
			ScrollAtV( CLayoutInt(0) );
		}
	}
	if( NULL != m_hwndHScrollBar ){

		/* 水平スクロールバー */
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = (Int)m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() - 1; //	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
		si.nPage = (Int)GetTextArea().m_nViewColNum;			/* 表示域の桁数 */
		si.nPos  = (Int)GetTextArea().GetViewLeftCol();		/* 表示域の一番左の桁(0開始) */
		si.nTrackPos = 1;
		::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );

		//	2006.1.28 aroka 判定条件誤り修正 (バーが消えてもスクロールしない)
		if( GetTextArea().m_nViewColNum >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() ){
			ScrollAtH( CLayoutInt(0) );
		}
	}

	return;
}



/*!	@brief 選択を考慮した行桁指定によるカーソル移動

	選択状態チェック→カーソル移動→選択領域更新という処理が
	あちこちのコマンドにあるのでまとめることにした．
	また，戻り値はほとんど使われていないのでvoidにした．

	選択状態を考慮してカーソルを移動する．
	非選択が指定された場合には既存選択範囲を解除して移動する．
	選択が指定された場合には選択範囲の開始・変更を併せて行う．
	インタラクティブ操作を前提とするため，必要に応じたスクロールを行う．
	カーソル移動後は上下移動でもカラム位置を保つよう，
	GetCaret().m_nCaretPosX_Prevの更新も併せて行う．

	@date 2006.07.09 genta 新規作成
*/
void CEditView::MoveCursorSelecting(
	CLayoutPoint	ptWk_CaretPos,		//!< [in] 移動先レイアウト位置
	bool			bSelect,			//!< true: 選択する  false: 選択解除
	int				nCaretMarginRate	//!< 縦スクロール開始位置を決める値
)
{
	if( bSelect ){
		if( !GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在のカーソル位置から選択を開始する */
			GetSelectionInfo().BeginSelectArea();
		}
	}else{
		if( GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			GetSelectionInfo().DisableSelectArea( TRUE );
		}
	}
	GetCaret().MoveCursor( ptWk_CaretPos, TRUE, nCaretMarginRate );	// 2007.08.22 ryoji nCaretMarginRateが使われていなかった
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
	if( bSelect ){
		/*	現在のカーソル位置によって選択範囲を変更．
		
			2004.04.02 Moca 
			キャレット位置が不正だった場合にMoveCursorの移動結果が
			引数で与えた座標とは異なることがあるため，
			nPosX, nPosYの代わりに実際の移動結果を使うように．
		*/
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
	}
	
}



/* IME編集エリアの位置を変更 */
void CEditView::SetIMECompFormPos( void )
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	COMPOSITIONFORM	CompForm;
	HIMC			hIMC = ::ImmGetContext( GetHwnd() );
	POINT			point;
	HWND			hwndFrame;
	hwndFrame = ::GetParent( m_hwndParent );

	::GetCaretPos( &point );
	CompForm.dwStyle = CFS_POINT;
	CompForm.ptCurrentPos.x = (long) point.x;
	CompForm.ptCurrentPos.y = (long) point.y + GetCaret().GetCaretSize().cy - GetTextMetrics().GetHankakuHeight();

	if ( hIMC ){
		::ImmSetCompositionWindow( hIMC, &CompForm );
	}
	::ImmReleaseContext( GetHwnd() , hIMC );
	return;
}





/* IME編集エリアの表示フォントを変更 */
void CEditView::SetIMECompFormFont( void )
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	HIMC	hIMC = ::ImmGetContext( GetHwnd() );
	if ( hIMC ){
		::ImmSetCompositionFont( hIMC, &(m_pShareData->m_Common.m_sView.m_lf) );
	}
	::ImmReleaseContext( GetHwnd() , hIMC );
	return;
}








/* マウス左ボタン押下 */
void CEditView::OnLBUTTONDOWN( WPARAM fwKeys, int _xPos , int _yPos )
{
	CMyPoint ptMouse(_xPos,_yPos);

	if( m_bHokan ){
		m_pcEditDoc->m_pcEditWnd->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

	//isearch 2004.10.22 isearchをキャンセルする
	if (m_nISearchMode > 0 ){
		ISearchExit();
	}

	CNativeW	cmemCurText;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;

	CLayoutRange sRange;

	CLogicInt	nIdx;
	int			nWork;
	BOOL		tripleClickMode = FALSE;	// 2007.10.02 nasukoji	トリプルクリックであることを示す
	int			nFuncID = 0;				// 2007.12.02 nasukoji	マウス左クリックに対応する機能コード

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() == 0 ){
		return;
	}
	if( !GetCaret().ExistCaretFocus() ){ //フォーカスがないとき
		return;
	}

	/* 辞書Tipが起動されている */
	if( 0 == m_dwTipTimer ){
		/* 辞書Tipを消す */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
	}
	else{
		m_dwTipTimer = ::GetTickCount();		/* 辞書Tip起動タイマー */
	}

	// 2007.12.02 nasukoji	トリプルクリックをチェック
	tripleClickMode = CheckTripleClick(ptMouse);

	if(tripleClickMode){
		// マウス左トリプルクリックに対応する機能コードはm_Common.m_pKeyNameArr[5]に入っている
		nFuncID = m_pShareData->m_pKeyNameArr[MOUSEFUNCTION_TRIPLECLICK].m_nFuncCodeArr[getCtrlKeyState()];
		if( 0 == nFuncID ){
			tripleClickMode = 0;	// 割り当て機能無しの時はトリプルクリック OFF
		}
	}else{
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
	}

	/* 現在のマウスカーソル位置→レイアウト位置 */
	CLayoutPoint ptNew;
	GetTextArea().ClientToLayout(ptMouse, &ptNew);

	// OLEによるドラッグ & ドロップを使う
	// 2007.12.02 nasukoji	トリプルクリック時はドラッグを開始しない
	if( !tripleClickMode && m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop ){
		if( m_pShareData->m_Common.m_sEdit.m_bUseOLE_DropSource ){		/* OLEによるドラッグ元にするか */
			/* 行選択エリアをドラッグした */
			if( ptMouse.x < GetTextArea().GetAreaLeft() - GetTextMetrics().GetHankakuDx() ){
				goto normal_action;
			}
			/* 指定カーソル位置が選択エリア内にあるか */
			if( 0 == IsCurrentPositionSelected(ptNew) ){
				/* 選択範囲のデータを取得 */
				if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
					DWORD dwEffects;
					m_bDragSource = TRUE;
					CDataObject data( cmemCurText.GetStringPtr() );
					dwEffects = data.DragDrop( TRUE, DROPEFFECT_COPY | DROPEFFECT_MOVE );
					m_bDragSource = FALSE;
					if( 0 == dwEffects ){
						if( GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
							/* 現在の選択範囲を非選択状態に戻す */
							GetSelectionInfo().DisableSelectArea( TRUE );
							
//@@@ 2002.01.08 YAZAKI フリーカーソルOFFで複数行選択し、行の後ろをクリックするとそこにキャレットが置かれてしまうバグ修正
							/* カーソル移動。 */
							if( ptMouse.y >= GetTextArea().GetAreaTop() && ptMouse.y < GetTextArea().GetAreaBottom() ){
								if( ptMouse.x >= GetTextArea().GetAreaLeft() && ptMouse.x < GetTextArea().GetAreaRight() ){
									GetCaret().MoveCursorToClientPoint( ptMouse );
								}
								else if( ptMouse.x < GetTextArea().GetAreaLeft() ){
									GetCaret().MoveCursorToClientPoint( CMyPoint(GetTextArea().GetDocumentLeftClientPointX(), ptMouse.y) );
								}
							}
						}
					}
				}
				return;
			}
		}
	}

normal_action:;

	// ALTキーが押されている、かつトリプルクリックでない		// 2007.11.15 nasukoji	トリプルクリック対応
	if( GetKeyState_Alt() &&( ! tripleClickMode)){
		if( GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			GetSelectionInfo().DisableSelectArea( TRUE );
		}
		if( ptMouse.y >= GetTextArea().GetAreaTop()  && ptMouse.y < GetTextArea().GetAreaBottom() ){
			if( ptMouse.x >= GetTextArea().GetAreaLeft() && ptMouse.x < GetTextArea().GetAreaRight() ){
				GetCaret().MoveCursorToClientPoint( ptMouse );
			}
			else if( ptMouse.x < GetTextArea().GetAreaLeft() ){
				GetCaret().MoveCursorToClientPoint( CMyPoint(GetTextArea().GetDocumentLeftClientPointX(), ptMouse.y) );
			}else{
				return;
			}
		}
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse;	// マウス範囲選択前回位置(XY座標)
		/*
		m_nMouseRollPosXOld = xPos;		// マウス範囲選択前回位置(X座標)
		m_nMouseRollPosYOld = yPos;		// マウス範囲選択前回位置(Y座標)
		*/

		/* 範囲選択開始 & マウスキャプチャー */
		GetSelectionInfo().SelectBeginBox();

		::SetCapture( GetHwnd() );
		GetCaret().HideCaret_( GetHwnd() ); // 2002/07/22 novice
		/* 現在のカーソル位置から選択を開始する */
		GetSelectionInfo().BeginSelectArea( );
		GetCaret().m_cUnderLine.CaretUnderLineOFF( TRUE );
		GetCaret().m_cUnderLine.Lock();
		if( ptMouse.x < GetTextArea().GetAreaLeft() ){
			/* カーソル下移動 */
			GetCommander().Command_DOWN( TRUE, FALSE );
		}
	}
	else{
		/* カーソル移動 */
		if( ptMouse.y >= GetTextArea().GetAreaTop() && ptMouse.y < GetTextArea().GetAreaBottom() ){
			if( ptMouse.x >= GetTextArea().GetAreaLeft() && ptMouse.x < GetTextArea().GetAreaRight() ){
			}
			else if( ptMouse.x < GetTextArea().GetAreaLeft() ){
			}
			else{
				return;
			}
		}
		else if( ptMouse.y < GetTextArea().GetAreaTop() ){
			//	ルーラクリック
			return;
		}
		else {
			return;
		}

		/* マウスのキャプチャなど */
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse;	// マウス範囲選択前回位置(XY座標)
		
		/* 範囲選択開始 & マウスキャプチャー */
		GetSelectionInfo().SelectBeginNazo();
		::SetCapture( GetHwnd() );
		GetCaret().HideCaret_( GetHwnd() ); // 2002/07/22 novice


		if(tripleClickMode){		// 2007.11.15 nasukoji	トリプルクリックを処理する
			// 1行選択でない場合は選択文字列を解除
			// トリプルクリックが1行選択でなくてもクアドラプルクリックを有効とする
			if(F_SELECTLINE != nFuncID){
				OnLBUTTONUP( fwKeys, ptMouse.x, ptMouse.y );	// ここで左ボタンアップしたことにする

				if( GetSelectionInfo().IsTextSelected() )		// テキストが選択されているか
					GetSelectionInfo().DisableSelectArea( TRUE );	// 現在の選択範囲を非選択状態に戻す
			}

			// 単語の途中で折り返されていると下の行が選択されてしまうことへの対処
			GetCaret().MoveCursorToClientPoint( ptMouse );	// カーソル移動

			// コマンドコードによる処理振り分け
			// マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
			::SendMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ), (LPARAM)NULL );

			// 1行選択でない場合はここで抜ける（他の選択コマンドの時問題となるかも）
			if(F_SELECTLINE != nFuncID)
				return;

			// 選択するものが無い（[EOF]のみの行）時は通常クリックと同じ処理
			if(( ! GetSelectionInfo().IsTextSelected() )&&
			   ( GetCaret().GetCaretLogicPos().y >= m_pcEditDoc->m_cDocLineMgr.GetLineCount() ))
			{
				GetSelectionInfo().BeginSelectArea();				// 現在のカーソル位置から選択を開始する
				GetSelectionInfo().m_bBeginLineSelect = FALSE;		// 行単位選択中 OFF
			}
		}else
		/* 選択開始処理 */
		/* SHIFTキーが押されていたか */
		if(GetKeyState_Shift()){
			if( GetSelectionInfo().IsTextSelected() ){		/* テキストが選択されているか */
				if( GetSelectionInfo().IsBoxSelecting() ){	/* 矩形範囲選択中 */
					/* 現在の選択範囲を非選択状態に戻す */
					GetSelectionInfo().DisableSelectArea( TRUE );

					/* 現在のカーソル位置から選択を開始する */
					GetSelectionInfo().BeginSelectArea( );
				}
				else{
				}
			}
			else{
				/* 現在のカーソル位置から選択を開始する */
				GetSelectionInfo().BeginSelectArea( );
			}

			/* カーソル移動 */
			if( ptMouse.y >= GetTextArea().GetAreaTop() && ptMouse.y < GetTextArea().GetAreaBottom() ){
				if( ptMouse.x >= GetTextArea().GetAreaLeft() && ptMouse.x < GetTextArea().GetAreaRight() ){
					GetCaret().MoveCursorToClientPoint( ptMouse );
				}
				else if( ptMouse.x < GetTextArea().GetAreaLeft() ){
					GetCaret().MoveCursorToClientPoint( CMyPoint(GetTextArea().GetDocumentLeftClientPointX(), ptMouse.y) );
				}
			}
		}
		else{
			if( GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
				/* 現在の選択範囲を非選択状態に戻す */
				GetSelectionInfo().DisableSelectArea( TRUE );
			}
			/* カーソル移動 */
			if( ptMouse.y >= GetTextArea().GetAreaTop() && ptMouse.y < GetTextArea().GetAreaBottom() ){
				if( ptMouse.x >= GetTextArea().GetAreaLeft() && ptMouse.x < GetTextArea().GetAreaRight() ){
					GetCaret().MoveCursorToClientPoint( ptMouse );
				}
				else if( ptMouse.x < GetTextArea().GetAreaLeft() ){
					GetCaret().MoveCursorToClientPoint( CMyPoint(GetTextArea().GetDocumentLeftClientPointX(), ptMouse.y) );
				}
			}
			/* 現在のカーソル位置から選択を開始する */
			GetSelectionInfo().BeginSelectArea( );
		}


		/******* この時点で必ず true == GetSelectionInfo().IsTextSelected() の状態になる ****:*/
		if( !GetSelectionInfo().IsTextSelected() ){
			WarningMessage( GetHwnd(), _T("バグってる") );
			return;
		}

		int	nWorkRel;
		nWorkRel = IsCurrentPositionSelected(
			GetCaret().GetCaretLayoutPos()	// カーソル位置
		);


		/* 現在のカーソル位置によって選択範囲を変更 */
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );


		// CTRLキーが押されている、かつトリプルクリックでない		// 2007.11.15 nasukoji	トリプルクリック対応
		if( GetKeyState_Control() &&( ! tripleClickMode)){
			GetSelectionInfo().m_bBeginWordSelect = TRUE;		/* 単語単位選択中 */
			if( !GetSelectionInfo().IsTextSelected() ){
				/* 現在位置の単語選択 */
				if ( GetCommander().Command_SELECTWORD() ){
					GetSelectionInfo().m_sSelectBgn = GetSelectionInfo().m_sSelect;
				}
			}else{

				/* 選択領域描画 */
				GetSelectionInfo().DrawSelectArea();


				/* 指定された桁に対応する行のデータ内の位置を調べる */
				const CLayout* pcLayout;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr(
					GetSelectionInfo().m_sSelect.GetFrom().GetY2(),
					&nLineLen,
					&pcLayout
				);
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pcLayout, GetSelectionInfo().m_sSelect.GetFrom().GetX2() );
					/* 現在位置の単語の範囲を調べる */
					int nWhareResult = m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						GetSelectionInfo().m_sSelect.GetFrom().GetY2(),
						nIdx,
						&sRange,
						NULL,
						NULL
					);
					if( nWhareResult ){
						// 指定された行のデータ内の位置に対応する桁の位置を調べる。
						// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
						/*
						pLine            = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetFrom().GetY2(), &nLineLen, &pcLayout );
						sRange.SetFromX( LineIndexToColmn( pcLayout, sRange.GetFrom().x ) );
						pLine            = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetTo().GetY2(), &nLineLen, &pcLayout );
						sRange.SetToX( LineIndexToColmn( pcLayout, sRange.GetTo().x ) );
						*/

						nWork = IsCurrentPositionSelected(
							sRange.GetFrom()	// カーソル位置
						);
						if( -1 == nWork || 0 == nWork ){
							GetSelectionInfo().m_sSelect.SetFrom(sRange.GetFrom());
							if( 1 == nWorkRel ){
								GetSelectionInfo().m_sSelectBgn = sRange;
							}
						}
					}
				}
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetSelectionInfo().m_sSelect.GetTo().GetY2(), &nLineLen, &pcLayout );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pcLayout, GetSelectionInfo().m_sSelect.GetTo().GetX2() );
					/* 現在位置の単語の範囲を調べる */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						GetSelectionInfo().m_sSelect.GetTo().GetY2(), nIdx, &sRange, NULL, NULL )
					){
						// 指定された行のデータ内の位置に対応する桁の位置を調べる
						// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
						/*
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetFrom().GetY2(), &nLineLen, &pcLayout );
						sRange.SetFromX( LineIndexToColmn( pcLayout, sRange.GetFrom().x ) );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetTo().GetY2(), &nLineLen, &pcLayout );
						sRange.SetToX( LineIndexToColmn( pcLayout, sRange.GetTo().x ) );
						*/

						nWork = IsCurrentPositionSelected(sRange.GetFrom());
						if( -1 == nWork || 0 == nWork ){
							GetSelectionInfo().m_sSelect.SetTo(sRange.GetFrom());
						}
						if( 1 == IsCurrentPositionSelected(sRange.GetTo()) ){
							GetSelectionInfo().m_sSelect.SetTo(sRange.GetTo());
						}
						if( -1 == nWorkRel || 0 == nWorkRel ){
							GetSelectionInfo().m_sSelectBgn=sRange;
						}
					}
				}

				if( 0 < nWorkRel ){

				}
				/* 選択領域描画 */
				GetSelectionInfo().DrawSelectArea();
			}
		}
		if( ptMouse.x < GetTextArea().GetAreaLeft() ){
			/* 現在のカーソル位置から選択を開始する */
			GetSelectionInfo().m_bBeginLineSelect = TRUE;

			// 2002.10.07 YAZAKI 折り返し行をインデントしているときに選択がおかしいバグの対策
			GetCommander().Command_GOLINEEND( TRUE, FALSE );
			GetCommander().Command_RIGHT( true, false, false );

			//	Apr. 14, 2003 genta
			//	行番号の下をクリックしてドラッグを開始するとおかしくなるのを修正
			//	行番号をクリックした場合にはGetSelectionInfo().ChangeSelectAreaByCurrentCursor()にて
			//	GetSelectionInfo().m_sSelect.GetTo().x/GetSelectionInfo().m_sSelect.GetTo().yに-1が設定されるが、上の
			//	GetCommander().Command_GOLINEEND(), Command_RIGHT()によって行選択が行われる。
			//	しかしキャレットが末尾にある場合にはキャレットが移動しないので
			//	GetSelectionInfo().m_sSelect.GetTo().x/GetSelectionInfo().m_sSelect.GetTo().yが-1のまま残ってしまい、それが
			//	原点に設定されるためにおかしくなっていた。
			//	なので、範囲選択が行われていない場合は起点末尾の設定を行わないようにする
			if( GetSelectionInfo().IsTextSelected() ){
				GetSelectionInfo().m_sSelectBgn.SetTo( GetSelectionInfo().m_sSelect.GetTo() );
			}
		}
		else{
			/* URLがクリックされたら選択するか */
			//	Sep. 7, 2003 genta URLの強調表示OFFの時はURLは普通の文字として扱う
			if( CTypeSupport(this,COLORIDX_URL).IsDisp() &&
				TRUE == m_pShareData->m_Common.m_sEdit.m_bSelectClickedURL ){

				CLogicRange cUrlRange;	//URL範囲
				// カーソル位置にURLが有る場合のその範囲を調べる
				bool bIsUrl = IsCurrentPositionURL(
					GetCaret().GetCaretLayoutPos(),	// カーソル位置
					&cUrlRange,						// URL範囲
					NULL							// URL受け取り先
				);
				if( bIsUrl ){
					/* 現在の選択範囲を非選択状態に戻す */
					GetSelectionInfo().DisableSelectArea( TRUE );

					/*
					  カーソル位置変換
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					  →レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						2002/04/08 YAZAKI 少しでもわかりやすく。
					*/
					CLayoutRange sRangeB;
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( cUrlRange, &sRangeB );
					/*
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( CLogicPoint(nUrlIdxBgn          , nUrlLine), sRangeB.GetFromPointer() );
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( CLogicPoint(nUrlIdxBgn + nUrlLen, nUrlLine), sRangeB.GetToPointer() );
					*/

					GetSelectionInfo().m_sSelectBgn = sRangeB;
					GetSelectionInfo().m_sSelect = sRangeB;

					/* 選択領域描画 */
					GetSelectionInfo().DrawSelectArea();
				}
			}
		}
	}
}

/*!
	指定カーソル位置にURLが有る場合のその範囲を調べる

	2007.01.18 kobake URL文字列の受け取りをwstringで行うように変更
*/
bool CEditView::IsCurrentPositionURL(
	const CLayoutPoint&	ptCaretPos,		//!< [in]  カーソル位置
	CLogicRange*		pUrlRange,		//!< [out] URL範囲。ロジック単位。
	std::wstring*		pwstrURL		//!< [out] URL文字列受け取り先。NULLを指定した場合はURL文字列を受け取らない。
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::IsCurrentPositionURL" );


	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	CLogicPoint ptXY;
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		ptCaretPos,
		&ptXY
	);
	pUrlRange->SetLine(ptXY.GetY2());
//	*pnUrlLine = ptXY.GetY2();
	CLogicInt		nLineLen;
	const wchar_t*	pLine = m_pcEditDoc->m_cDocLineMgr.GetLine(ptXY.GetY2())->GetDocLineStrWithEOL(&nLineLen); //2007.10.09 kobake レイアウト・ロジック混在バグ修正

	int			nUrlLen;
	CLogicInt	i = ptXY.GetX2() - CLogicInt(200);
	if( i < CLogicInt(0) ){
		i = CLogicInt(0);
	}
	for( ; i <= ptXY.GetX2() && i < nLineLen && i < ptXY.GetX2() + CLogicInt(200); ){
	/* カーソル位置から前方に250バイトまでの範囲内で行頭に向かってサーチ */
		/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
		if( !IsURL( &pLine[i], (Int)(nLineLen - i), &nUrlLen ) ){
			++i;
		}
		else{
			if( i <= ptXY.GetX2() && ptXY.GetX2() < i + CLogicInt(nUrlLen) ){
				/* URLを返す場合 */
				if( pwstrURL ){
					pwstrURL->assign(&pLine[i],nUrlLen);
				}
				pUrlRange->SetLine(ptXY.GetY2());
				pUrlRange->SetXs(i, i+CLogicInt(nUrlLen));
				return true;
			}else{
				i += CLogicInt(nUrlLen);
			}
		}
	}
	return false;
}


/*!	トリプルクリックのチェック
	@brief トリプルクリックを判定する
	
	2回目のクリックから3回目のクリックまでの時間がダブルクリック時間以内で、
	かつその時のクリック位置のずれがシステムメトリック（X:SM_CXDOUBLECLK,
	Y:SM_CYDOUBLECLK）の値（ピクセル）以下の時トリプルクリックとする。
	
	@param[in] xPos		マウスクリックX座標
	@param[in] yPos		マウスクリックY座標
	@return		トリプルクリックの時はTRUEを返す
	トリプルクリックでない時はFALSEを返す

	@note	m_dwTripleClickCheckが0でない時にチェックモードと判定するが、PCを
			連続稼動している場合49.7日毎にカウンタが0になる為、わずかな可能性
			であるがトリプルクリックが判定できない時がある。
			行番号表示エリアのトリプルクリックは通常クリックとして扱う。
	
	@date 2007.11.15 nasukoji	新規作成
*/
BOOL CEditView::CheckTripleClick( CMyPoint ptMouse )
{

	// トリプルクリックチェック有効でない（時刻がセットされていない）
	if(! m_dwTripleClickCheck)
		return FALSE;

	BOOL result = FALSE;

	// 前回クリックとのクリック位置のずれを算出
	CMyPoint dpos( GetSelectionInfo().m_ptMouseRollPosOld.x - ptMouse.x,
				   GetSelectionInfo().m_ptMouseRollPosOld.y - ptMouse.y );

	if(dpos.x < 0)
		dpos.x = -dpos.x;	// 絶対値化

	if(dpos.y < 0)
		dpos.y = -dpos.y;	// 絶対値化

	// 行番号表示エリアでない、かつクリックプレスからダブルクリック時間以内、
	// かつダブルクリックの許容ずれピクセル以下のずれの時トリプルクリックとする
	//	2007.10.12 genta/dskoba システムのダブルクリック速度，ずれ許容量を取得
	if( (ptMouse.x >= GetTextArea().GetAreaLeft())&&
		(::GetTickCount() - m_dwTripleClickCheck <= GetDoubleClickTime() )&&
		(dpos.x <= GetSystemMetrics(SM_CXDOUBLECLK) ) &&
		(dpos.y <= GetSystemMetrics(SM_CYDOUBLECLK)))
	{
		result = TRUE;
	}else{
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
	}
	
	return result;
}


/* マウス右ボタン押下 */
void CEditView::OnRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	/* 現在のマウスカーソル位置→レイアウト位置 */

	CLayoutPoint ptNew;
	GetTextArea().ClientToLayout(CMyPoint(xPos,yPos), &ptNew);
	/*
	ptNew.x = GetTextArea().GetViewLeftCol() + (xPos - GetTextArea().GetAreaLeft()) / GetTextMetrics().GetHankakuDx();
	ptNew.y = GetTextArea().GetViewTopLine() + (yPos - GetTextArea().GetAreaTop()) / GetTextMetrics().GetHankakuDy();
	*/
	/* 指定カーソル位置が選択エリア内にあるか */
	if( 0 == IsCurrentPositionSelected(
		ptNew		// カーソル位置
		)
	){
		return;
	}
	OnLBUTTONDOWN( fwKeys, xPos , yPos );
	return;
}

/* マウス右ボタン開放 */
void CEditView::OnRBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	if( GetSelectionInfo().IsMouseSelecting() ){	/* 範囲選択中 */
		/* マウス左ボタン開放のメッセージ処理 */
		OnLBUTTONUP( fwKeys, xPos, yPos );
	}


	int		nIdx;
	int		nFuncID;
// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス右クリックに対応する機能コードはm_Common.m_pKeyNameArr[1]に入っている */
	nFuncID = m_pShareData->m_pKeyNameArr[MOUSEFUNCTION_RIGHT].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}
//	/* 右クリックメニュー */
//	GetCommander().Command_MENU_RBUTTON();
	return;
}


// novice 2004/10/11 マウス中ボタン対応
/*!
	マウス中ボタンを押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.11 novice 新規作成
*/
void CEditView::OnMBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス左サイドボタンに対応する機能コードはm_Common.m_pKeyNameArr[2]に入っている */
	nFuncID = m_pShareData->m_pKeyNameArr[MOUSEFUNCTION_CENTER].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}
}


// novice 2004/10/10 マウスサイドボタン対応
/*!
	マウス左サイドボタンを押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.10 novice 新規作成
	@date 2004.10.11 novice マウス中ボタン対応のため変更
*/
void CEditView::OnXLBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス左サイドボタンに対応する機能コードはm_Common.m_pKeyNameArr[3]に入っている */
	nFuncID = m_pShareData->m_pKeyNameArr[MOUSEFUNCTION_LEFTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	return;
}


/*!
	マウス右サイドボタン押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.10 novice 新規作成
	@date 2004.10.11 novice マウス中ボタン対応のため変更
*/
void CEditView::OnXRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス右サイドボタンに対応する機能コードはm_Common.m_pKeyNameArr[4]に入っている */
	nFuncID = m_pShareData->m_pKeyNameArr[MOUSEFUNCTION_RIGHTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	return;
}


VOID CEditView::OnTimer(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT idEvent,	// timer identifier
	DWORD dwTime 	// current system time
	)
{
	POINT		po;
	RECT		rc;

	if( m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop ){	/* OLEによるドラッグ & ドロップを使う */
		if( m_bDragSource ){
			return;
		}
	}
	/* 範囲選択中でない場合 */
	if(!GetSelectionInfo().IsMouseSelecting()){
		if(TRUE == KeyWordHelpSearchDict( LID_SKH_ONTIMER, &po, &rc ) ){	// 2006.04.10 fon
			/* 辞書Tipを表示 */
			m_cTipWnd.Show( po.x, po.y + GetTextMetrics().GetHankakuHeight(), NULL );
		}
	}else{
		::GetCursorPos( &po );
		::GetWindowRect(GetHwnd(), &rc );
		if( !PtInRect( &rc, po ) ){
			OnMOUSEMOVE( 0, GetSelectionInfo().m_ptMouseRollPosOld.x, GetSelectionInfo().m_ptMouseRollPosOld.y );
		}
	}
}

/*! キーワード辞書検索の前提条件チェックと、検索

	@date 2006.04.10 fon OnTimer, CreatePopUpMenu_Rから分離
*/
BOOL CEditView::KeyWordHelpSearchDict( LID_SKH nID, POINT* po, RECT* rc )
{
	CNativeW	cmemCurText;
	int			i;

	/* キーワードヘルプを使用するか？ */
	if( !m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyWordHelp )	/* キーワードヘルプ機能を使用する */	// 2006.04.10 fon
		goto end_of_search;
	/* フォーカスがあるか？ */
	if( !GetCaret().ExistCaretFocus() ) 
		goto end_of_search;
	/* ウィンドウ内にマウスカーソルがあるか？ */
	GetCursorPos( po );
	GetWindowRect( GetHwnd(), rc );
	if( !PtInRect( rc, *po ) )
		goto end_of_search;
	switch(nID){
	case LID_SKH_ONTIMER:
		/* 右コメントの１～３でない場合 */
		if(!( m_bInMenuLoop == FALSE	&&			/* １．メニュー モーダル ループに入っていない */
			0 != m_dwTipTimer			&&			/* ２．辞書Tipを表示していない */
			300 < ::GetTickCount() - m_dwTipTimer	/* ３．一定時間以上、マウスが固定されている */
		) )	goto end_of_search;
		break;
	case LID_SKH_POPUPMENU_R:
		if(!( m_bInMenuLoop == FALSE	//&&			/* １．メニュー モーダル ループに入っていない */
		//	0 != m_dwTipTimer			&&			/* ２．辞書Tipを表示していない */
		//	1000 < ::GetTickCount() - m_dwTipTimer	/* ３．一定時間以上、マウスが固定されている */
		) )	goto end_of_search;
		break;
	default:
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, _T("作者に教えて欲しいエラー"),
		_T("CEditView::KeyWordHelpSearchDict\nnID=%d") );
	}
	/* 選択範囲のデータを取得(複数行選択の場合は先頭の行のみ) */
	if( GetSelectedData( &cmemCurText, TRUE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		wchar_t* pszWork = cmemCurText.GetStringPtr();
		int nWorkLength	= wcslen( pszWork );
		for( i = 0; i < nWorkLength; ++i ){
			if( pszWork[i] == L'\0' ||
				pszWork[i] == WCODE::CR ||
				pszWork[i] == WCODE::LF ){
				break;
			}
		}
		wchar_t* pszBuf = new wchar_t[i + 1];
		wmemcpy( pszBuf, pszWork, i );
		pszBuf[i] = L'\0';
		cmemCurText.SetString( pszBuf, i );
		delete [] pszBuf;
	}/* キャレット位置の単語を取得する処理 */	// 2006.03.24 fon
	else if(m_pShareData->m_Common.m_sSearch.m_bUseCaretKeyWord){
		if(!GetParser().GetCurrentWord(&cmemCurText))
			goto end_of_search;
	}else
		goto end_of_search;

	if( CNativeW::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) &&	/* 既に検索済みか */
		(!m_cTipWnd.m_KeyWasHit) )								/* 該当するキーがなかった */
		goto end_of_search;
	m_cTipWnd.m_cKey = cmemCurText;

	/* 検索実行 */
	if( FALSE == KeySearchCore(&m_cTipWnd.m_cKey) )
		goto end_of_search;
	m_dwTipTimer = 0;		/* 辞書Tipを表示している */
	m_poTipCurPos = *po;	/* 現在のマウスカーソル位置 */
	return TRUE;			/* ここまで来ていればヒット・ワード */

	/* キーワードヘルプ表示処理終了 */
	end_of_search:
	return FALSE;
}

/*! キーワード辞書検索処理メイン

	@date 2006.04.10 fon KeyWordHelpSearchDictから分離
*/
BOOL CEditView::KeySearchCore( const CNativeW* pcmemCurText )
{
	CNativeW*	pcmemRefKey;
	int			nCmpLen = STRNCMP_MAX; // 2006.04.10 fon
	int			nLine; // 2006.04.10 fon


	CDocumentType nTypeNo = m_pcEditDoc->m_cDocType.GetDocumentType();
	m_cTipWnd.m_cInfo.SetString( _T("") );	/* tooltipバッファ初期化 */
	/* 1行目にキーワード表示の場合 */
	if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpKeyDisp){	/* キーワードも表示する */	// 2006.04.10 fon
		m_cTipWnd.m_cInfo.AppendString( _T("[ ") );
		m_cTipWnd.m_cInfo.AppendString( pcmemCurText->GetStringT() );
		m_cTipWnd.m_cInfo.AppendString( _T(" ]") );
	}
	/* 途中まで一致を使う場合 */
	if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpPrefix)
		nCmpLen = wcslen( pcmemCurText->GetStringPtr() );	// 2006.04.10 fon
	m_cTipWnd.m_KeyWasHit = FALSE;
	for(int i=0;i<m_pShareData->GetTypeSetting(nTypeNo).m_nKeyHelpNum;i++){	//最大数：MAX_KEYHELP_FILE
		if( 1 == m_pShareData->GetTypeSetting(nTypeNo).m_KeyHelpArr[i].m_nUse ){
			// 2006.04.10 fon (nCmpLen,pcmemRefKey,nSearchLine)引数を追加
			CNativeW*	pcmemRefText;
			int nSearchResult=m_cDicMgr.CDicMgr::Search(
				pcmemCurText->GetStringPtr(),
				nCmpLen,
				&pcmemRefKey,
				&pcmemRefText,
				m_pShareData->GetTypeSetting(nTypeNo).m_KeyHelpArr[i].m_szPath,
				&nLine
			);
			if(nSearchResult){
				/* 該当するキーがある */
				LPWSTR		pszWork;
				pszWork = pcmemRefText->GetStringPtr();
				/* 有効になっている辞書を全部なめて、ヒットの都度説明の継ぎ増し */
				if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpAllSearch){	/* ヒットした次の辞書も検索 */	// 2006.04.10 fon
					/* バッファに前のデータが詰まっていたらseparator挿入 */
					if(m_cTipWnd.m_cInfo.GetStringLength() != 0)
						m_cTipWnd.m_cInfo.AppendString( _T("\n--------------------\n■") );
					else
						m_cTipWnd.m_cInfo.AppendString( _T("■") );	/* 先頭の場合 */
					/* 辞書のパス挿入 */
					m_cTipWnd.m_cInfo.AppendString( m_pShareData->GetTypeSetting(nTypeNo).m_KeyHelpArr[i].m_szPath );
					m_cTipWnd.m_cInfo.AppendString( _T("\n") );
					/* 前方一致でヒットした単語を挿入 */
					if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpPrefix){	/* 選択範囲で前方一致検索 */
						m_cTipWnd.m_cInfo.AppendString( pcmemRefKey->GetStringT() );
						m_cTipWnd.m_cInfo.AppendString( _T(" >>\n") );
					}/* 調査した「意味」を挿入 */
					m_cTipWnd.m_cInfo.AppendStringW( pszWork );
					delete pcmemRefText;
					delete pcmemRefKey;	// 2006.07.02 genta
					/* タグジャンプ用の情報を残す */
					if(FALSE == m_cTipWnd.m_KeyWasHit){
						m_cTipWnd.m_nSearchDict=i;	/* 辞書を開くとき最初にヒットした辞書を開く */
						m_cTipWnd.m_nSearchLine=nLine;
						m_cTipWnd.m_KeyWasHit = TRUE;
					}
				}
				else{	/* 最初のヒット項目のみ返す場合 */
					/* キーワードが入っていたらseparator挿入 */
					if(m_cTipWnd.m_cInfo.GetStringLength() != 0)
						m_cTipWnd.m_cInfo.AppendString( _T("\n--------------------\n") );
					
					/* 前方一致でヒットした単語を挿入 */
					if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpPrefix){	/* 選択範囲で前方一致検索 */
						m_cTipWnd.m_cInfo.AppendString( pcmemRefKey->GetStringT() );
						m_cTipWnd.m_cInfo.AppendString( _T(" >>\n") );
					}
					
					/* 調査した「意味」を挿入 */
					m_cTipWnd.m_cInfo.AppendStringW( pszWork );
					delete pcmemRefText;
					delete pcmemRefKey;	// 2006.07.02 genta
					/* タグジャンプ用の情報を残す */
					m_cTipWnd.m_nSearchDict=i;
					m_cTipWnd.m_nSearchLine=nLine;
					m_cTipWnd.m_KeyWasHit = TRUE;
					return TRUE;
				}
			}
		}
	}
	if(m_cTipWnd.m_KeyWasHit == TRUE){
			return TRUE;
	}
	/* 該当するキーがなかった場合 */
	return FALSE;
}


/* マウス移動のメッセージ処理 */
void CEditView::OnMOUSEMOVE( WPARAM fwKeys, int _xPos , int _yPos )
{
	CMyPoint ptMouse(_xPos,_yPos);

	CLayoutInt	nScrollRowNum;
	POINT		po;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;

	CLogicInt	nIdx;
	int			nWorkF;
	int			nWorkT;

	CLayoutRange sRange;
	CLayoutRange sSelectBgn_Old; // 範囲選択(原点)
	CLayoutRange sSelect_Old;
	CLayoutRange sSelect;

	sSelectBgn_Old = GetSelectionInfo().m_sSelectBgn;
	sSelect_Old    = GetSelectionInfo().m_sSelect;

	if( !GetSelectionInfo().IsMouseSelecting() ){	/* 範囲選択中 */
		::GetCursorPos( &po );
		//	2001/06/18 asa-o: 補完ウィンドウが表示されていない
		if(!m_bHokan){
			/* 辞書Tipが起動されている */
			if( 0 == m_dwTipTimer ){
				if( (m_poTipCurPos.x != po.x || m_poTipCurPos.y != po.y ) ){
					/* 辞書Tipを消す */
					m_cTipWnd.Hide();
					m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
				}
			}else{
				m_dwTipTimer = ::GetTickCount();		/* 辞書Tip起動タイマー */
			}
		}
		/* 現在のマウスカーソル位置→レイアウト位置 */
		CLayoutPoint ptNew;
		GetTextArea().ClientToLayout(ptMouse, &ptNew);

		CLogicRange	cUrlRange;	//URL範囲

		/* 選択テキストのドラッグ中か */
		if( m_bDragMode ){
			if( m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop ){	/* OLEによるドラッグ & ドロップを使う */
				/* 座標指定によるカーソル移動 */
				nScrollRowNum = GetCaret().MoveCursorToClientPoint( ptMouse );
			}
		}
		else{
			/* 行選択エリア? */
			if( ptMouse.x < GetTextArea().GetAreaLeft() || ptMouse.y < GetTextArea().GetAreaTop() ){	//	2002/2/10 aroka
				/* 矢印カーソル */
				if( ptMouse.y >= GetTextArea().GetAreaTop() )
					::SetCursor( ::LoadCursor( m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_RVARROW ) ) );
				else
					::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}
			else if( m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop	/* OLEによるドラッグ & ドロップを使う */
			 && m_pShareData->m_Common.m_sEdit.m_bUseOLE_DropSource /* OLEによるドラッグ元にするか */
			 && 0 == IsCurrentPositionSelected(						/* 指定カーソル位置が選択エリア内にあるか */
				ptNew	// カーソル位置
				)
			){
				/* 矢印カーソル */
				::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}
			/* カーソル位置にURLが有る場合 */
			//	Sep. 7, 2003 genta URLの強調表示OFFの時はURLチェックも行わない
			else if( CTypeSupport(this,COLORIDX_URL).IsDisp() &&
				IsCurrentPositionURL(
					ptNew,			// カーソル位置
					&cUrlRange,		// URL範囲
					NULL			// URL受け取り先
				)
			){
				/* 手カーソル */
				::SetCursor( ::LoadCursor( m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_HAND ) ) );
			}else{
				//migemo isearch 2004.10.22
				if( m_nISearchMode > 0 ){
					if (m_nISearchDirection == 1){
						::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
					}else{
						::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
					}
				}else
				/* アイビーム */
				::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
			}
		}
		return;
	}
	::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
	if( GetSelectionInfo().IsBoxSelecting() ){	/* 矩形範囲選択中 */
		/* 座標指定によるカーソル移動 */
		nScrollRowNum = GetCaret().MoveCursorToClientPoint( ptMouse );
		/* 現在のカーソル位置によって選択範囲を変更 */
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse; // マウス範囲選択前回位置(XY座標)
	}
	else{
		/* 座標指定によるカーソル移動 */
		if(( ptMouse.x < GetTextArea().GetAreaLeft() || m_dwTripleClickCheck )&& GetSelectionInfo().m_bBeginLineSelect ){	// 行単位選択中
			// 2007.11.15 nasukoji	上方向の行選択時もマウスカーソルの位置の行が選択されるようにする
			CMyPoint nNewPos(0, ptMouse.y);

			// 1行の高さ
			int nLineHeight = GetTextMetrics().GetHankakuHeight() + m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nLineSpace;

			// 選択開始行以下へのドラッグ時は1行下にカーソルを移動する
			if( GetTextArea().GetViewTopLine() + (ptMouse.y - GetTextArea().GetAreaTop()) / nLineHeight >= GetSelectionInfo().m_sSelectBgn.GetTo().y)
				nNewPos.y += nLineHeight;

			// カーソルを移動
			nNewPos.x = GetTextArea().GetAreaLeft() - Int(GetTextArea().GetViewLeftCol()) * ( GetTextMetrics().GetHankakuWidth() + m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nColmSpace );
			nScrollRowNum = GetCaret().MoveCursorToClientPoint( nNewPos );

			// 2.5クリックによる行単位のドラッグ
			if( m_dwTripleClickCheck ){
				// 選択開始行以上にドラッグした
				if( GetCaret().GetCaretLayoutPos().GetY() <= GetSelectionInfo().m_sSelectBgn.GetTo().y ){
					GetCommander().Command_GOLINETOP( TRUE, 0x09 );		// 改行単位の行頭へ移動
				}else{
					CLayoutPoint ptCaret;

					CLogicPoint ptCaretPrevLog(0, GetCaret().GetCaretLogicPos().y);

					// 選択開始行より下にカーソルがある時は1行前と物理行番号の違いをチェックする
					// 選択開始行にカーソルがある時はチェック不要
					if( GetCaret().GetCaretLayoutPos().GetY() > GetSelectionInfo().m_sSelectBgn.GetTo().y ){
						// 1行前の物理行を取得する
						m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( CLayoutPoint(CLayoutInt(0), GetCaret().GetCaretLayoutPos().GetY() - 1), &ptCaretPrevLog );
					}

					// 前の行と同じ物理行
					if( ptCaretPrevLog.y == GetCaret().GetCaretLogicPos().y ){
						// 1行先の物理行からレイアウト行を求める
						m_pcEditDoc->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, GetCaret().GetCaretLogicPos().y + 1), &ptCaret );

						// カーソルを次の物理行頭へ移動する
						nScrollRowNum = GetCaret().MoveCursor( ptCaret, TRUE );
					}
				}
			}
		}else{
			nScrollRowNum = GetCaret().MoveCursorToClientPoint( ptMouse );
		}
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse; // マウス範囲選択前回位置(XY座標)

		/* CTRLキーが押されていたか */
//		if( GetKeyState_Control() ){
		if( !GetSelectionInfo().m_bBeginWordSelect ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
		}else{
			/* 現在のカーソル位置によって選択範囲を変更(テストのみ) */
			GetSelectionInfo().ChangeSelectAreaByCurrentCursorTEST(
				GetCaret().GetCaretLayoutPos(),
				&sSelect
			);
			/* 選択範囲に変更なし */
			if( sSelect_Old == sSelect ){
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor(
					GetCaret().GetCaretLayoutPos()
				);
				return;
			}
			const CLayout* pcLayout;
			if( NULL != ( pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout ) ) ){
				nIdx = LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );
				/* 現在位置の単語の範囲を調べる */
				int nResult=m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
					GetCaret().GetCaretLayoutPos().GetY2(),
					nIdx,
					&sRange,
					NULL,
					NULL
				);
				if( nResult ){
					// 指定された行のデータ内の位置に対応する桁の位置を調べる
					// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
					/*
					pLine     = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetFrom().GetY2(), &nLineLen, &pcLayout );
					sRange.SetFromX( LineIndexToColmn( pcLayout, sRange.GetFrom().x ) );
					pLine     = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetTo().GetY2(), &nLineLen, &pcLayout );
					sRange.SetToX( LineIndexToColmn( pcLayout, sRange.GetTo().x ) );
					*/

					nWorkF = IsCurrentPositionSelectedTEST(
						sRange.GetFrom(), //カーソル位置
						sSelect
					);
					nWorkT = IsCurrentPositionSelectedTEST(
						sRange.GetTo(),	// カーソル位置
						sSelect
					);
					if( -1 == nWorkF ){
						/* 始点が前方に移動。現在のカーソル位置によって選択範囲を変更 */
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRange.GetFrom() );
					}
					else if( 1 == nWorkT ){
						/* 終点が後方に移動。現在のカーソル位置によって選択範囲を変更 */
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRange.GetTo() );
					}
					else if( sSelect_Old.GetFrom() == sSelect.GetFrom() ){
						/* 始点が無変更＝前方に縮小された */
						/* 現在のカーソル位置によって選択範囲を変更 */
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRange.GetTo() );
					}
					else if( sSelect_Old.GetTo()==sSelect.GetTo() ){
						/* 終点が無変更＝後方に縮小された */
						/* 現在のカーソル位置によって選択範囲を変更 */
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRange.GetFrom() );
					}
				}else{
					/* 現在のカーソル位置によって選択範囲を変更 */
					GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
				}
			}else{
				/* 現在のカーソル位置によって選択範囲を変更 */
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
			}
		}
	}
	return;
}
//m_dwTipTimerm_dwTipTimerm_dwTipTimer




/* マウスホイールのメッセージ処理 */
LRESULT CEditView::OnMOUSEWHEEL( WPARAM wParam, LPARAM lParam )
{
	WORD	fwKeys;
	short	zDelta;
	short	xPos;
	short	yPos;
	int		i;
	int		nScrollCode;
	int		nRollLineNum;

	fwKeys = LOWORD(wParam);			// key flags
	zDelta = (short) HIWORD(wParam);	// wheel rotation
	xPos = (short) LOWORD(lParam);		// horizontal position of pointer
	yPos = (short) HIWORD(lParam);		// vertical position of pointer
//	MYTRACE_A( "CEditView::DispatchEvent() WM_MOUSEWHEEL fwKeys=%xh zDelta=%d xPos=%d yPos=%d \n", fwKeys, zDelta, xPos, yPos );

	if( 0 < zDelta ){
		nScrollCode = SB_LINEUP;
	}else{
		nScrollCode = SB_LINEDOWN;
	}

	/* マウスホイールによるスクロール行数をレジストリから取得 */
	nRollLineNum = 6;

	/* レジストリの存在チェック */
	// 2006.06.03 Moca ReadRegistry に書き換え
	unsigned int uDataLen;	// size of value data
	TCHAR szValStr[256];
	uDataLen = _countof(szValStr) - 1;
	if( ReadRegistry( HKEY_CURRENT_USER, _T("Control Panel\\desktop"), _T("WheelScrollLines"), szValStr, uDataLen ) ){
		nRollLineNum = ::_ttoi( szValStr );
	}

	if( -1 == nRollLineNum ){/* 「1画面分スクロールする」 */
		nRollLineNum = (Int)GetTextArea().m_nViewRowNum;	// 表示域の行数
	}
	else{
		if( nRollLineNum < 1 ){
			nRollLineNum = 1;
		}
		if( nRollLineNum > 30 ){	//@@@ YAZAKI 2001.12.31 10→30へ。
			nRollLineNum = 30;
		}
	}
	for( i = 0; i < nRollLineNum; ++i ){
		//	Sep. 11, 2004 genta 同期スクロール行数
		CLayoutInt line;

		if( nScrollCode == SB_LINEUP ){
			line = ScrollAtV( GetTextArea().GetViewTopLine() - CLayoutInt(1) );
		}else{
			line = ScrollAtV( GetTextArea().GetViewTopLine() + CLayoutInt(1) );
		}
		SyncScrollV( line );
	}
	return 0;
}






/* マウス左ボタン開放のメッセージ処理 */
void CEditView::OnLBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
//	MYTRACE_A( "OnLBUTTONUP()\n" );
	CMemory		cmemBuf, cmemClip;

	/* 範囲選択終了 & マウスキャプチャーおわり */
	if( GetSelectionInfo().IsMouseSelecting() ){	/* 範囲選択中 */
		/* マウス キャプチャを解放 */
		::ReleaseCapture();
		GetCaret().ShowCaret_( GetHwnd() ); // 2002/07/22 novice

		GetSelectionInfo().SelectEnd();

		if( GetSelectionInfo().m_sSelect.IsOne() ){
			/* 現在の選択範囲を非選択状態に戻す */
			GetSelectionInfo().DisableSelectArea( TRUE );

			// 対括弧の強調表示	// 2007.10.18 ryoji
			DrawBracketPair( false );
			SetBracketPairPos( true );
			DrawBracketPair( true );
		}
		GetCaret().m_cUnderLine.UnLock();
	}
	return;
}





// マウス左ボタンダブルクリック
// 2007.01.18 kobake IsCurrentPositionURL仕様変更に伴い、処理の書き換え
void CEditView::OnLBUTTONDBLCLK( WPARAM fwKeys, int _xPos , int _yPos )
{
	CMyPoint ptMouse(_xPos,_yPos);

	CLogicRange		cUrlRange;	// URL範囲
	std::wstring	wstrURL;
	const wchar_t*	pszMailTo = L"mailto:";

	// 2007.10.06 nasukoji	クアドラプルクリック時はチェックしない
	if(! m_dwTripleClickCheck){
		/* カーソル位置にURLが有る場合のその範囲を調べる */
		//	Sep. 7, 2003 genta URLの強調表示OFFの時はURLチェックも行わない
		if( CTypeSupport(this,COLORIDX_URL).IsDisp()
			&&
			IsCurrentPositionURL(
				GetCaret().GetCaretLayoutPos(),	// カーソル位置
				&cUrlRange,				// URL範囲
				&wstrURL				// URL受け取り先
			)
		){
			std::wstring wstrOPEN;

			// URLを開く
		 	// 現在位置がメールアドレスならば、NULL以外と、その長さを返す
			if( IsMailAddress( wstrURL.c_str(), wstrURL.length(), NULL ) ){
				wstrOPEN = pszMailTo + wstrURL;
			}
			else{
				if( wcsnicmp( wstrURL.c_str(), L"ttp://", 6 ) == 0 ){	//抑止URL
					wstrOPEN = L"h" + wstrURL;
				}
				else if( wcsnicmp( wstrURL.c_str(), L"tp://", 5 ) == 0 ){	//抑止URL
					wstrOPEN = L"ht" + wstrURL;
				}
				else{
					wstrOPEN = wstrURL;
				}
			}
			::ShellExecute( NULL, _T("open"), to_tchar(wstrOPEN.c_str()), NULL, NULL, SW_SHOW );
			return;
		}

		/* GREP出力モードまたはデバッグモード かつ マウス左ボタンダブルクリックでタグジャンプ の場合 */
		//	2004.09.20 naoh 外部コマンドの出力からTagjumpできるように
		if( (CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode || CAppMode::Instance()->IsDebugMode()) && m_pShareData->m_Common.m_sSearch.m_bGTJW_LDBLCLK ){
			/* タグジャンプ機能 */
			GetCommander().Command_TAGJUMP();
			return;
		}
	}

// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	int	nIdx = getCtrlKeyState();

	/* マウス左クリックに対応する機能コードはm_Common.m_pKeyNameArr[?]に入っている 2007.11.15 nasukoji */
	EFunctionCode	nFuncID = m_pShareData->m_pKeyNameArr[
		m_dwTripleClickCheck ? MOUSEFUNCTION_QUADCLICK : MOUSEFUNCTION_DOUBLECLICK
		].m_nFuncCodeArr[nIdx];
	if(m_dwTripleClickCheck){
		// 非選択状態にした後左クリックしたことにする
		// すべて選択の場合は、3.5クリック時の選択状態保持とドラッグ開始時の
		// 範囲変更のため。
		// クアドラプルクリック機能が割り当てられていない場合は、ダブルクリック
		// として処理するため。
		if( GetSelectionInfo().IsTextSelected() )		// テキストが選択されているか
			GetSelectionInfo().DisableSelectArea( TRUE );		// 現在の選択範囲を非選択状態に戻す

		if(! nFuncID){
			m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
			nFuncID = m_pShareData->m_pKeyNameArr[MOUSEFUNCTION_DOUBLECLICK].m_nFuncCodeArr[nIdx];
			OnLBUTTONDOWN( fwKeys, ptMouse.x , ptMouse.y );	// カーソルをクリック位置へ移動する
		}
	}

	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::SendMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	// 2007.10.06 nasukoji	クアドラプルクリック時もここで抜ける
	if(m_dwTripleClickCheck){
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF（次回は通常クリック）
		return;
	}

	// 2007.11.06 nasukoji	ダブルクリックが単語選択でなくてもトリプルクリックを有効とする
	// 2007.10.02 nasukoji	トリプルクリックチェック用に時刻を取得
	m_dwTripleClickCheck = ::GetTickCount();

	// ダブルクリック位置として記憶
	GetSelectionInfo().m_ptMouseRollPosOld = ptMouse;	// マウス範囲選択前回位置(XY座標)

	/*	2007.07.09 maru 機能コードの判定を追加
		ダブルクリックからのドラッグでは単語単位の範囲選択(エディタの一般的動作)になるが
		この動作は、ダブルクリック＝単語選択を前提としたもの。
		キー割り当ての変更により、ダブルクリック≠単語選択のときには GetSelectionInfo().m_bBeginWordSelect = TRUE
		にすると、処理の内容によっては表示がおかしくなるので、ここで抜けるようにする。
	*/
	if(F_SELECTWORD != nFuncID) return;

	/* 範囲選択開始 & マウスキャプチャー */
	GetSelectionInfo().SelectBeginWord();

	if( m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH ){	/* 現在のフォントは固定幅フォントである */
		/* ALTキーが押されていたか */
		if( GetKeyState_Alt() ){
			GetSelectionInfo().SetBoxSelect(true);	/* 矩形範囲選択中 */
		}
	}
	::SetCapture( GetHwnd() );
	GetCaret().HideCaret_( GetHwnd() ); // 2002/07/22 novice
	if( GetSelectionInfo().IsTextSelected() ){
		/* 常時選択範囲の範囲 */
		GetSelectionInfo().m_sSelectBgn.SetTo( GetSelectionInfo().m_sSelect.GetTo() );
	}
	else{
		/* 現在のカーソル位置から選択を開始する */
		GetSelectionInfo().BeginSelectArea( );
	}

	return;
}









/*! 指定上端行位置へスクロール

	@param nPos [in] スクロール位置
	@retval 実際にスクロールした行数 (正:下方向/負:上方向)

	@date 2004.09.11 genta 行数を戻り値として返すように．(同期スクロール用)
*/
CLayoutInt CEditView::ScrollAtV( CLayoutInt nPos )
{
	CLayoutInt	nScrollRowNum;
	RECT		rcScrol;
	RECT		rcClip;
	if( nPos < 0 ){
		nPos = CLayoutInt(0);
	}
	else if( (m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 )- GetTextArea().m_nViewRowNum < nPos ){
		nPos = ( m_pcEditDoc->m_cLayoutMgr.GetLineCount() + CLayoutInt(2) ) - GetTextArea().m_nViewRowNum;
		if( nPos < 0 ){
			nPos = CLayoutInt(0);
		}
	}
	if( GetTextArea().GetViewTopLine() == nPos ){
		return CLayoutInt(0);	//	スクロール無し。
	}
	/* 垂直スクロール量（行数）の算出 */
	nScrollRowNum = GetTextArea().GetViewTopLine() - nPos;

	/* スクロール */
	if( t_abs( nScrollRowNum ) >= GetTextArea().m_nViewRowNum ){
		GetTextArea().SetViewTopLine( CLayoutInt(nPos) );
		::InvalidateRect( GetHwnd(), NULL, TRUE );
	}else{
		rcScrol.left = 0;
		rcScrol.right = GetTextArea().GetAreaRight();
		rcScrol.top = GetTextArea().GetAreaTop();
		rcScrol.bottom = GetTextArea().GetAreaBottom();
		if( nScrollRowNum > 0 ){
			rcScrol.bottom =
				GetTextArea().GetAreaBottom() -
				(Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
			GetTextArea().SetViewTopLine( CLayoutInt(nPos) );
			rcClip.left = 0;
			rcClip.right = GetTextArea().GetAreaRight();
			rcClip.top = GetTextArea().GetAreaTop();
			rcClip.bottom =
				GetTextArea().GetAreaTop() + (Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
		}
		else if( nScrollRowNum < 0 ){
			rcScrol.top =
				GetTextArea().GetAreaTop() - (Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
			GetTextArea().SetViewTopLine( CLayoutInt(nPos) );
			rcClip.left = 0;
			rcClip.right = GetTextArea().GetAreaRight();
			rcClip.top =
				GetTextArea().GetAreaBottom() +
				(Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
			rcClip.bottom = GetTextArea().GetAreaBottom();
		}
		if( GetDrawSwitch() ){
			::ScrollWindowEx(
				GetHwnd(),
				0,	/* 水平スクロール量 */
				(Int)nScrollRowNum * GetTextMetrics().GetHankakuDy(),		/* 垂直スクロール量 */
				&rcScrol,	/* スクロール長方形の構造体のアドレス */
				NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
			);
			::InvalidateRect( GetHwnd(), &rcClip, TRUE );
			::UpdateWindow( GetHwnd() );
		}
	}

	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	/* キャレットの表示・更新 */
	GetCaret().ShowEditCaret();

	return -nScrollRowNum;	//方向が逆なので符号反転が必要
}




/*! 指定左端桁位置へスクロール

	@param nPos [in] スクロール位置
	@retval 実際にスクロールした桁数 (正:右方向/負:左方向)

	@date 2004.09.11 genta 桁数を戻り値として返すように．(同期スクロール用)
*/
CLayoutInt CEditView::ScrollAtH( CLayoutInt nPos )
{
	CLayoutInt	nScrollColNum;
	RECT		rcScrol;
	RECT		rcClip2;
	if( nPos < 0 ){
		nPos = CLayoutInt(0);
	}else
	//	Aug. 18, 2003 ryoji 変数のミスを修正
	//	ウィンドウの幅をきわめて狭くしたときに編集領域が行番号から離れてしまうことがあった．
	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if( m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() - GetTextArea().m_nViewColNum  < nPos ){
		nPos = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() - GetTextArea().m_nViewColNum ;
		//	May 29, 2004 genta 折り返し幅よりウィンドウ幅が大きいときにWM_HSCROLLが来ると
		//	nPosが負の値になることがあり，その場合にスクロールバーから編集領域が
		//	離れてしまう．
		if( nPos < 0 )
			nPos = CLayoutInt(0);
	}
	if( GetTextArea().GetViewLeftCol() == nPos ){
		return CLayoutInt(0);
	}
	/* 水平スクロール量（文字数）の算出 */
	nScrollColNum = GetTextArea().GetViewLeftCol() - nPos;

	/* スクロール */
	if( t_abs( nScrollColNum ) >= GetTextArea().m_nViewColNum /*|| abs( nScrollRowNum ) >= GetTextArea().m_nViewRowNum*/ ){
		GetTextArea().SetViewLeftCol( nPos );
		::InvalidateRect( GetHwnd(), NULL, TRUE );
	}else{
		rcScrol.left = 0;
		rcScrol.right = GetTextArea().GetAreaRight();
		rcScrol.top = GetTextArea().GetAreaTop();
		rcScrol.bottom = GetTextArea().GetAreaBottom();
		if( nScrollColNum > 0 ){
			rcScrol.left = GetTextArea().GetAreaLeft();
			rcScrol.right =
				GetTextArea().GetAreaRight() - (Int)nScrollColNum * GetTextMetrics().GetHankakuDx();
			rcClip2.left = GetTextArea().GetAreaLeft();
			rcClip2.right = GetTextArea().GetAreaLeft() + (Int)nScrollColNum * GetTextMetrics().GetHankakuDx();
			rcClip2.top = GetTextArea().GetAreaTop();
			rcClip2.bottom = GetTextArea().GetAreaBottom();
		}
		else if( nScrollColNum < 0 ){
			rcScrol.left = GetTextArea().GetAreaLeft() - (Int)nScrollColNum * GetTextMetrics().GetHankakuDx();
			rcClip2.left =
				GetTextArea().GetAreaRight() + (Int)nScrollColNum * GetTextMetrics().GetHankakuDx();
			rcClip2.right = GetTextArea().GetAreaRight();
			rcClip2.top = GetTextArea().GetAreaTop();
			rcClip2.bottom = GetTextArea().GetAreaBottom();
		}
		GetTextArea().SetViewLeftCol( nPos );
		if( GetDrawSwitch() ){
			::ScrollWindowEx(
				GetHwnd(),
				(Int)nScrollColNum * GetTextMetrics().GetHankakuDx(),		/* 水平スクロール量 */
				0,	/* 垂直スクロール量 */
				&rcScrol,	/* スクロール長方形の構造体のアドレス */
				NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
			);
			::InvalidateRect( GetHwnd(), &rcClip2, TRUE );
			::UpdateWindow( GetHwnd() );
		}
	}
	//	2006.1.28 aroka 判定条件誤り修正 (バーが消えてもスクロールしない)
	// 先にAdjustScrollBarsを呼んでしまうと、二度目はここまでこないので、
	// GetRuler().DispRulerが呼ばれない。そのため、順序を入れ替えた。
	GetRuler().SetRedrawFlag(); // ルーラーを再描画する。
	HDC hdc = ::GetDC( GetHwnd() );
	GetRuler().DispRuler( hdc );
	::ReleaseDC( GetHwnd(), hdc );

	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	/* キャレットの表示・更新 */
	GetCaret().ShowEditCaret();

	return -nScrollColNum;	//方向が逆なので符号反転が必要
}

/*!	垂直同期スクロール

	垂直同期スクロールがONならば，対応するウィンドウを指定行数同期スクロールする
	
	@param line [in] スクロール行数 (正:下方向/負:上方向/0:何もしない)
	
	@author asa-o
	@date 2001.06.20 asa-o 新規作成
	@date 2004.09.11 genta 関数化

	@note 動作の詳細は設定や機能拡張により変更になる可能性がある

*/
void CEditView::SyncScrollV( CLayoutInt line )
{
	if( m_pShareData->m_Common.m_sWindow.m_bSplitterWndVScroll && line != 0 )
	{
		CEditView*	pcEditView = m_pcEditDoc->m_pcEditWnd->m_pcEditViewArr[m_nMyIndex^0x01];
#if 0
		//	差分を保ったままスクロールする場合
		pcEditView -> ScrollByV( line );
#else
		pcEditView -> ScrollAtV( GetTextArea().GetViewTopLine() );
#endif
	}
}

/*!	水平同期スクロール

	水平同期スクロールがONならば，対応するウィンドウを指定行数同期スクロールする．
	
	@param col [in] スクロール桁数 (正:右方向/負:左方向/0:何もしない)
	
	@author asa-o
	@date 2001.06.20 asa-o 新規作成
	@date 2004.09.11 genta 関数化

	@note 動作の詳細は設定や機能拡張により変更になる可能性がある
*/
void CEditView::SyncScrollH( CLayoutInt col )
{
	if( m_pShareData->m_Common.m_sWindow.m_bSplitterWndHScroll && col != 0 )
	{
		CEditView*	pcEditView = m_pcEditDoc->m_pcEditWnd->m_pcEditViewArr[m_nMyIndex^0x02];
		HDC			hdc = ::GetDC( pcEditView->GetHwnd() );
		
#if 0
		//	差分を保ったままスクロールする場合
		pcEditView -> ScrollByH( col );
#else
		pcEditView -> ScrollAtH( GetTextArea().GetViewLeftCol() );
#endif
		GetRuler().SetRedrawFlag(); //2002.02.25 Add By KK スクロール時ルーラー全体を描きなおす。
		GetRuler().DispRuler( hdc );
		::ReleaseDC( GetHwnd(), hdc );
	}
}

/* 選択範囲のデータを取得 */
/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
bool CEditView::GetSelectedData(
	CNativeW*		cmemBuf,
	BOOL			bLineOnly,
	const wchar_t*	pszQuote,			/* 先頭に付ける引用符 */
	BOOL			bWithLineNumber,	/* 行番号を付与する */
	BOOL			bAddCRLFWhenCopy,	/* 折り返し位置で改行記号を入れる */
	enumEOLType		neweol				//	コピー後の改行コード EOL_NONEはコード保存
)
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLayoutInt		nLineNum;
	CLogicInt		nIdxFrom;
	CLogicInt		nIdxTo;
	int				nRowNum;
	int				nLineNumCols;
	wchar_t*		pszLineNum;
	wchar_t*		pszSpaces = L"                    ";
	const CLayout*	pcLayout;
	CEol			appendEol( neweol );
	bool			addnl = false;

	/* 範囲選択がされていない */
	if( !GetSelectionInfo().IsTextSelected() ){
		return false;
	}
	if( bWithLineNumber ){	/* 行番号を付与する */
		/* 行番号表示に必要な桁数を計算 */
		nLineNumCols = GetTextArea().DetectWidthOfLineNumberArea_calculate();
		nLineNumCols += 1;
		pszLineNum = new wchar_t[nLineNumCols + 1];
	}

	CLayoutRect			rcSel;

	if( GetSelectionInfo().IsBoxSelecting() ){	/* 矩形範囲選択中 */
		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcSel,
			GetSelectionInfo().m_sSelect.GetFrom(),	// 範囲選択開始
			GetSelectionInfo().m_sSelect.GetTo()		// 範囲選択終了
		);
//		cmemBuf.SetData( "", 0 );
		cmemBuf->SetString(L"");

		//<< 2002/04/18 Azumaiya
		// サイズ分だけ要領をとっておく。
		// 結構大まかに見ています。
		CLayoutInt i = rcSel.bottom - rcSel.top;

		// 最初に行数分の改行量を計算してしまう。
		int nBufSize = wcslen(WCODE::CRLF) * (Int)i;

		// 実際の文字量。
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( rcSel.top, &nLineLen, &pcLayout );
		for(; i != CLayoutInt(0) && pcLayout != NULL; i--, pcLayout = pcLayout->GetNextLayout())
		{
			pLine = pcLayout->GetPtr() + pcLayout->GetLogicOffset();
			nLineLen = CLogicInt(pcLayout->GetLengthWithEOL());
			if( NULL != pLine )
			{
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSel.left  );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSel.right );

				nBufSize += nIdxTo - nIdxFrom;
			}
			if( bLineOnly ){	/* 複数行選択の場合は先頭の行のみ */
				break;
			}
		}

		// 大まかに見た容量を元にサイズをあらかじめ確保しておく。
		cmemBuf->AllocStringBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		nRowNum = 0;
		for( nLineNum = rcSel.top; nLineNum <= rcSel.bottom; ++nLineNum ){
//			if( nRowNum > 0 ){
//				cmemBuf.AppendSz( CRLF );
//				if( bLineOnly ){	/* 複数行選択の場合は先頭の行のみ */
//					break;
//				}
//			}
//			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSel.left  );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSel.right );
				//2002.02.08 hor
				// pLineがNULLのとき(矩形エリアの端がEOFのみの行を含むとき)は以下を処理しない
				if( nIdxTo - nIdxFrom > 0 ){
					if( pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r' ){
						cmemBuf->AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 );
					}else{
						cmemBuf->AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
					}
				}
			}
			++nRowNum;
//			if( nRowNum > 0 ){
				cmemBuf->AppendString( WCODE::CRLF );
				if( bLineOnly ){	/* 複数行選択の場合は先頭の行のみ */
					break;
				}
//			}
		}
	}
	else{
		cmemBuf->SetString(L"");

		//<< 2002/04/18 Azumaiya
		//  これから貼り付けに使う領域の大まかなサイズを取得する。
		//  大まかというレベルですので、サイズ計算の誤差が（容量を多く見積もる方に）結構出ると思いますが、
		// まぁ、速さ優先ということで勘弁してください。
		//  無駄な容量確保が出ていますので、もう少し精度を上げたいところですが・・・。
		//  とはいえ、逆に小さく見積もることになってしまうと、かなり速度をとられる要因になってしまうので
		// 困ってしまうところですが・・・。
		m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetSelectionInfo().m_sSelect.GetFrom().GetY2(), &nLineLen, &pcLayout );
		int nBufSize = 0;

		int i = (Int)(GetSelectionInfo().m_sSelect.GetTo().y - GetSelectionInfo().m_sSelect.GetFrom().y);

		// 先頭に引用符を付けるとき。
		if ( NULL != pszQuote )
		{
			nBufSize += wcslen(pszQuote);
		}

		// 行番号を付ける。
		if ( bWithLineNumber )
		{
			nBufSize += nLineNumCols;
		}

		// 改行コードについて。
		if ( neweol == EOL_UNKNOWN )
		{
			nBufSize += wcslen(WCODE::CRLF);
		}
		else
		{
			nBufSize += appendEol.GetLen();
		}

		// すべての行について同様の操作をするので、行数倍する。
		nBufSize *= (Int)i;

		// 実際の各行の長さ。
		for (; i != 0 && pcLayout != NULL; i--, pcLayout = pcLayout->GetNextLayout() )
		{
			nBufSize += pcLayout->GetLengthWithEOL() + appendEol.GetLen();
			if( bLineOnly ){	/* 複数行選択の場合は先頭の行のみ */
				break;
			}
		}

		// 調べた長さ分だけバッファを取っておく。
		cmemBuf->AllocStringBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		for( nLineNum = GetSelectionInfo().m_sSelect.GetFrom().GetY2(); nLineNum <= GetSelectionInfo().m_sSelect.GetTo().y; ++nLineNum ){
//			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL == pLine ){
				break;
			}
			if( nLineNum == GetSelectionInfo().m_sSelect.GetFrom().y ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom = LineColmnToIndex( pcLayout, GetSelectionInfo().m_sSelect.GetFrom().GetX2() );
			}else{
				nIdxFrom = CLogicInt(0);
			}
			if( nLineNum == GetSelectionInfo().m_sSelect.GetTo().y ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxTo = LineColmnToIndex( pcLayout, GetSelectionInfo().m_sSelect.GetTo().GetX2() );
			}else{
				nIdxTo = nLineLen;
			}
			if( nIdxTo - nIdxFrom == CLogicInt(0) ){
				continue;
			}

			if( NULL != pszQuote && 0 < wcslen( pszQuote ) ){	/* 先頭に付ける引用符 */
				cmemBuf->AppendString( pszQuote );
			}
			if( bWithLineNumber ){	/* 行番号を付与する */
				auto_sprintf( pszLineNum, L" %d:" , nLineNum + 1 );
				cmemBuf->AppendString( pszSpaces, nLineNumCols - wcslen( pszLineNum ) );
				cmemBuf->AppendString( pszLineNum );
			}


			if( EOL_NONE != pcLayout->GetLayoutEol() ){
//			if( pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r' ){
//				cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 );
//				cmemBuf.AppendSz( CRLF );

				if( nIdxTo >= nLineLen ){
					cmemBuf->AppendString( &pLine[nIdxFrom], nLineLen - 1 - nIdxFrom );
					//	Jul. 25, 2000 genta
					cmemBuf->AppendString( ( neweol == EOL_UNKNOWN ) ?
						(pcLayout->GetLayoutEol()).GetValue2() :	//	コード保存
						appendEol.GetValue2() );			//	新規改行コード
				}
				else {
					cmemBuf->AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
				}
			}else{
				cmemBuf->AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
				if( nIdxTo - nIdxFrom >= nLineLen ){
					if( bAddCRLFWhenCopy ||  /* 折り返し行に改行を付けてコピー */
						NULL != pszQuote || /* 先頭に付ける引用符 */
						bWithLineNumber 	/* 行番号を付与する */
					){
//						cmemBuf.Append( CRLF, lstrlen( CRLF ) );
						//	Jul. 25, 2000 genta
						cmemBuf->AppendString(( neweol == EOL_UNKNOWN ) ?
							WCODE::CRLF :						//	コード保存
							appendEol.GetValue2() );		//	新規改行コード
					}
				}
			}
			if( bLineOnly ){	/* 複数行選択の場合は先頭の行のみ */
				break;
			}
		}
	}
	if( bWithLineNumber ){	/* 行番号を付与する */
		delete [] pszLineNum;
	}
	return true;
}




/* 選択範囲内の全行をクリップボードにコピーする */
void CEditView::CopySelectedAllLines(
	const wchar_t*	pszQuote,		//!< 先頭に付ける引用符
	BOOL			bWithLineNumber	//!< 行番号を付与する
)
{
	RECT		rcSel;
	CNativeW	cmemBuf;

	if( !GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		return;
	}
	/* 矩形範囲選択中か */
	if( GetSelectionInfo().IsBoxSelecting() ){
		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcSel,
			GetSelectionInfo().m_sSelect.GetFrom(),	// 範囲選択開始
			GetSelectionInfo().m_sSelect.GetTo()		// 範囲選択終了
		);

		/* 現在の選択範囲を非選択状態に戻す */
		GetSelectionInfo().DisableSelectArea( TRUE );

		/* 挿入データの先頭位置へカーソルを移動 */
		GetSelectionInfo().m_sSelect.SetFrom(CLayoutPoint(0,rcSel.top     )); // 範囲選択開始
		GetSelectionInfo().m_sSelect.SetTo  (CLayoutPoint(0,rcSel.bottom+1)); // 範囲選択終了
	}
	else{
		CLayoutRange sSelectOld;
		sSelectOld.SetFrom(CLayoutPoint(CLayoutInt(0),GetSelectionInfo().m_sSelect.GetFrom().y));
		sSelectOld.SetTo  (CLayoutPoint(CLayoutInt(0),GetSelectionInfo().m_sSelect.GetTo().y  ));
		if( GetSelectionInfo().m_sSelect.GetTo().x > 0 ){
			sSelectOld.GetToPointer()->y++;
		}
		/*
		int			nSelectLineFromOld;	// 範囲選択開始行
		int			nSelectColFromOld; 	// 範囲選択開始桁
		int			nSelectLineToOld;	// 範囲選択終了行
		int			nSelectColToOld;	// 範囲選択終了桁
		nSelectLineFromOld = GetSelectionInfo().m_sSelect.GetFrom().y;	// 範囲選択開始行
		nSelectColFromOld = 0;					// 範囲選択開始桁
		nSelectLineToOld = GetSelectionInfo().m_sSelect.GetTo().y;		// 範囲選択終了行
		if( GetSelectionInfo().m_sSelect.GetTo().x > 0 ){
			++nSelectLineToOld;					// 範囲選択終了行
		}
		nSelectColToOld = 0;					// 範囲選択終了桁
		*/
		// 現在の選択範囲を非選択状態に戻す
		GetSelectionInfo().DisableSelectArea( TRUE );
		GetSelectionInfo().m_sSelect = sSelectOld;		//範囲選択
	}
	/* 再描画 */
	//	::UpdateWindow();
	Call_OnPaint(PAINT_LINENUMBER | PAINT_BODY, TRUE); // メモリＤＣを使用してちらつきのない再描画
	/* 選択範囲をクリップボードにコピー */
	/* 選択範囲のデータを取得 */
	/* 正常時はTRUE,範囲未選択の場合は終了する */
	if( !GetSelectedData(
		&cmemBuf,
		FALSE,
		pszQuote, /* 引用符 */
		bWithLineNumber, /* 行番号を付与する */
		m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy /* 折り返し位置に改行記号を入れる */
	) ){
		ErrorBeep();
		return;
	}
	/* クリップボードにデータを設定 */
	MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), false );


	return;
}

/* 選択エリアのテキストを指定方法で変換 */
void CEditView::ConvSelectedArea( EFunctionCode nFuncCode )
{
	CNativeW	cmemBuf;

	CLayoutPoint sPos;

	CLogicInt	nIdxFrom;
	CLogicInt	nIdxTo;
	CLayoutInt	nLineNum;
	CLogicInt	nDelLen;
	CLogicInt	nDelPosNext;
	CLogicInt	nDelLenNext;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	const wchar_t*	pLine2;
	CLogicInt		nLineLen2;
	CWaitCursor cWaitCursor( GetHwnd() );

	BOOL		bBeginBoxSelectOld;

	/* テキストが選択されているか */
	if( !GetSelectionInfo().IsTextSelected() ){
		return;
	}

	CLayoutRange sSelectOld = GetSelectionInfo().m_sSelect;		// 範囲選択

	bBeginBoxSelectOld	= GetSelectionInfo().IsBoxSelecting();

	/* 矩形範囲選択中か */
	if( GetSelectionInfo().IsBoxSelecting() ){

		/* 2点を対角とする矩形を求める */
		CLayoutRect	rcSelLayout;
		TwoPointToRect(
			&rcSelLayout,
			GetSelectionInfo().m_sSelect.GetFrom(),	// 範囲選択開始
			GetSelectionInfo().m_sSelect.GetTo()		// 範囲選択終了
		);

		/* 現在の選択範囲を非選択状態に戻す */
		GetSelectionInfo().DisableSelectArea( TRUE );

		nIdxFrom = CLogicInt(0);
		nIdxTo = CLogicInt(0);
		for( nLineNum = rcSelLayout.bottom; nLineNum >= rcSelLayout.top - 1; nLineNum-- ){
			const CLayout* pcLayout;
			nDelPosNext = nIdxFrom;
			nDelLenNext	= nIdxTo - nIdxFrom;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSelLayout.left );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSelLayout.right );

				for( CLogicInt i = nIdxFrom; i <= nIdxTo; ++i ){
					if( pLine[i] == WCODE::CR || pLine[i] == WCODE::LF ){
						nIdxTo = i;
						break;
					}
				}
			}else{
				nIdxFrom	= CLogicInt(0);
				nIdxTo		= CLogicInt(0);
			}
			CLogicInt	nDelPos = nDelPosNext;
			nDelLen	= nDelLenNext;
			if( nLineNum < rcSelLayout.bottom && 0 < nDelLen ){
				pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum + CLayoutInt(1), &nLineLen2, &pcLayout );
				sPos.Set(
					LineIndexToColmn( pcLayout, nDelPos ),
					nLineNum + 1
				);

				//2007.10.18 COpe整理
				// 指定位置の指定長データ削除
				DeleteData2(
					sPos,
					nDelLen,
					&cmemBuf
				);
				
				{
					/* 機能種別によるバッファの変換 */
					CConvertMediator::ConvMemory( &cmemBuf, nFuncCode, (Int)m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );

					/* 現在位置にデータを挿入 */
					CLayoutPoint ptLayoutNew;	// 挿入された部分の次の位置
					InsertData_CEditView(
						sPos,
						cmemBuf.GetStringPtr(),
						cmemBuf.GetStringLength(),
						&ptLayoutNew,
						true
					);

					/* カーソルを移動 */
					GetCaret().MoveCursor( ptLayoutNew, FALSE );
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
				}
			}
		}
		/* 挿入データの先頭位置へカーソルを移動 */
		GetCaret().MoveCursor( rcSelLayout.UpperLeft(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
					GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
				)
			);
		}
	}
	else{
		/* 選択範囲のデータを取得 */
		/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
		GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy );

		/* 機能種別によるバッファの変換 */
		CConvertMediator::ConvMemory( &cmemBuf, nFuncCode, (Int)m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );

//		/* 選択エリアを削除 */
//		DeleteData( FALSE );

		CLayoutInt nCaretPosYOLD=GetCaret().GetCaretLayoutPos().GetY();

		/* データ置換 削除&挿入にも使える */
		ReplaceData_CEditView(
			GetSelectionInfo().m_sSelect,
			NULL,					/* 削除されたデータのコピー(NULL可能) */
			cmemBuf.GetStringPtr(),		/* 挿入するデータ */ // 2002/2/10 aroka CMemory変更
			cmemBuf.GetStringLength(),	/* 挿入するデータの長さ */ // 2002/2/10 aroka CMemory変更
			false,
			m_bDoing_UndoRedo?NULL:m_pcOpeBlk
		);

		// From Here 2001.12.03 hor
		//	選択エリアの復元
		GetSelectionInfo().m_sSelect.SetFrom(sSelectOld.GetFrom());	// 範囲選択開始位置
		GetSelectionInfo().m_sSelect.SetTo(GetCaret().GetCaretLayoutPos());	// 範囲選択終了位置
		if(nCaretPosYOLD==GetSelectionInfo().m_sSelect.GetFrom().y) {
			GetCaret().MoveCursor( GetSelectionInfo().m_sSelect.GetFrom(), TRUE );
		}else{
			GetCaret().MoveCursor( GetSelectionInfo().m_sSelect.GetTo(), TRUE );
		}
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
					GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
				)
			);
		}
		RedrawAll();
		// To Here 2001.12.03 hor

		return;
	}

	return;
}





/* ポップアップメニュー(右クリック) */
int	CEditView::CreatePopUpMenu_R( void )
{
	int			nId;
	HMENU		hMenu;
	POINT		po;
	RECT		rc;
	CMemory		cmemCurText;
	int			i;
	int			nMenuIdx;
	WCHAR		szLabel[300];
	WCHAR		szLabel2[300];
	UINT		uFlags;


	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->GetMenuDrawer().ResetContents();

	/* 右クリックメニューの定義はカスタムメニュー配列の0番目 */
	nMenuIdx = CUSTMENU_INDEX_FOR_RBUTTONUP;	//マジックナンバー排除	//@@@ 2003.06.13 MIK

	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = m_pcEditDoc->m_cFuncLookup;

	hMenu = ::CreatePopupMenu();
	for( i = 0; i < m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx]; ++i ){
		if( 0 == m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ){
			::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
		}else{
			//	Oct. 3, 2001 genta
			FuncLookup.Funccode2Name( m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
			/* キー */
			if( L'\0' == m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i] ){
				auto_strcpy( szLabel2, szLabel );
			}else{
				auto_sprintf( szLabel2, LTEXT("%ls (&%lc)"),
					szLabel,
					m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i]
				);
			}
			/* 機能が利用可能か調べる */
			if( TRUE == IsFuncEnable( m_pcEditDoc, m_pShareData, m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ) ){
				uFlags = MF_STRING | MF_ENABLED;
			}else{
				uFlags = MF_STRING | MF_DISABLED | MF_GRAYED;
			}
			pCEditWnd->GetMenuDrawer().MyAppendMenu(
				hMenu, /*MF_BYPOSITION | MF_STRING*/uFlags,
				m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] , szLabel2 );

		}
	}

	if( !GetSelectionInfo().IsMouseSelecting() ){	/* 範囲選択中 */
		if( TRUE == KeyWordHelpSearchDict( LID_SKH_POPUPMENU_R, &po, &rc ) ){	// 2006.04.10 fon
			const TCHAR*	pszWork;
			pszWork = m_cTipWnd.m_cInfo.GetStringPtr();
			// 2002.05.25 Moca &の考慮を追加 
			TCHAR*	pszShortOut = new TCHAR[160 + 1];
			if( 80 < _tcslen( pszWork ) ){
				TCHAR*	pszShort = new TCHAR[80 + 1];
				auto_memcpy( pszShort, pszWork, 80 );
				pszShort[80] = _T('\0');
				dupamp( pszShort, pszShortOut );
				delete [] pszShort;
			}else{
				dupamp( pszWork, pszShortOut );
			}
			::InsertMenu( hMenu, 0, MF_BYPOSITION, IDM_COPYDICINFO, _T("キーワードの説明をクリップボードにコピー(&K)") );	// 2006.04.10 fon ToolTip内容を直接表示するのをやめた
			delete [] pszShortOut;
			::InsertMenu( hMenu, 1, MF_BYPOSITION, IDM_JUMPDICT, _T("キーワード辞書を開く(&J)") );	// 2006.04.10 fon
			::InsertMenu( hMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
		}
	}
	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );
	po.y -= 4;
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		::GetParent( m_hwndParent )/*GetHwnd()*/,
		NULL
	);
	::DestroyMenu( hMenu );
	return nId;
}





/* 設定変更を反映させる */
void CEditView::OnChangeSetting( void )
{
	RECT		rc;

	GetTextArea().SetTopYohaku( m_pShareData->m_Common.m_sWindow.m_nRulerBottomSpace ); 		/* ルーラーとテキストの隙間 */
	GetTextArea().SetAreaTop( GetTextArea().GetTopYohaku() );									/* 表示域の上端座標 */

	/* ルーラー表示 */
	if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		GetTextArea().SetAreaTop(GetTextArea().GetAreaTop() + m_pShareData->m_Common.m_sWindow.m_nRulerHeight);	/* ルーラー高さ */
	}

	// フォント更新
	GetFontset().UpdateFont();

	/* フォントの変更 */
	SetFont();

	/* フォントが変わっているかもしれないので、カーソル移動 */
	GetCaret().MoveCursor( GetCaret().GetCaretLayoutPos(), TRUE );

	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	/* ウィンドウサイズの変更処理 */
	::GetClientRect( GetHwnd(), &rc );
	OnSize( rc.right, rc.bottom );

	/* 再描画 */
	::InvalidateRect( GetHwnd(), NULL, TRUE );

	return;
}




/* フォーカス移動時の再描画

	@date 2001/06/21 asa-o 「スクロールバーの状態を更新する」「カーソル移動」削除
*/
void CEditView::RedrawAll( void )
{
	// ウィンドウ全体を再描画
	PAINTSTRUCT	ps;
	HDC hdc = ::GetDC( GetHwnd() );
	::GetClientRect( GetHwnd(), &ps.rcPaint );
	OnPaint( hdc, &ps, FALSE );	// メモリＤＣを使用してちらつきのない再描画
	::ReleaseDC( GetHwnd(), hdc );

	// キャレットの表示
	GetCaret().ShowEditCaret();

	// キャレットの行桁位置を表示する
	GetCaret().ShowCaretPosInfo();

	// 親ウィンドウのタイトルを更新
	m_pcEditWnd->UpdateCaption();

	//	Jul. 9, 2005 genta	選択範囲の情報をステータスバーへ表示
	GetSelectionInfo().PrintSelectionInfoMsg();

	// スクロールバーの状態を更新する
	AdjustScrollBars();

	return;
}

// 2001/06/21 Start by asa-o 再描画
void CEditView::Redraw( void )
{
	HDC			hdc;
	PAINTSTRUCT	ps;

	hdc = ::GetDC( GetHwnd() );

	::GetClientRect( GetHwnd(), &ps.rcPaint );

	OnPaint( hdc, &ps, FALSE );	/* メモリＤＣを使用してちらつきのない再描画 */

	::ReleaseDC( GetHwnd(), hdc );

	return;
}
// 2001/06/21 End

/* 自分の表示状態を他のビューにコピー */
void CEditView::CopyViewStatus( CEditView* pView )
{
	if( pView == NULL ){
		return;
	}
	if( pView == this ){
		return;
	}

	// 入力状態
	GetCaret().CopyCaretStatus(&pView->GetCaret());

	// 選択状態
	GetSelectionInfo().CopySelectStatus(&pView->GetSelectionInfo());

	/* 画面情報 */
	GetTextArea().CopyTextAreaStatus(&pView->GetTextArea());

	/* 表示方法 */
	GetTextMetrics().CopyTextMetricsStatus(&pView->GetTextMetrics());

	return;
}


/* 縦・横の分割ボックス・サイズボックスのＯＮ／ＯＦＦ */
void CEditView::SplitBoxOnOff( BOOL bVert, BOOL bHorz, BOOL bSizeBox )
{
	RECT	rc;
	if( bVert ){
		if( NULL != m_pcsbwVSplitBox ){	/* 垂直分割ボックス */
		}else{
			m_pcsbwVSplitBox = new CSplitBoxWnd;
			m_pcsbwVSplitBox->Create( m_hInstance, GetHwnd(), TRUE );
		}
	}else{
		if( NULL != m_pcsbwVSplitBox ){	/* 垂直分割ボックス */
			delete m_pcsbwVSplitBox;
			m_pcsbwVSplitBox = NULL;
		}else{
		}
	}
	if( bHorz ){
		if( NULL != m_pcsbwHSplitBox ){	/* 水平分割ボックス */
		}else{
			m_pcsbwHSplitBox = new CSplitBoxWnd;
			m_pcsbwHSplitBox->Create( m_hInstance, GetHwnd(), FALSE );
		}
	}else{
		if( NULL != m_pcsbwHSplitBox ){	/* 水平分割ボックス */
			delete m_pcsbwHSplitBox;
			m_pcsbwHSplitBox = NULL;
		}else{
		}
	}

	if( bSizeBox ){
		if( NULL != m_hwndSizeBox ){
			::DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		m_hwndSizeBox = ::CreateWindowEx(
			0L,													/* no extended styles */
			_T("SCROLLBAR"),										/* scroll bar control class */
			NULL,												/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,													/* horizontal position */
			0,													/* vertical position */
			200,												/* width of the scroll bar */
			CW_USEDEFAULT,										/* default height */
			GetHwnd(),												/* handle of main window */
			(HMENU) NULL,										/* no menu for a scroll bar */
			m_hInstance,										/* instance owning this window */
			(LPVOID) NULL										/* pointer not needed */
		);
	}else{
		if( NULL != m_hwndSizeBox ){
			DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		m_hwndSizeBox = ::CreateWindowEx(
			0L,														/* no extended styles */
			_T("STATIC"),											/* scroll bar control class */
			NULL,													/* text for window title bar */
			WS_VISIBLE | WS_CHILD /*| SBS_SIZEBOX | SBS_SIZEGRIP*/, /* scroll bar styles */
			0,														/* horizontal position */
			0,														/* vertical position */
			200,													/* width of the scroll bar */
			CW_USEDEFAULT,											/* default height */
			GetHwnd(),													/* handle of main window */
			(HMENU) NULL,											/* no menu for a scroll bar */
			m_hInstance,											/* instance owning this window */
			(LPVOID) NULL											/* pointer not needed */
		);
	}
	::ShowWindow( m_hwndSizeBox, SW_SHOW );

	::GetClientRect( GetHwnd(), &rc );
	OnSize( rc.right, rc.bottom );

	return;
}





/* 指定カーソル位置が選択エリア内にあるか
	【戻り値】
	-1	選択エリアより前方 or 無選択
	0	選択エリア内
	1	選択エリアより後方
*/
int CEditView::IsCurrentPositionSelected(
	CLayoutPoint	ptCaretPos		// カーソル位置
)
{
	if( !GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		return -1;
	}
	CLayoutRect		rcSel;
	CLayoutPoint	po;

	/* 矩形範囲選択中か */
	if( GetSelectionInfo().IsBoxSelecting() ){
		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcSel,
			GetSelectionInfo().m_sSelect.GetFrom(),	// 範囲選択開始
			GetSelectionInfo().m_sSelect.GetTo()		// 範囲選択終了
		);
		++rcSel.bottom;
		po = ptCaretPos;
		if( m_bDragSource ){
			if( GetKeyState_Control() ){ /* Ctrlキーが押されていたか */
				++rcSel.left;
			}else{
				++rcSel.right;
			}
		}
		if( rcSel.PtInRect( po ) ){
			return 0;
		}
		if( rcSel.top > ptCaretPos.y ){
			return -1;
		}
		if( rcSel.bottom < ptCaretPos.y ){
			return 1;
		}
		if( rcSel.left > ptCaretPos.x ){
			return -1;
		}
		if( rcSel.right < ptCaretPos.x ){
			return 1;
		}
	}else{
		if( GetSelectionInfo().m_sSelect.GetFrom().y > ptCaretPos.y ){
			return -1;
		}
		if( GetSelectionInfo().m_sSelect.GetTo().y < ptCaretPos.y ){
			return 1;
		}
		if( GetSelectionInfo().m_sSelect.GetFrom().y == ptCaretPos.y ){
			if( m_bDragSource ){
				if( GetKeyState_Control() ){	/* Ctrlキーが押されていたか */
					if( GetSelectionInfo().m_sSelect.GetFrom().x >= ptCaretPos.x ){
						return -1;
					}
				}else{
					if( GetSelectionInfo().m_sSelect.GetFrom().x > ptCaretPos.x ){
						return -1;
					}
				}
			}else
			if( GetSelectionInfo().m_sSelect.GetFrom().x > ptCaretPos.x ){
				return -1;
			}
		}
		if( GetSelectionInfo().m_sSelect.GetTo().y == ptCaretPos.y ){
			if( m_bDragSource ){
				if( GetKeyState_Control() ){	/* Ctrlキーが押されていたか */
					if( GetSelectionInfo().m_sSelect.GetTo().x <= ptCaretPos.x ){
						return 1;
					}
				}else{
					if( GetSelectionInfo().m_sSelect.GetTo().x < ptCaretPos.x ){
						return 1;
					}
				}
			}else
			if( GetSelectionInfo().m_sSelect.GetTo().x <= ptCaretPos.x ){
				return 1;
			}
		}
		return 0;
	}
	return -1;
}

/* 指定カーソル位置が選択エリア内にあるか (テスト)
	【戻り値】
	-1	選択エリアより前方 or 無選択
	0	選択エリア内
	1	選択エリアより後方
*/
//2007.09.01 kobake 整理
int CEditView::IsCurrentPositionSelectedTEST(
	const CLayoutPoint& ptCaretPos,      //カーソル位置
	const CLayoutRange& sSelect //
) const
{
	if( !GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		return -1;
	}

	if( PointCompare( ptCaretPos, sSelect.GetFrom() ) < 0 )return -1;
	if( PointCompare( ptCaretPos, sSelect.GetTo() ) >= 0 )return 1;

	return 0;
}

/*! クリップボードからデータを取得
	@date 2005/05/29 novice UNICODE TEXT 対応処理を追加
	@date 2007.10.04 ryoji MSDEVLineSelect対応処理を追加
*/
bool CEditView::MyGetClipboardData( CNativeW& cmemBuf, bool* pbColmnSelect, bool* pbLineSelect /*= NULL*/ )
{
	if(pbColmnSelect)
		*pbColmnSelect = false;

	if(pbLineSelect)
		*pbLineSelect = false;

	if(!CClipboard::HasValidData())
		return false;
	
	CClipboard cClipboard(GetHwnd());
	if(!cClipboard)
		return false;

	return cClipboard.GetText(&cmemBuf,pbColmnSelect,pbLineSelect);
}

/* クリップボードにデータを設定
	@date 2004.02.17 Moca エラーチェックするように
 */
bool CEditView::MySetClipboardData( const ACHAR* pszText, int nTextLen, bool bColmnSelect, bool bLineSelect /*= false*/ )
{
	//WCHARに変換
	vector<wchar_t> buf;
	mbstowcs_vector(pszText,nTextLen,&buf);
	return MySetClipboardData(&buf[0],buf.size()-1,bColmnSelect,bLineSelect);
}
bool CEditView::MySetClipboardData( const WCHAR* pszText, int nTextLen, bool bColmnSelect, bool bLineSelect /*= false*/ )
{
	/* Windowsクリップボードにコピー */
	CClipboard cClipboard(GetHwnd());
	if(!cClipboard){
		return false;
	}
	cClipboard.Empty();
	return cClipboard.SetText(pszText,nTextLen,bColmnSelect,bLineSelect);
}



STDMETHODIMP CEditView::DragEnter( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE_A( "CEditView::DragEnter()\n" );
#endif
	//「OLEによるドラッグ & ドロップを使う」オプションが無効の場合にはドロップを受け付けない
	if(!m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop)return E_UNEXPECTED;

	//ビューモードの場合はドロップを受け付けない
	if(CAppMode::Instance()->IsViewMode())return E_UNEXPECTED;

	//上書き禁止の場合はドロップを受け付けない
	if(!m_pcEditDoc->m_cDocLocker.IsDocWritable())return E_UNEXPECTED;


	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;

	if( IsDataAvailable( pDataObject, CF_UNICODETEXT) || IsDataAvailable( pDataObject, CF_TEXT) ){
		/* 自分をアクティブペインにする */
		m_pcEditDoc->m_pcEditWnd->SetActivePane( m_nMyIndex );

		/* 選択テキストのドラッグ中か */
		_SetDragMode(TRUE);

		/* 入力フォーカスを受け取ったときの処理 */
		OnSetFocus();

		m_pcDropTarget->m_pDataObject = pDataObject;
		/* Ctrl,ALT,キーが押されていたか */
		if( GetKeyState_Control() || !m_bDragSource ){	// Aug. 6, 2004 genta DragOver/Dropでは入っているがここだけ漏れていた
			*pdwEffect = DROPEFFECT_COPY;
		}else{
			*pdwEffect = DROPEFFECT_MOVE;
		}

		// アクティブにする
		::SetFocus( GetHwnd() );
	}
	else{
		return E_INVALIDARG;
	}

	DragOver( dwKeyState, pt, pdwEffect );
	return S_OK;
}

STDMETHODIMP CEditView::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE_A( "CEditView::DragOver()\n" );
#endif

	/* マウス移動のメッセージ処理 */
	::ScreenToClient( GetHwnd(), (LPPOINT)&pt );
	OnMOUSEMOVE( dwKeyState, pt.x , pt.y );

	if ( pdwEffect == NULL )
		return E_INVALIDARG;

	if( NULL == m_pcDropTarget->m_pDataObject ){
		*pdwEffect = DROPEFFECT_NONE;
	}
	/* 指定カーソル位置が選択エリア内にあるか */
	else if( m_bDragSource && 0==IsCurrentPositionSelected(GetCaret().GetCaretLayoutPos()) ){
		*pdwEffect = DROPEFFECT_NONE;
	}
	/* Ctrl,ALT,キーが押されていたか */
	else if( GetKeyState_Control() || !m_bDragSource ){
		*pdwEffect = DROPEFFECT_COPY;
	}
	else{
		*pdwEffect = DROPEFFECT_MOVE;
	}
	return S_OK;
}



STDMETHODIMP CEditView::DragLeave( void )
{
#ifdef _DEBUG
	MYTRACE_A( "CEditView::DragLeave()\n" );
#endif

	if( !m_bDragSource ){
		/* 入力フォーカスを失ったときの処理 */
		OnKillFocus();
		::SetFocus(NULL);
	}else{
		// 1999.11.15
		OnSetFocus();
		::SetFocus( ::GetParent( m_hwndParent ) );
	}

	m_pcDropTarget->m_pDataObject = NULL;

	/* 選択テキストのドラッグ中か */
	_SetDragMode(FALSE);

	return S_OK;
}

STDMETHODIMP CEditView::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE_A( "CEditView::Drop()\n" );
#endif
	BOOL		bBoxSelected;
	BOOL		bMove;
	BOOL		bMoveToPrev;
	RECT		rcSel;
	CNativeW	cmemBuf;
	bool		bBeginBoxSelect_Old;

	CLayoutRange sSelectBgn_Old;
	CLayoutRange sSelect_Old;

	if( !m_bDragSource && GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		GetSelectionInfo().DisableSelectArea( TRUE );
	}



	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;

	*pdwEffect = DROPEFFECT_NONE;

	CLIPFORMAT cf = 0;
	if( IsDataAvailable(pDataObject, CF_UNICODETEXT) )
		cf = CF_UNICODETEXT;
	else if( IsDataAvailable( pDataObject, CF_TEXT) )
		cf = CF_TEXT;
	if( cf == CF_UNICODETEXT || cf == CF_TEXT ){
		HGLOBAL		hData = GetGlobalData(pDataObject, cf);
#ifdef _DEBUG
		MYTRACE_A( "%xh == GetGlobalData(pDataObject, %d)\n", hData, cf );
#endif
		if (hData == NULL){
			m_pcDropTarget->m_pDataObject = NULL;
			/* 選択テキストのドラッグ中か */
			_SetDragMode(FALSE);
			return E_INVALIDARG;
		}

		DWORD	nSize = 0;
		LPVOID lpszSource = ::GlobalLock(hData);
		if( cf == CF_UNICODETEXT )
			cmemBuf.SetString( (LPWSTR)lpszSource );
		else
			cmemBuf.SetStringOld( (LPSTR)lpszSource );

		/* 移動かコピーか */
		if( GetKeyState_Control() || !m_bDragSource){
			bMove = FALSE;
			*pdwEffect = DROPEFFECT_COPY;
		}
		else{
			bMove = TRUE;
			*pdwEffect = DROPEFFECT_MOVE;
		}

		if( m_bDragSource ){
			if( NULL != m_pcOpeBlk ){
			}else{
				m_pcOpeBlk = new COpeBlk;
			}
			bBoxSelected = GetSelectionInfo().IsBoxSelecting();

			/* 移動の場合、位置関係を算出 */
			if( bMove ){
				if( bBoxSelected ){
					/* 2点を対角とする矩形を求める */
					TwoPointToRect(
						&rcSel,
						GetSelectionInfo().m_sSelect.GetFrom(),	// 範囲選択開始
						GetSelectionInfo().m_sSelect.GetTo()		// 範囲選択終了
					);
					++rcSel.bottom;
					if( GetCaret().GetCaretLayoutPos().GetY() >= rcSel.bottom ){
						bMoveToPrev = FALSE;
					}else
					if( GetCaret().GetCaretLayoutPos().GetY() + rcSel.bottom - rcSel.top < rcSel.top ){
						bMoveToPrev = TRUE;
					}else
					if( GetCaret().GetCaretLayoutPos().GetX2() < rcSel.left ){
						bMoveToPrev = TRUE;
					}else{
						bMoveToPrev = FALSE;
					}
				}else{
					if( GetSelectionInfo().m_sSelect.GetFrom().y > GetCaret().GetCaretLayoutPos().GetY() ){
						bMoveToPrev = TRUE;
					}else
					if( GetSelectionInfo().m_sSelect.GetFrom().y == GetCaret().GetCaretLayoutPos().GetY() ){
						if( GetSelectionInfo().m_sSelect.GetFrom().x > GetCaret().GetCaretLayoutPos().GetX2() ){
							bMoveToPrev = TRUE;
						}else{
							bMoveToPrev = FALSE;
						}
					}else{
						bMoveToPrev = FALSE;
					}
				}
			}
			if( !bMove ){
				/* コピーモード */
				/* 現在の選択範囲を非選択状態に戻す */
				GetSelectionInfo().DisableSelectArea( TRUE );
			}

			CLayoutPoint ptCaretPos_Old = GetCaret().GetCaretLayoutPos();
			if( bMove ){
				if( bMoveToPrev ){
					/* 移動モード & 前に移動 */
					/* 選択エリアを削除 */
					DeleteData( TRUE );
					GetCaret().MoveCursor( ptCaretPos_Old, TRUE );
				}else{
					bBeginBoxSelect_Old = GetSelectionInfo().IsBoxSelecting();

					sSelectBgn_Old = GetSelectionInfo().m_sSelectBgn;

					sSelect_Old = GetSelectionInfo().m_sSelect;

					/* 現在の選択範囲を非選択状態に戻す */
					GetSelectionInfo().DisableSelectArea( TRUE );
				}
			}
			if( FALSE == bBoxSelected ){	/* 矩形範囲選択中 */
				//	2004,05.14 Moca 引数に文字列長を追加
				GetCommander().Command_INSTEXT( TRUE, cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), FALSE );
			}else{
				// 2004.07.12 Moca クリップボードを書き換えないように
				// TRUE == bBoxSelected
				// FALSE == GetSelectionInfo().IsBoxSelecting()
				/* 貼り付け（クリップボードから貼り付け）*/
				GetCommander().Command_PASTEBOX( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength() );
				AdjustScrollBars(); // 2007.07.22 ryoji
				Redraw();
			}
			if( bMove ){
				if( bMoveToPrev ){
				}else{
					/* 移動モード & 後ろに移動*/
					GetSelectionInfo().SetBoxSelect(bBeginBoxSelect_Old);

					GetSelectionInfo().m_sSelectBgn = sSelectBgn_Old;
					GetSelectionInfo().m_sSelect = sSelect_Old;

					/* 選択エリアを削除 */
					DeleteData( TRUE );
					GetCaret().MoveCursor( ptCaretPos_Old, TRUE );
				}
			}
			/* アンドゥバッファの処理 */
			if( NULL != m_pcOpeBlk ){
				if( 0 < m_pcOpeBlk->GetNum() ){	/* 操作の数を返す */
					/* 操作の追加 */
					m_pcEditDoc->m_cDocEditor.m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
					m_pcEditDoc->m_pcEditWnd->RedrawInactivePane();	// 他のペインの表示	// 2007.07.22 ryoji
				}else{
					delete m_pcOpeBlk;
				}
				m_pcOpeBlk = NULL;
			}
		}else{
			GetCommander().HandleCommand( F_INSTEXT_W, TRUE, (LPARAM)cmemBuf.GetStringPtr(), TRUE, 0, 0 );
		}
		::GlobalUnlock(hData);
		// 2004.07.12 fotomo/もか メモリーリークの修正
		if( 0 == (GMEM_LOCKCOUNT & ::GlobalFlags(hData)) ){
			::GlobalFree(hData);
		}
	}else{
#ifdef _DEBUG
		MYTRACE_A( "FALSE == IsDataAvailable()\n" );
#endif
	}
	m_pcDropTarget->m_pDataObject = NULL;

	/* 選択テキストのドラッグ中か */
	_SetDragMode(FALSE);

	/* 編集ウィンドウオブジェクトからのアクティブ要求 */
	::SetFocus( ::GetParent( m_hwndParent ) );
	SetActiveWindow( GetHwnd() );

	return S_OK;
}





/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
void CEditView::GetCurrentTextForSearch( CNativeW& cmemCurText )
{

	int				i;
	wchar_t			szTopic[_MAX_PATH];
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLogicInt		nIdx;
	CLayoutRange	sRange;

	cmemCurText.SetString(L"");
	szTopic[0] = L'\0';
	if( GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 選択範囲のデータを取得 */
		if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
			/* 検索文字列を現在位置の単語で初期化 */
			wcsncpy( szTopic, cmemCurText.GetStringPtr(), _MAX_PATH - 1 );
			szTopic[_MAX_PATH - 1] = L'\0';
		}
	}else{
		const CLayout*	pcLayout;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* 指定された桁に対応する行のデータ内の位置を調べる */
			nIdx = LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

			/* 現在位置の単語の範囲を調べる */
			if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
				GetCaret().GetCaretLayoutPos().GetY2(), nIdx,
				&sRange, NULL, NULL )
			){
				// 指定された行のデータ内の位置に対応する桁の位置を調べる
				// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
				/*
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetFrom().GetY2(), &nLineLen, &pcLayout );
				sRange.SetFromX( LineIndexToColmn( pcLayout, sRange.GetFrom().x ) );
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetTo().GetY2(), &nLineLen, &pcLayout );
				sRange.SetToX( LineIndexToColmn( pcLayout, sRange.GetTo().x ) );
				*/

				/* 選択範囲の変更 */
				GetSelectionInfo().m_sSelectBgn = sRange;
				GetSelectionInfo().m_sSelect    = sRange;

				/* 選択範囲のデータを取得 */
				if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
					/* 検索文字列を現在位置の単語で初期化 */
					wcsncpy( szTopic, cmemCurText.GetStringPtr(), MAX_PATH - 1 );
					szTopic[MAX_PATH - 1] = L'\0';
				}
				/* 現在の選択範囲を非選択状態に戻す */
				GetSelectionInfo().DisableSelectArea( FALSE );
			}
		}
	}

	/* 検索文字列は改行まで */
	for( i = 0; i < (int)wcslen( szTopic ); ++i ){
		if( szTopic[i] == WCODE::CR || szTopic[i] == WCODE::LF ){
			szTopic[i] = L'\0';
			break;
		}
	}
//	cmemCurText.SetData( szTopic, lstrlen( szTopic ) );
	cmemCurText.SetString( szTopic );
	return;

}


/*!	現在カーソル位置単語または選択範囲より検索等のキーを取得（ダイアログ用）
	@date 2006.08.23 ryoji 新規作成
*/
void CEditView::GetCurrentTextForSearchDlg( CNativeW& cmemCurText )
{
	cmemCurText.SetString(L"");

	if( GetSelectionInfo().IsTextSelected() ){	// テキストが選択されている
		GetCurrentTextForSearch( cmemCurText );
	}
	else{	// テキストが選択されていない
		if( m_pShareData->m_Common.m_sSearch.m_bCaretTextForSearch ){
			GetCurrentTextForSearch( cmemCurText );	// カーソル位置単語を取得
		}
		else{
			cmemCurText.SetString( m_pShareData->m_aSearchKeys[0] );	// 履歴からとってくる
		}
	}
}


/* カーソル行アンダーラインのON */
void CEditView::CaretUnderLineON( bool bDraw )
{
	if( FALSE == m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp ){
		return;
	}

	if( GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		return;
	}
	m_nOldUnderLineY = GetTextArea().GetAreaTop() + (Int)(GetCaret().GetCaretLayoutPos().GetY2() - GetTextArea().GetViewTopLine()) * GetTextMetrics().GetHankakuDy() + GetTextMetrics().GetHankakuHeight();
	if( -1 == m_nOldUnderLineY ){
		m_nOldUnderLineY = -2;
	}

	if( bDraw
	 && GetDrawSwitch()
	 && m_nOldUnderLineY >=GetTextArea().GetAreaTop()
	 && m_bDoing_UndoRedo == FALSE	/* アンドゥ・リドゥの実行中か */
	){
//		MYTRACE_A( "★カーソル行アンダーラインの描画\n" );
		/* ★カーソル行アンダーラインの描画 */
		HDC		hdc;
		HPEN	hPen, hPenOld;
		hdc = ::GetDC( GetHwnd() );
		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx(
			hdc,
			GetTextArea().GetAreaLeft(),
			m_nOldUnderLineY,
			NULL
		);
		::LineTo(
			hdc,
			GetTextArea().GetAreaRight(),
			m_nOldUnderLineY
		);
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
		::ReleaseDC( GetHwnd(), hdc );
		hdc= NULL;
	}
	return;
}



/* カーソル行アンダーラインのOFF */
void CEditView::CaretUnderLineOFF( bool bDraw )
{
	if( FALSE == m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp ){
		return;
	}

	if( -1 != m_nOldUnderLineY ){
		if( bDraw
		 && GetDrawSwitch()
		 && m_nOldUnderLineY >=GetTextArea().GetAreaTop()
		 && !m_bDoing_UndoRedo	/* アンドゥ・リドゥの実行中か */
		){
			// -- -- カーソル行アンダーラインの消去（無理やり） -- -- //

			GetCaret().m_cUnderLine.Lock();

			PAINTSTRUCT ps;
			ps.rcPaint.left = GetTextArea().GetAreaLeft();
			ps.rcPaint.right = GetTextArea().GetAreaRight();
			ps.rcPaint.top = m_nOldUnderLineY;
			ps.rcPaint.bottom = m_nOldUnderLineY;

			//	不本意ながら選択情報をバックアップ。
			CLayoutRange sSelectBackup = GetSelectionInfo().m_sSelect;
			GetSelectionInfo().m_sSelect.Clear(-1);

			// 描画
			HDC hdc = this->GetDC();
			OnPaint( hdc, &ps, FALSE );
			this->ReleaseDC( hdc );

			//	選択情報を復元
			GetSelectionInfo().m_sSelect = sSelectBackup;
			GetCaret().m_cUnderLine.UnLock();
		}
		m_nOldUnderLineY = -1;
	}
	return;
}


#if 0
/* 現在、Enterなどで挿入する改行コードの種類を取得 */
CEol CEditView::GetCurrentInsertEOL( void )
{
	return m_pcEditDoc->m_cDocEditor.GetNewLineCode();
}
#endif

/*!	@brief	外部コマンドの実行

	@param[in] pszCmd コマンドライン
	@param[in] nFlgOpt オプション
		@li	0x01	標準出力を得る
		@li	0x02	標準出力のりダイレクト先（無効=アウトプットウィンドウ / 有効=編集中のウィンドウ）
		@li	0x04	編集中ファイルを標準入力へ

	@note	子プロセスの標準出力取得はパイプを使用する
	@note	子プロセスの標準入力への送信は一時ファイルを使用

	@author	N.Nakatani
	@date	2001/06/23
	@date	2001/06/30	GAE
	@date	2002/01/24	YAZAKI	1バイト取りこぼす可能性があった
	@date	2003/06/04	genta
	@date	2004/09/20	naoh	多少は見やすく・・・
	@date	2004/01/23	genta
	@date	2004/01/28	Moca	改行コードが分割されるのを防ぐ
	@date	2007/03/18	maru	オプションの拡張
*/
void CEditView::ExecCmd( const TCHAR* pszCmd, int nFlgOpt )
{
	HANDLE				hStdOutWrite, hStdOutRead, hStdIn;
	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(pi) );
	CDlgCancel				cDlgCancel;

	//	From Here 2006.12.03 maru 引数を拡張のため
	BOOL	bGetStdout		= nFlgOpt & 0x01 ? TRUE : FALSE;	//	子プロセスの標準出力を得る
	BOOL	bToEditWindow	= nFlgOpt & 0x02 ? TRUE : FALSE;	//	TRUE=編集中のウィンドウ / FALSAE=アウトプットウィンドウ
	BOOL	bSendStdin		= nFlgOpt & 0x04 ? TRUE : FALSE;	//	編集中ファイルを子プロセスSTDINに渡す
	//	To Here 2006.12.03 maru 引数を拡張のため

	// 編集中のウィンドウに出力する場合の選択範囲処理用	/* 2007.04.29 maru */
	CLayoutInt	nLineFrom, nColmFrom;
	bool bBeforeTextSelected = GetSelectionInfo().IsTextSelected();
	if (bBeforeTextSelected){
		nLineFrom = this->GetSelectionInfo().m_sSelect.GetFrom().y; //m_nSelectLineFrom;
		nColmFrom = this->GetSelectionInfo().m_sSelect.GetFrom().x; //m_nSelectColmFrom;
	}

	//子プロセスの標準出力と接続するパイプを作成
	SECURITY_ATTRIBUTES	sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	if( CreatePipe( &hStdOutRead, &hStdOutWrite, &sa, 1000 ) == FALSE ) {
		//エラー。対策無し
		return;
	}
	//hStdOutReadのほうは子プロセスでは使用されないので継承不能にする（子プロセスのリソースを無駄に増やさない）
	DuplicateHandle( GetCurrentProcess(), hStdOutRead,
				GetCurrentProcess(), &hStdOutRead,					// 新しい継承不能ハンドルを受け取る	// 2007.01.31 ryoji
				0, FALSE,
				DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS );	// 元の継承可能ハンドルは DUPLICATE_CLOSE_SOURCE で閉じる	// 2007.01.31 ryoji


	// From Here 2007.03.18 maru 子プロセスの標準入力ハンドル
	// CDocLineMgr::WriteFileなど既存のファイル出力系の関数のなかには
	// ファイルハンドルを返すタイプのものがないので、一旦書き出してから
	// 一時ファイル属性でオープンすることに。
	hStdIn = NULL;
	if(bSendStdin){	/* 現在編集中のファイルを子プロセスの標準入力へ */
		TCHAR		szPathName[MAX_PATH];
		TCHAR		szTempFileName[MAX_PATH];
		int			nFlgOpt;

		GetTempPath( MAX_PATH, szPathName );
		GetTempFileName( szPathName, TEXT("skr_"), 0, szTempFileName );
		DBPRINT( _T("CEditView::ExecCmd() TempFilename=[%ts]\n"), szTempFileName );
		
		nFlgOpt = bBeforeTextSelected ? 0x01 : 0x00;		/* 選択範囲を出力 */
		
		if( !GetCommander().Command_PUTFILE( to_wchar(szTempFileName), CODE_SJIS, nFlgOpt) ){	// 一時ファイル出力
			hStdIn = NULL;
		} else {
			// 子プロセスへの継承用にファイルを開く
			hStdIn = CreateFile(
				szTempFileName,
				GENERIC_READ,
				0,
				&sa,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
				NULL
			);
			if(hStdIn == INVALID_HANDLE_VALUE) hStdIn = NULL;
		}
	}
	
	if (hStdIn == NULL) {	/* 標準入力を制御しない場合、または一時ファイルの生成に失敗した場合 */
		bSendStdin = FALSE;
		hStdIn = GetStdHandle( STD_INPUT_HANDLE );
	}
	// To Here 2007.03.18 maru 子プロセスの標準入力ハンドル
	

	//CreateProcessに渡すSTARTUPINFOを作成
	STARTUPINFO	sui;
	ZeroMemory( &sui, sizeof(sui) );
	sui.cb = sizeof(sui);
	if( bGetStdout || bSendStdin ) {
		sui.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		sui.wShowWindow = bGetStdout ? SW_HIDE : SW_SHOW;
		sui.hStdInput = hStdIn;
		sui.hStdOutput = bGetStdout ? hStdOutWrite : GetStdHandle( STD_OUTPUT_HANDLE );
		sui.hStdError = bGetStdout ? hStdOutWrite : GetStdHandle( STD_ERROR_HANDLE );
	}

	//コマンドライン実行
	TCHAR	cmdline[1024];
	_tcscpy( cmdline, pszCmd );
	if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
				CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE ) {
		//実行に失敗した場合、コマンドラインベースのアプリケーションと判断して
		// command(9x) か cmd(NT) を呼び出す

		//OSバージョン取得
		COsVersionInfo cOsVer;
		//コマンドライン文字列作成
		auto_sprintf(
			cmdline,
			_T("%ls %ls%ts"),
			( cOsVer.IsWin32NT() ? L"cmd.exe" : L"command.com" ),
			( bGetStdout ? L"/C " : L"/K " ),
			pszCmd
		);
		if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
					CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE ) {
			MessageBox( NULL, cmdline, _T("コマンド実行は失敗しました。"), MB_OK | MB_ICONEXCLAMATION );
			goto finish;
		}
	}

	// ファイル全体に対するフィルタ動作
	//	現在編集中のファイルからのデータ書きだしおよびデータ取り込みが
	//	指定されていて，かつ範囲選択が行われていない場合は
	//	「すべて選択」されているものとして，編集データ全体を
	//	コマンドの出力結果と置き換える．
	//	2007.05.20 maru
	if(!bBeforeTextSelected && bSendStdin && bGetStdout && bToEditWindow){
		GetSelectionInfo().SetSelectArea(
			CLayoutRange(
				CLayoutPoint(CLayoutInt(0), CLayoutInt(0)),
				CLayoutPoint(CLayoutInt(0), m_pcEditDoc->m_cLayoutMgr.GetLineCount())
			)
		);
		DeleteData( TRUE );
	}

	// hStdOutWrite は CreateProcess() で継承したので親プロセスでは用済み
	// hStdInも親プロセスでは使用しないが、Win9x系では子プロセスが終了してから
	// クローズするようにしないと一時ファイルが自動削除されない
	CloseHandle(hStdOutWrite);
	hStdOutWrite = NULL;	// 2007.09.08 genta 二重closeを防ぐ

	if( bGetStdout ) {
		DWORD	read_cnt;
		DWORD	new_cnt;
		int		bufidx = 0;
		int		j;
		BOOL	bLoopFlag = TRUE;

		//中断ダイアログ表示
		cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_EXECRUNNING );
		//実行したコマンドラインを表示
		// 2004.09.20 naoh 多少は見やすく・・・
		if (FALSE==bToEditWindow)	//	2006.12.03 maru アウトプットウィンドウにのみ出力
		{
			TCHAR szTextDate[1024], szTextTime[1024];
			SYSTEMTIME systime;
			::GetLocalTime( &systime );
			CShareData::getInstance()->MyGetDateFormat( systime, szTextDate, _countof( szTextDate ) - 1 );
			CShareData::getInstance()->MyGetTimeFormat( systime, szTextTime, _countof( szTextTime ) - 1 );
			CShareData::getInstance()->TraceOut( _T("\r\n%ls\r\n"), _T("#============================================================") );
			CShareData::getInstance()->TraceOut( _T("#DateTime : %ls %ls\r\n"), szTextDate, szTextTime );
			CShareData::getInstance()->TraceOut( _T("#CmdLine  : %ls\r\n"), pszCmd );
			CShareData::getInstance()->TraceOut( _T("#%ls\r\n"), _T("==============================") );
		}
		
		//charで読む
		typedef char PIPE_CHAR;
		PIPE_CHAR work[1024];
		
		//実行結果の取り込み
		do {
			//プロセスが終了していないか確認
			// Jun. 04, 2003 genta CPU消費を減らすために200msec待つ
			// その間メッセージ処理が滞らないように待ち方をWaitForSingleObjectから
			// MsgWaitForMultipleObjectに変更
			// Jan. 23, 2004 genta
			// 子プロセスの出力をどんどん受け取らないと子プロセスが
			// 停止してしまうため，待ち時間を200msから20msに減らす
			switch( MsgWaitForMultipleObjects( 1, &pi.hProcess, FALSE, 20, QS_ALLEVENTS )){
				case WAIT_OBJECT_0:
					//終了していればループフラグをFALSEとする
					//ただしループの終了条件は プロセス終了 && パイプが空
					bLoopFlag = FALSE;
					break;
				case WAIT_OBJECT_0 + 1:
					//処理中のユーザー操作を可能にする
					if( !::BlockingHook( cDlgCancel.GetHwnd() ) ){
						break;
					}
					break;
				default:
					break;
			}
			//中断ボタン押下チェック
			if( cDlgCancel.IsCanceled() ){
				//指定されたプロセスと、そのプロセスが持つすべてのスレッドを終了させます。
				::TerminateProcess( pi.hProcess, 0 );
				if (!bToEditWindow) {	//	2006.12.03 maru アウトプットウィンドウにのみ出力
					//最後にテキストを追加
					CShareData::getInstance()->TraceOut( _T("%ts"), _T("\r\n中断しました。\r\n") );
				}
				break;
			}
			new_cnt = 0;

			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ) {	//パイプの中の読み出し待機中の文字数を取得
				while( new_cnt > 0 ) {												//待機中のものがある

					if( new_cnt >= _countof(work)-2 ) {							//パイプから読み出す量を調整
						new_cnt = _countof(work)-2;
					}
					::ReadFile( hStdOutRead, &work[bufidx], new_cnt, &read_cnt, NULL );	//パイプから読み出し
					read_cnt += bufidx;													//work内の実際のサイズにする

					if( read_cnt == 0 ) {
						// Jan. 23, 2004 genta while追加のため制御を変更
						break;
					}
					//読み出した文字列をチェックする
					// \r\n を \r だけとか漢字の第一バイトだけを出力するのを防ぐ必要がある
					//@@@ 2002.1.24 YAZAKI 1バイト取りこぼす可能性があった。
					//	Jan. 28, 2004 Moca 最後の文字はあとでチェックする
					for( j=0; j<(int)read_cnt - 1; j++ ) {
						//	2007.09.10 ryoji
						if( CNativeA::GetSizeOfChar(work, read_cnt, j) == 2 ) {
							j++;
						} else {
							if( work[j] == _T2(PIPE_CHAR,'\r') && work[j+1] == _T2(PIPE_CHAR,'\n') ) {
								j++;
							} else if( work[j] == _T2(PIPE_CHAR,'\n') && work[j+1] == _T2(PIPE_CHAR,'\r') ) {
								j++;
							}
						}
					}
					//	From Here Jan. 28, 2004 Moca
					//	改行コードが分割されるのを防ぐ
					if( j == read_cnt - 1 ){
						if( _IS_SJIS_1(work[j]) ) {
							j = read_cnt + 1; // ぴったり出力できないことを主張
						}else if( work[j] == _T2(PIPE_CHAR,'\r') || work[j] == _T2(PIPE_CHAR,'\n') ) {
							// CRLFの一部ではない改行が末尾にある
							// 次の読み込みで、CRLFの一部になる可能性がある
							j = read_cnt + 1;
						}else{
							j = read_cnt;
						}
					}
					//	To Here Jan. 28, 2004 Moca
					if( j == (int)read_cnt ) {	//ぴったり出力できる場合
						//	2006.12.03 maru アウトプットウィンドウor編集中のウィンドウ分岐追加
						if (FALSE==bToEditWindow) {
							work[read_cnt] = '\0';
							CShareData::getInstance()->TraceOut( _T("%hs"), work );
						} else {
							GetCommander().Command_INSTEXT(FALSE, to_wchar(work,read_cnt), CLogicInt(-1), TRUE);
						}
						bufidx = 0;
					}
					else {
						char tmp = work[read_cnt-1];
						//	2006.12.03 maru アウトプットウィンドウor編集中のウィンドウ分岐追加
						if (FALSE==bToEditWindow) {
							work[read_cnt-1] = '\0';
							CShareData::getInstance()->TraceOut( _T("%hs"), work );
						} else {
							GetCommander().Command_INSTEXT(FALSE, to_wchar(work,read_cnt-1), CLogicInt(-1), TRUE);
						}
						work[0] = tmp;
						bufidx = 1;
						DBPRINT_A( "ExecCmd: Carry last character [%d]\n", tmp );
					}
					// Jan. 23, 2004 genta
					// 子プロセスの出力をどんどん受け取らないと子プロセスが
					// 停止してしまうため，バッファが空になるまでどんどん読み出す．
					new_cnt = 0;
					if( ! PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ){
						break;
					}
					Sleep(0);
				}
			}
		} while( bLoopFlag || new_cnt > 0 );
		
		if (!bToEditWindow) {	//	2006.12.03 maru アウトプットウィンドウにのみ出力
			work[bufidx] = '\0';
			CShareData::getInstance()->TraceOut( _T("%hs"), work );	/* 最後の文字の処理 */
			//	Jun. 04, 2003 genta	終了コードの取得と出力
			DWORD result;
			::GetExitCodeProcess( pi.hProcess, &result );
			CShareData::getInstance()->TraceOut( _T("\r\n終了コード: %d\r\n"), result );

			// 2004.09.20 naoh 終了コードが1以上の時はアウトプットをアクティブにする
			if(result > 0) ActivateFrameWindow( m_pShareData->m_hwndDebug );
		}
		else {						//	2006.12.03 maru 編集中のウィンドウに出力時は最後に再描画
			GetCommander().Command_INSTEXT(FALSE, to_wchar(work,bufidx), CLogicInt(-1), TRUE);	/* 最後の文字の処理 */
			if (bBeforeTextSelected){	// 挿入された部分を選択状態に
				GetSelectionInfo().SetSelectArea(
					CLayoutRange(
						CLayoutPoint(nColmFrom, nLineFrom),
						GetCaret().GetCaretLayoutPos()// CLayoutPoint(m_nCaretPosY, m_nCaretPosX )
					)
				);
				GetSelectionInfo().DrawSelectArea();
			}
			RedrawAll();
		}
	}


finish:
	//終了処理
	if(bSendStdin) CloseHandle( hStdIn );	/* 2007.03.18 maru 標準入力の制御のため */
	if(hStdOutWrite) CloseHandle( hStdOutWrite );
	CloseHandle( hStdOutRead );
	if( pi.hProcess ) CloseHandle( pi.hProcess );
	if( pi.hThread ) CloseHandle( pi.hThread );
}

/*!
	検索／置換／ブックマーク検索時の状態をステータスバーに表示する

	@date 2002.01.26 hor 新規作成
	@date 2002.12.04 genta 実体をCEditWndへ移動
*/
void CEditView::SendStatusMessage( const TCHAR* msg )
{
	m_pcEditDoc->m_pcEditWnd->SendStatusMessage( msg );
}



//  2002.04.09 minfu from here
/*再変換用 カーソル位置から前後200byteを取り出してRECONVERTSTRINGを埋める */
/*  引数  pReconv RECONVERTSTRING構造体へのポインタ。                     */
/*        bUnicode trueならばUNICODEで構造体を埋める                      */
/*  戻り値   RECONVERTSTRINGのサイズ                                      */
LRESULT CEditView::SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode)
{
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;
	
	//矩形選択中は何もしない
	if( GetSelectionInfo().IsBoxSelecting() )
		return 0;
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      選択範囲を取得                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//選択範囲を取得 -> ptSelect, ptSelectTo, nSelectedLen
	CLogicPoint	ptSelect;
	CLogicPoint	ptSelectTo;
	int			nSelectedLen;
	if( GetSelectionInfo().IsTextSelected() ){
		//テキストが選択されているとき
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(GetSelectionInfo().m_sSelect.GetFrom(), &ptSelect);
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(GetSelectionInfo().m_sSelect.GetTo(), &ptSelectTo);
		
		//選択範囲が複数行の時は
		if (ptSelectTo.y != ptSelect.y){
			//行末までに制限
			CDocLine* pDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(ptSelect.GetY2());
			ptSelectTo.x = pDocLine->GetLengthWithEOL();
		}
	}else{
		//テキストが選択されていないとき
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos(), &ptSelect);
		ptSelectTo = ptSelect;
	}
	nSelectedLen = ptSelectTo.x - ptSelect.x;

	//ドキュメント行取得 -> pcCurDocLine
	CDocLine* pcCurDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(ptSelect.GetY2());
	if (NULL == pcCurDocLine )
		return 0;

	//テキスト取得 -> pLine, nLineLen
	int nLineLen = pcCurDocLine->GetLengthWithEOL() - pcCurDocLine->GetEol().GetLen() ; //改行コードをのぞいた長さ
	if ( 0 == nLineLen )
		return 0;
	const wchar_t* pLine = pcCurDocLine->GetPtr();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      選択範囲を修正                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//再変換考慮文字列開始  //行の中で再変換のAPIにわたすとする文字列の開始位置
	int nReconvIndex = 0;
	if ( ptSelect.x > 200 ) { //$$マジックナンバー注意
		const wchar_t* pszWork = pLine;
		while( (ptSelect.x - nReconvIndex) > 200 ){
			pszWork = ::CharNextW_AnyBuild( pszWork);
			nReconvIndex = pszWork - pLine ;
		}
	}
	
	//再変換考慮文字列終了  //行の中で再変換のAPIにわたすとする文字列の長さ
	int nReconvLen = nLineLen - nReconvIndex;
	if ( (nReconvLen + nReconvIndex - ptSelect.x) > 200 ){
		const wchar_t* pszWork = pLine + ptSelect.x;
		nReconvLen = ptSelect.x - nReconvIndex;
		while( ( nReconvLen + nReconvIndex - ptSelect.x) <= 200 ){
			pszWork = ::CharNextW_AnyBuild( pszWork);
			nReconvLen = pszWork - (pLine + nReconvIndex) ;
		}
	}
	
	//対象文字列の調整
	if ( ptSelect.x + nSelectedLen > nReconvIndex + nReconvLen ){
		nSelectedLen = nReconvIndex + nReconvLen - ptSelect.x;
	}
	

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      構造体設定要素                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//行の中で再変換のAPIにわたすとする文字列の長さ
	int			nReconvLenWithNull;
	DWORD		dwReconvTextLen;
	DWORD		dwCompStrOffset, dwCompStrLen;
	CNativeW	cmemBuf1;
	const void*	pszReconv;

	//UNICODE→UNICODE
	if(bUnicode){
		dwReconvTextLen    = nReconvLen;											//reconv文字列長。文字単位。
		nReconvLenWithNull = (nReconvLen + 1) * sizeof(wchar_t);					//reconvデータ長。バイト単位。
		dwCompStrOffset    = (Int)(ptSelect.x - nReconvIndex) * sizeof(wchar_t);	//compオフセット。バイト単位。
		dwCompStrLen       = nSelectedLen;											//comp文字列長。文字単位。
		pszReconv          = reinterpret_cast<const void*>(pLine + nReconvIndex);	//reconv文字列へのポインタ。
	}
	//UNICODE→ANSI
	else{
		const wchar_t* pszReconvSrc =  pLine + nReconvIndex;

		//考慮文字列の開始から対象文字列の開始まで -> dwCompStrOffset
		if( ptSelect.x - nReconvIndex > 0 ){
			cmemBuf1.SetString(pszReconvSrc, ptSelect.x - nReconvIndex);
			CShiftJis::UnicodeToSJIS(cmemBuf1._GetMemory());
			dwCompStrOffset = cmemBuf1._GetMemory()->GetRawLength();				//compオフセット。バイト単位。
		}else{
			dwCompStrOffset = 0;
		}
		
		//対象文字列の開始から対象文字列の終了まで -> dwCompStrLen
		if (nSelectedLen > 0 ){
			cmemBuf1.SetString(pszReconvSrc + ptSelect.x, nSelectedLen);  
			CShiftJis::UnicodeToSJIS(cmemBuf1._GetMemory());
			dwCompStrLen = cmemBuf1._GetMemory()->GetRawLength();					//comp文字列長。文字単位。
		}else{
			dwCompStrLen = 0;
		}
		
		//考慮文字列すべて
		cmemBuf1.SetString(pszReconvSrc , nReconvLen );
		CShiftJis::UnicodeToSJIS(cmemBuf1._GetMemory());
		
		dwReconvTextLen =  cmemBuf1._GetMemory()->GetRawLength();						//reconv文字列長。文字単位。
		nReconvLenWithNull =  cmemBuf1._GetMemory()->GetRawLength() + sizeof(char);		//reconvデータ長。バイト単位。
		
		pszReconv = reinterpret_cast<const void*>(cmemBuf1._GetMemory()->GetRawPtr());	//reconv文字列へのポインタ
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        構造体設定                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if ( NULL != pReconv) {
		//再変換構造体の設定
		pReconv->dwSize            = sizeof(*pReconv) + nReconvLenWithNull ;
		pReconv->dwVersion         = 0;
		pReconv->dwStrLen          = dwReconvTextLen ;	//文字単位
		pReconv->dwStrOffset       = sizeof(*pReconv) ;
		pReconv->dwCompStrLen      = dwCompStrLen;		//文字単位
		pReconv->dwCompStrOffset   = dwCompStrOffset;	//バイト単位
		pReconv->dwTargetStrLen    = dwCompStrLen;		//文字単位
		pReconv->dwTargetStrOffset = dwCompStrOffset;	//バイト単位
		
		// 2004.01.28 Moca ヌル終端の修正
		if( bUnicode ){
			WCHAR* p = (WCHAR*)(pReconv + 1);
			CopyMemory(p, pszReconv, nReconvLenWithNull - sizeof(wchar_t));
			p[dwReconvTextLen] = L'\0';
		}else{
			ACHAR* p = (ACHAR*)(pReconv + 1);
			CopyMemory(p, pszReconv, nReconvLenWithNull - sizeof(char));
			p[dwReconvTextLen]='\0';
		}
	}
	
	// 再変換情報の保存
	m_nLastReconvIndex = nReconvIndex;
	m_nLastReconvLine  = ptSelect.y;
	
	return sizeof(RECONVERTSTRING) + nReconvLenWithNull;

}

/*再変換用 エディタ上の選択範囲を変更する 2002.04.09 minfu */
LRESULT CEditView::SetSelectionFromReonvert(const PRECONVERTSTRING pReconv, bool bUnicode){
	
	// 再変換情報が保存されているか
	if ( (m_nLastReconvIndex < 0) || (m_nLastReconvLine < 0))
		return 0;

	if ( GetSelectionInfo().IsTextSelected()) 
		GetSelectionInfo().DisableSelectArea( TRUE );

	DWORD dwOffset, dwLen;

	//UNICODE→UNICODE
	if(bUnicode){
		dwOffset = pReconv->dwCompStrOffset/sizeof(WCHAR);	//0またはデータ長。バイト単位。→文字単位
		dwLen    = pReconv->dwCompStrLen;					//0または文字列長。文字単位。
	}
	//ANSI→UNICODE
	else{
		CNativeA	cmemBuf;

		//考慮文字列の開始から対象文字列の開始まで
		if( pReconv->dwCompStrOffset > 0){
			const char* p=(const char*)(pReconv+1);
			cmemBuf.SetString(p, pReconv->dwCompStrOffset ); 
			CShiftJis::SJISToUnicode(cmemBuf._GetMemory());
			dwOffset = cmemBuf._GetMemory()->GetRawLength()/sizeof(WCHAR);
		}else{
			dwOffset = 0;
		}

		//対象文字列の開始から対象文字列の終了まで
		if( pReconv->dwCompStrLen > 0 ){
			const char* p=(const char*)(pReconv+1);
			cmemBuf.SetString(p + pReconv->dwCompStrOffset, pReconv->dwCompStrLen); 
			CShiftJis::SJISToUnicode(cmemBuf._GetMemory());
			dwLen = cmemBuf._GetMemory()->GetRawLength()/sizeof(WCHAR);
		}else{
			dwLen = 0;
		}
	}
	
	//選択開始の位置を取得
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		CLogicPoint(m_nLastReconvIndex + dwOffset, m_nLastReconvLine),
		GetSelectionInfo().m_sSelect.GetFromPointer()
	);

	//選択終了の位置を取得
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		CLogicPoint(m_nLastReconvIndex + dwOffset + dwLen, m_nLastReconvLine),
		GetSelectionInfo().m_sSelect.GetToPointer()
	);

	// 単語の先頭にカーソルを移動
	GetCaret().MoveCursor( GetSelectionInfo().m_sSelect.GetFrom(), TRUE );

	//選択範囲再描画 
	GetSelectionInfo().DrawSelectArea();

	// 再変換情報の破棄
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;

	return 1;

}



/*!
	@date 2003/02/18 ai
	@param flag [in] モード(true:登録, false:解除)
*/
void CEditView::SetBracketPairPos( bool flag )
{
	int	mode;

	// 03/03/06 ai すべて置換、すべて置換後のUndo&Redoがかなり遅い問題に対応
	if( m_bDoing_UndoRedo || !GetDrawSwitch() ){
		return;
	}

	if( !m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// 対括弧の検索&登録
	/*
	bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
	bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
	bit2(out) : 見つかった位置         0:後ろ      1:前
	*/
	mode = 2;

	CLayoutPoint ptColLine;

	if( flag && !GetSelectionInfo().IsTextSelected() && !GetSelectionInfo().m_bDrawSelectArea
		&& !GetSelectionInfo().IsBoxSelecting() && SearchBracket( GetCaret().GetCaretLayoutPos(), &ptColLine, &mode ) )
	{
		// 登録指定(flag=true)			&&
		// テキストが選択されていない	&&
		// 選択範囲を描画していない		&&
		// 矩形範囲選択中でない			&&
		// 対応する括弧が見つかった		場合
		if ( ( ptColLine.x >= GetTextArea().GetViewLeftCol() ) && ( ptColLine.x <= GetTextArea().GetRightCol() )
			&& ( ptColLine.y >= GetTextArea().GetViewTopLine() ) && ( ptColLine.y <= GetTextArea().GetBottomLine() ) )
		{
			// 表示領域内の場合

			// レイアウト位置から物理位置へ変換(強調表示位置を登録)
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( ptColLine, &m_ptBracketPairPos_PHY );
			m_ptBracketCaretPos_PHY.y = GetCaret().GetCaretLogicPos().y;
			if( 0 == ( mode & 4 ) ){
				// カーソルの後方文字位置
				m_ptBracketCaretPos_PHY.x = GetCaret().GetCaretLogicPos().x;
			}else{
				// カーソルの前方文字位置
				m_ptBracketCaretPos_PHY.x = GetCaret().GetCaretLogicPos().x - 1;
			}
			return;
		}
	}

	// 括弧の強調表示位置情報初期化
	m_ptBracketPairPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));
	m_ptBracketCaretPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));

	return;
}

/*!
	対括弧の強調表示
	@date 2002/09/18 ai
	@date 2003/02/18 ai 再描画対応の為大改造
*/
void CEditView::DrawBracketPair( bool bDraw )
{
	// 03/03/06 ai すべて置換、すべて置換後のUndo&Redoがかなり遅い問題に対応
	if( m_bDoing_UndoRedo || !GetDrawSwitch() ){
		return;
	}

	if( !m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// 括弧の強調表示位置が未登録の場合は終了
	if( m_ptBracketPairPos_PHY.HasNegative() || m_ptBracketCaretPos_PHY.HasNegative() ){
		return;
	}

	// 描画指定(bDraw=true)				かつ
	// ( テキストが選択されている		又は
	//   選択範囲を描画している			又は
	//   矩形範囲選択中					又は
	//   フォーカスを持っていない		又は
	//   アクティブなペインではない )	場合は終了
	if( bDraw
	 &&( GetSelectionInfo().IsTextSelected() || GetSelectionInfo().m_bDrawSelectArea || GetSelectionInfo().IsBoxSelecting() || !m_bDrawBracketPairFlag
	 || ( m_pcEditDoc->m_pcEditWnd->m_nActivePaneIndex != m_nMyIndex ) ) ){
		return;
	}

	HDC			hdc;
	hdc = ::GetDC( GetHwnd() );
	Types *TypeDataPtr = &( m_pcEditDoc->m_cDocType.GetDocumentAttribute() );

	for( int i = 0; i < 2; i++ )
	{
		// i=0:カーソル位置の括弧,i=1:対括弧

		CLayoutPoint	ptColLine;

		if( i == 0 ){
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout( m_ptBracketCaretPos_PHY, &ptColLine );
		}else{
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout( m_ptBracketPairPos_PHY,  &ptColLine );
		}

		if ( ( ptColLine.x >= GetTextArea().GetViewLeftCol() ) && ( ptColLine.x <= GetTextArea().GetRightCol() )
			&& ( ptColLine.y >= GetTextArea().GetViewTopLine() ) && ( ptColLine.y <= GetTextArea().GetBottomLine() ) )
		{	// 表示領域内の場合
			if( !bDraw && GetSelectionInfo().m_bDrawSelectArea && ( 0 == IsCurrentPositionSelected( ptColLine ) ) )
			{	// 選択範囲描画済みで消去対象の括弧が選択範囲内の場合
				continue;
			}
			const CLayout* pcLayout;
			CLogicInt		nLineLen;
			const wchar_t*	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( ptColLine.GetY2(), &nLineLen, &pcLayout );
			if( pLine )
			{
				int		nColorIndex;
				CLogicInt	OutputX = LineColmnToIndex( pcLayout, ptColLine.GetX2() );
				if( bDraw )	{
					nColorIndex = COLORIDX_BRACKET_PAIR;
				}
				else{
					if( IsBracket( pLine, OutputX, CLogicInt(1) ) ){
						// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応
						if( i == 0 ){
							nColorIndex = GetColorIndex( hdc, pcLayout, m_ptBracketCaretPos_PHY.x );
						}else{
							nColorIndex = GetColorIndex( hdc, pcLayout, m_ptBracketPairPos_PHY.x );
						}
					}
					else{
						SetBracketPairPos( false );
						break;
					}
				}


				m_hFontOld = NULL;

				//色設定
				CTypeSupport cTextType(this,COLORIDX_TEXT);
				cTextType.SetFont(hdc);
				cTextType.SetColors(hdc);

				SetCurrentColor( hdc, nColorIndex );

				int nHeight = GetTextMetrics().GetHankakuDy();
				int nLeft = (GetTextArea().GetDocumentLeftClientPointX()) + (Int)ptColLine.x * GetTextMetrics().GetHankakuDx();
				int nTop  = (Int)( ptColLine.GetY2() - GetTextArea().GetViewTopLine() ) * nHeight + GetTextArea().GetAreaTop();

				// 03/03/03 ai カーソルの左に括弧があり括弧が強調表示されている状態でShift+←で選択開始すると
				//             選択範囲内に反転表示されない部分がある問題の修正
				if( ptColLine.x == GetCaret().GetCaretLayoutPos().GetX2() && GetCaret().GetCaretShowFlag() ){
					GetCaret().HideCaret_( GetHwnd() );	// キャレットが一瞬消えるのを防止
					GetTextDrawer().DispText( hdc, nLeft, nTop, &pLine[OutputX], 1 );
					// 2006.04.30 Moca 対括弧の縦線対応
					GetTextDrawer().DispVerticalLines( hdc, nTop, nTop + nHeight, ptColLine.x, ptColLine.x + CLayoutInt(2) ); //※括弧が全角幅である場合を考慮
					GetCaret().ShowCaret_( GetHwnd() );	// キャレットが一瞬消えるのを防止
				}
				else{
					GetTextDrawer().DispText( hdc, nLeft, nTop, &pLine[OutputX], 1 );
					// 2006.04.30 Moca 対括弧の縦線対応
					GetTextDrawer().DispVerticalLines( hdc, nTop, nTop + nHeight, ptColLine.x, ptColLine.x + CLayoutInt(2) ); //※括弧が全角幅である場合を考慮
				}

				if( NULL != m_hFontOld ){
					::SelectObject( hdc, m_hFontOld );
					m_hFontOld = NULL;
				}

				cTextType.RewindFont(hdc);
				cTextType.RewindColors(hdc);

				if( ( m_pcEditDoc->m_pcEditWnd->m_nActivePaneIndex == m_nMyIndex )
					&& ( ( ptColLine.y == GetCaret().GetCaretLayoutPos().GetY() ) || ( ptColLine.y - 1 == GetCaret().GetCaretLayoutPos().GetY() ) ) ){	// 03/02/27 ai 行の間隔が"0"の時にアンダーラインが欠ける事がある為修正
					GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
				}
			}
		}
	}

	::ReleaseDC( GetHwnd(), hdc );

	return;
}

/*! 指定位置のColorIndexの取得
	CEditView::DispLineNewを元にしたためCEditView::DispLineNewに
	修正があった場合は、ここも修正が必要。

	@par nCOMMENTMODE
	関数内部で状態遷移のために使われる変数nCOMMENTMODEと状態の関係。
 - COLORIDX_TEXT     : テキスト
 - COLORIDX_COMMENT  : 行コメント
 - COLORIDX_BLOCK1   : ブロックコメント1
 - COLORIDX_SSTRING  : シングルコーテーション
 - COLORIDX_WSTRING  : ダブルコーテーション
 - COLORIDX_KEYWORD1 : 強調キーワード1
 - COLORIDX_CTRLCODE : コントロールコード
 - COLORIDX_DIGIT    : 半角数値
 - COLORIDX_BLOCK2   : ブロックコメント2
 - COLORIDX_KEYWORD2 : 強調キーワード2
 - COLORIDX_URL      : URL
 - COLORIDX_SEARCH   : 検索
 - 1000: 正規表現キーワード
 	色指定SetCurrentColorを呼ぶときにCOLORIDX_*値を加算するので、
 	1000～COLORIDX_LASTまでは正規表現で使用する。
*/
int CEditView::GetColorIndex(
		HDC						hdc,
		const CLayout*			pcLayout,
		int						nCol
)
{
	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->m_cDocType.GetDocumentAttribute());

	const wchar_t*			pLine;	//@@@ 2002.09.22 YAZAKI
	CLogicInt				nLineLen;
	int						nLineBgn;
	int						nCOMMENTMODE;
	int						nCOMMENTMODE_OLD;
	int						nCOMMENTEND;
	int						nCOMMENTEND_OLD;
	const CLayout*			pcLayout2;
	int						i, j;
	int						nIdx;
	int						nUrlLen;
	BOOL					bSearchStringMode;
	bool					bSearchFlg;			// 2002.02.08 hor
	bool					bKeyWordTop = true;	//	Keyword Top
	int						nColorIndex;

//@@@ 2001.11.17 add start MIK
	int		nMatchLen;
	int		nMatchColor;
//@@@ 2001.11.17 add end MIK

	bSearchStringMode = FALSE;
	bSearchFlg	= true;	// 2002.02.08 hor

	CLogicInt	nSearchStart = CLogicInt(-1);
	CLogicInt	nSearchEnd   = CLogicInt(-1);

	/* 論理行データの取得 */
	if( NULL != pcLayout ){
		// 2002/2/10 aroka CMemory変更
		nLineLen = pcLayout->GetDocLineRef()->GetLengthWithEOL()/* - pcLayout->GetLogicOffset()*/;	// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応
		pLine = pcLayout->GetPtr()/* + pcLayout->GetLogicOffset()*/;			// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応

		// 2005.11.20 Moca 色が正しくないことがある問題に対処
		const CLayout* pcLayoutLineFirst = pcLayout;
		// 論理行の最初のレイアウト情報を取得する
		while( 0 != pcLayoutLineFirst->GetLogicOffset() ){
			pcLayoutLineFirst = pcLayoutLineFirst->GetPrevLayout();
		}
		nCOMMENTMODE = pcLayoutLineFirst->GetColorTypePrev();
		nCOMMENTEND = 0;
		pcLayout2 = pcLayout;

	}else{
		pLine = NULL;
		nLineLen = CLogicInt(0);
		nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		nCOMMENTEND = 0;
		pcLayout2 = NULL;
	}

	/* 現在の色を指定 */
	//@SetCurrentColor( hdc, nCOMMENTMODE );
	nColorIndex = nCOMMENTMODE;	// 02/12/18 ai

	int						nBgn;
	CLogicInt				nPos;
	nBgn = 0;
	nPos = CLogicInt(0);
	nLineBgn = 0;

	CLogicInt				nCharChars = CLogicInt(0);
	CLogicInt				nCharChars_2;

	if( NULL != pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		while( nPos <= nCol ){	// 03/10/24 ai 行頭のColorIndexが取得できない問題に対応

			nBgn = nPos;
			nLineBgn = nBgn;

			while( nPos - nLineBgn <= nCol ){	// 02/12/18 ai
				/* 検索文字列の色分け */
				if( m_bCurSrchKeyMark	/* 検索文字列のマーク */
				 && TypeDataPtr->m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
searchnext:;
				// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
					if(!bSearchStringMode && (!m_sCurSearchOption.bRegularExp || (bSearchFlg && nSearchStart < nPos))){
						bSearchFlg=IsSearchString( pLine, nLineLen, nPos, &nSearchStart, &nSearchEnd );
					}
					if( !bSearchStringMode && bSearchFlg && nSearchStart==nPos ){
						nBgn = nPos;
						bSearchStringMode = TRUE;
						/* 現在の色を指定 */
						//@SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
						nColorIndex = COLORIDX_SEARCH;	// 02/12/18 ai
					}
					else if( bSearchStringMode && nSearchEnd == nPos ){
						nBgn = nPos;
						/* 現在の色を指定 */
						//@SetCurrentColor( hdc, nCOMMENTMODE );
						nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						bSearchStringMode = FALSE;
						goto searchnext;
					}
				}

				if( nPos >= nLineLen - pcLayout2->GetLayoutEol().GetLen() ){
					goto end_of_line;
				}
				SEARCH_START:;
				switch( nCOMMENTMODE ){
				case COLORIDX_TEXT: // 2002/03/13 novice
//@@@ 2001.11.17 add start MIK
					//正規表現キーワード
					if( TypeDataPtr->m_bUseRegexKeyword
					 && m_cRegexKeyword->RegexIsKeyword( pLine, nPos, nLineLen, &nMatchLen, &nMatchColor )
					 /*&& TypeDataPtr->m_ColorInfoArr[nMatchColor].m_bDisp*/ )
					{
						/* 現在の色を指定 */
						nBgn = nPos;
						nCOMMENTMODE = MakeColorIndexType_RegularExpression(nMatchColor);	/* 色指定 */	//@@@ 2002.01.04 upd
						nCOMMENTEND = nPos + nMatchLen;  /* キーワード文字列の終端をセットする */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );	//@@@ 2002.01.04
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
					}
					else
//@@@ 2001.11.17 add end MIK
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cLineComment.Match( nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;

						nCOMMENTMODE = COLORIDX_COMMENT;	/* 行コメントである */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
					}else
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(0) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI

//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
					}else
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(1) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
//#endif
					}else
					if( pLine[nPos] == L'\'' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* シングルクォーテーション文字列を表示する */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_SSTRING;	/* シングルクォーテーション文字列である */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* シングルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( pLine[nPos] == L'"' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* ダブルクォーテーション文字列を表示する */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_WSTRING;	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* ダブルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URLを表示する */
					 && ( TRUE == IsURL( &pLine[nPos], nLineLen - nPos, &nUrlLen ) )	/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_URL;	/* URLモード */ // 2002/03/13 novice
						nCOMMENTEND = nPos + nUrlLen;
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
//@@@ 2001.02.17 Start by MIK: 半角数値を強調表示
					}else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (i = IsNumber( pLine, nPos, nLineLen )) > 0 )		/* 半角数字を表示する */
					{
						/* キーワード文字列の終端をセットする */
						i = nPos + i;
						/* 現在の色を指定 */
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_DIGIT;	/* 半角数値である */ // 2002/03/13 novice
						nCOMMENTEND = i;
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
//@@@ 2001.02.17 End by MIK: 半角数値を強調表示
					}else
					if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* キーワードセット */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* 強調キーワードを表示する */ // 2002/03/13 novice
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						//	Mar 4, 2001 genta comment out
						/* キーワード文字列の終端を探す */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							if( IS_KEYWORD_CHAR( pLine[i] ) ){
							}else{
								break;
							}
						}
						/* キーワードが登録単語ならば、色を変える */
						j = i - nPos;
						/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */
						nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(		//MIK UPDATE 2000.12.01 binary search
							TypeDataPtr->m_nKeyWordSetIdx[0],
							&pLine[nPos],
							j
						);
						if( nIdx != -1 ){
							/* 現在の色を指定 */
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_KEYWORD1;	/* 強調キーワード1 */ // 2002/03/13 novice
							nCOMMENTEND = i;
							if( !bSearchStringMode ){
								//@SetCurrentColor( hdc, nCOMMENTMODE );
								nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
							}
						}else{		//MIK START ADD 2000.12.01 second keyword & binary search
							// 2005.01.13 MIK 強調キーワード数追加に伴う配列化
							for( int my_i = 1; my_i < 10; my_i++ )
							{
								if(TypeDataPtr->m_nKeyWordSetIdx[my_i] != -1 && /* キーワードセット */							//MIK 2000.12.01 second keyword
									TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + my_i].m_bDisp)									//MIK
								{																							//MIK
									/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */						//MIK
									nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(									//MIK 2000.12.01 binary search
										TypeDataPtr->m_nKeyWordSetIdx[my_i] ,													//MIK
										&pLine[nPos],																		//MIK
										j																					//MIK
									);																						//MIK
									if( nIdx != -1 ){																		//MIK
										/* 現在の色を指定 */																//MIK
										nBgn = nPos;																		//MIK
										nCOMMENTMODE = COLORIDX_KEYWORD1 + my_i;	/* 強調キーワード2 */ // 2002/03/13 novice		//MIK
										nCOMMENTEND = i;																	//MIK
										if( !bSearchStringMode ){															//MIK
											//@SetCurrentColor( hdc, nCOMMENTMODE );										//MIK
											nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
										}																					//MIK
										break;
									}																						//MIK
								}																							//MIK
								else
								{
									if(TypeDataPtr->m_nKeyWordSetIdx[my_i] == -1 )
										break;
								}
							}
						}			//MIK END
					}
					//	From Here Mar. 4, 2001 genta
					if( IS_KEYWORD_CHAR( pLine[nPos] ))	bKeyWordTop = false;
					else								bKeyWordTop = true;
					//	To Here
					break;
// 2002/03/13 novice
				case COLORIDX_URL:		/* URLモードである */
				case COLORIDX_KEYWORD1:	/* 強調キーワード1 */
				case COLORIDX_DIGIT:	/* 半角数値である */  //@@@ 2001.02.17 by MIK
				case COLORIDX_KEYWORD2:	/* 強調キーワード2 */	//MIK
				case COLORIDX_KEYWORD3:
				case COLORIDX_KEYWORD4:
				case COLORIDX_KEYWORD5:
				case COLORIDX_KEYWORD6:
				case COLORIDX_KEYWORD7:
				case COLORIDX_KEYWORD8:
				case COLORIDX_KEYWORD9:
				case COLORIDX_KEYWORD10:
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* コントロールコード */ // 2002/03/13 novice
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = nCOMMENTMODE_OLD;
						nCOMMENTEND = nCOMMENTEND_OLD;
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;

				case COLORIDX_COMMENT:	/* 行コメントである */ // 2002/03/13 novice
					break;
				case COLORIDX_BLOCK1:	/* ブロックコメント1である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* シングルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ダブルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				default:	//@@@ 2002.01.04 add start
					if( nCOMMENTMODE >= 1000 && nCOMMENTMODE <= 1099 ){	//正規表現キーワード1～10
						if( nPos == nCOMMENTEND ){
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
							/* 現在の色を指定 */
							if( !bSearchStringMode ){
								//@SetCurrentColor( hdc, nCOMMENTMODE );
								nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
							}
							goto SEARCH_START;
						}
					}
					break;	//@@@ 2002.01.04 add end
				}
				if( pLine[nPos] == WCODE::TAB ){
					nBgn = nPos + 1;
					nCharChars = CLogicInt(1);
				}
				else if( WCODE::isZenkakuSpace(pLine[nPos]) && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )	//@@@ 2002.01.04
				{
					nBgn = nPos + 1;
					nCharChars = CLogicInt(1);
				}
				//半角空白（半角スペース）を表示 2002.04.28 Add by KK 
				else if (pLine[nPos] == L' ' && CTypeSupport(this,COLORIDX_SPACE).IsDisp() && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )
				{
					nBgn = nPos + 1;
					nCharChars = CLogicInt(1);
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = CLogicInt(1);
					}
					if( !bSearchStringMode
					 && 1 == nCharChars
					 && COLORIDX_CTRLCODE != nCOMMENTMODE // 2002/03/13 novice
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* コントロールコードを色分け */
					 && WCODE::isControlCode(pLine[nPos])
					){
						nBgn = nPos;
						nCOMMENTMODE_OLD = nCOMMENTMODE;
						nCOMMENTEND_OLD = nCOMMENTEND;
						nCOMMENTMODE = COLORIDX_CTRLCODE;	/* コントロールコード モード */ // 2002/03/13 novice
						/* コントロールコード列の終端を探す */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if(!WCODE::isControlCode(pLine[i])){
								break;
							}
						}
						nCOMMENTEND = i;
						/* 現在の色を指定 */
						//@SetCurrentColor( hdc, nCOMMENTMODE );
						nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
					}
				}
				nPos+= nCharChars;
			} //end of while( nPos - nLineBgn < pcLayout2->m_nLength ){
			if( nPos > nCol ){	// 03/10/24 ai 行頭のColorIndexが取得できない問題に対応
				break;
			}
		}

end_of_line:;

	}

//@end_of_func:;
	return nColorIndex;
}

/*!	挿入モード取得

	@date 2005.10.02 genta 管理方法変更のため関数化
*/
bool CEditView::IsInsMode(void) const
{
	return m_pcEditDoc->m_cDocEditor.IsInsMode();
}

void CEditView::SetInsMode(bool mode)
{
	m_pcEditDoc->m_cDocEditor.SetInsMode( mode );
}


void CEditView::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// -- -- ※ InitAllViewでやってたこと -- -- //
	pcDoc->m_nCommandExecNum=0;

	m_cHistory->Flush();

	/* 現在の選択範囲を非選択状態に戻す */
	GetSelectionInfo().DisableSelectArea( FALSE );

	OnChangeSetting();
	GetCaret().MoveCursor( CLayoutPoint(0, 0), TRUE );
	GetCaret().m_nCaretPosX_Prev = CLayoutInt(0);
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//	2004.05.13 Moca 改行コードの設定内からここに移動
	m_pcEditWnd->GetActiveView().GetCaret().ShowCaretPosInfo();
}
