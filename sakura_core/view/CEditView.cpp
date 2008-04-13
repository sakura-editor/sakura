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
#include "view/CEditView.h"
#include "debug.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include "CEditWnd.h"
#include "CDlgCancel.h"
#include "doc/CLayout.h"/// 2002/2/3 aroka
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
#include "util/os.h" //WM_MOUSEWHEEL,IMR_RECONVERTSTRING,WM_XBUTTON*,IMR_CONFIRMRECONVERTSTRING



CEditView*	g_m_pcEditView;
LRESULT CALLBACK EditViewWndProc( HWND, UINT, WPARAM, LPARAM );
VOID CALLBACK EditViewTimerProc( HWND, UINT, UINT, DWORD );

#define IDT_ROLLMOUSE	1



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


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ウィンドウイベント                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           設定                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        キャレット                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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






// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           解析                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           変換                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         メニュー                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         設定反映                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 設定変更を反映させる */
void CEditView::OnChangeSetting()
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
}


/* 自分の表示状態を他のビューにコピー */
void CEditView::CopyViewStatus( CEditView* pView ) const
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
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       分割ボックス                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 縦・横の分割ボックス・サイズボックスのＯＮ／ＯＦＦ */
void CEditView::SplitBoxOnOff( BOOL bVert, BOOL bHorz, BOOL bSizeBox )
{
	RECT	rc;
	if( bVert ){
		if( m_pcsbwVSplitBox == NULL ){	/* 垂直分割ボックス */
			m_pcsbwVSplitBox = new CSplitBoxWnd;
			m_pcsbwVSplitBox->Create( m_hInstance, GetHwnd(), TRUE );
		}
	}
	else{
		SAFE_DELETE(m_pcsbwVSplitBox);	/* 垂直分割ボックス */
	}
	if( bHorz ){
		if( m_pcsbwHSplitBox == NULL ){	/* 水平分割ボックス */
			m_pcsbwHSplitBox = new CSplitBoxWnd;
			m_pcsbwHSplitBox->Create( m_hInstance, GetHwnd(), FALSE );
		}
	}
	else{
		SAFE_DELETE(m_pcsbwHSplitBox);	/* 水平分割ボックス */
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
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       テキスト選択                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      クリップボード                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


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






// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      アンダーライン                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         状態表示                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	検索／置換／ブックマーク検索時の状態をステータスバーに表示する

	@date 2002.01.26 hor 新規作成
	@date 2002.12.04 genta 実体をCEditWndへ移動
*/
void CEditView::SendStatusMessage( const TCHAR* msg )
{
	m_pcEditDoc->m_pcEditWnd->SendStatusMessage( msg );
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        編集モード                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          その他                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



//!	現在のカーソル行位置を履歴に登録する
void CEditView::AddCurrentLineToHistory( void )
{
	CLogicPoint ptPos;
	//int PosX, PosY;	//	物理位置（改行単位の計算）

	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( GetCaret().GetCaretLayoutPos(), &ptPos );

	CMarkMgr::CMark m( ptPos );
	m_cHistory->Add( m );
}


//	2001/06/18 Start by asa-o: 補完ウィンドウ用のキーワードヘルプ表示
bool  CEditView::ShowKeywordHelp( POINT po, LPCWSTR pszHelp, LPRECT prcHokanWin)
{
	CNativeW	cmemCurText;
	RECT		rcTipWin,
				rcDesktop;

	if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyWordHelp ){ /* キーワードヘルプを使用する */
		if( m_bInMenuLoop == FALSE	&&	/* メニュー モーダル ループに入っていない */
			0 != m_dwTipTimer			/* 辞書Tipを表示していない */
		){
			cmemCurText.SetString( pszHelp );

			/* 既に検索済みか */
			if( CNativeW::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) ){
				/* 該当するキーがなかった */
				if( !m_cTipWnd.m_KeyWasHit ){
					return false;
				}
			}else{
				m_cTipWnd.m_cKey = cmemCurText;
				/* 検索実行 */
				if(FALSE == KeySearchCore(&m_cTipWnd.m_cKey))	// 2006.04.10 fon
					return FALSE;
			}
			m_dwTipTimer = 0;	/* 辞書Tipを表示している */

		// 2001/06/19 Start by asa-o: 辞書Tipの表示位置調整
			// 辞書Tipのサイズを取得
			m_cTipWnd.GetWindowSize(&rcTipWin);

			//	May 01, 2004 genta マルチモニタ対応
			::GetMonitorWorkRect( m_cTipWnd.GetHwnd(), &rcDesktop );

			// 右に入る
			if(prcHokanWin->right + rcTipWin.right < rcDesktop.right){
				// そのまま
			}else
			// 左に入る
			if(rcDesktop.left < prcHokanWin->left - rcTipWin.right ){
				// 左に表示
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			}else
			// どちらもスペースが無いとき広いほうに表示
			if(rcDesktop.right - prcHokanWin->right > prcHokanWin->left ){
				// 右に表示 そのまま
			}else{
				// 左に表示
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			}
		// 2001/06/19 End

			/* 辞書Tipを表示 */
			m_cTipWnd.Show( po.x, po.y , NULL , &rcTipWin);
			return true;
		}
	}
	return false;
}
//	2001/06/18 End
