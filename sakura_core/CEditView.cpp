//	$Id$
/*!	@file
	文書ウィンドウの管理
	
	@author Norio Nakatani
	@date	1998/03/13 作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

//#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "CEditView.h"
#include "debug.h"
#include "keycode.h"
#include "funccode.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include "CEditWnd.h"
#include "CShareData.h"
#include "CDlgCancel.h"
#include "sakura_rc.h"
//#include "_global_fio.h"
#include "etc_uty.h"
#include "CJre.h"
#include "global.h"
//#include "CDataObject.h"
#include "CAutoSave.h"


#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL	0x020A
#endif




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

//DWORD DoGrepProc(
//	DWORD	dwGrepParam
//);

BOOL IsDataAvailable( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
//	STGMEDIUM	stgMedium;
	FORMATETC	fe;
	BOOL		bRes;
//初期形
//	fe.cfFormat = cfFormat;
//	fe.ptd = NULL;
//	fe.dwAspect = DVASPECT_CONTENT;
//	fe.lindex = -1;
//	fe.tymed = (TYMED)-1;
//	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );


/*
	TYMED_HGLOBAL	= 1,
	TYMED_FILE		= 2,
	TYMED_ISTREAM	= 4,
	TYMED_ISTORAGE	= 8,
	TYMED_GDI		= 16,
	TYMED_MFPICT	= 32,
	TYMED_ENHMF		= 64,
	TYMED_NULL		= 0
*/
//	MYTRACE( "=====================\n" );

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_FILE;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_ISTREAM;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_ISTORAGE;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_GDI;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_MFPICT;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_ENHMF;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}


	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_NULL;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_HGLOBAL;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );

//	MYTRACE( "=====================\n" );
	return bRes;
}
HGLOBAL GetGlobalData( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC fe;
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)-1;

	HGLOBAL hDest = NULL;
	STGMEDIUM stgMedium;
	if( SUCCEEDED( pDataObject->GetData( &fe, &stgMedium ) ) ){
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
					memcpy( lpDest, lpSource, nSize );
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
	CEditView*	pCEdit;
	switch( uMsg ){
	case WM_CREATE:
		pCEdit = ( CEditView* )g_m_pcEditView;
		return pCEdit->DispatchEvent( hwnd, uMsg, wParam, lParam );
	default:
		pCEdit = ( CEditView* )::GetWindowLong( hwnd, 0 );
		if( NULL != pCEdit ){
			//	May 16, 2000 genta
			//	From Here
			if( uMsg == WM_COMMAND ){
				::SendMessage( ::GetParent( pCEdit->m_hwndParent ), WM_COMMAND, wParam,  lParam );
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
	pCEditView = ( CEditView* )::GetWindowLong( hwnd, 0 );
	if( NULL != pCEditView ){
		pCEditView->OnTimer( hwnd, uMsg, idEvent, dwTime );
	}
	return;
}




CEditView::CEditView() : m_cHistory( new CAutoMarkMgr )
{
	TEXTMETRIC	tm;
	LOGFONT		lf;
	HDC			hdc;
	HFONT		hFontOld;
	int			i;

	m_CurSrch_CJre.Init();

	m_bDrawSWITCH = TRUE;
	m_pcDropTarget = new CDropTarget( this );
	m_bDragSource = FALSE;
	m_bDragMode = FALSE;					/* 選択テキストのドラッグ中か */
	m_bCurSrchKeyMark = FALSE;				/* 検索文字列 */
	strcpy( m_szCurSrchKey, "" );			/**/
	m_bCurSrchRegularExp = 0;				/* 検索／置換  1==正規表現 */
	m_bCurSrchLoHiCase = 0;					/* 検索／置換  1==英大文字小文字の区別 */
	m_bCurSrchWordOnly = 0;					/* 検索／置換  1==単語のみ検索 */
//	m_CurSrch_CJre.Init();

	m_bExecutingKeyMacro = FALSE;			/* キーボードマクロの実行中 */
	m_bPrevCommand = 0;
	m_nMyIndex = 0;


	/* キャレットの行桁位置表示用フォント */
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

	/* 共有データ構造体のアドレスを返す */
	m_cShareData.Init();
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );
	m_bCommandRunning = FALSE;	/* コマンドの実行中 */
	m_pcOpeBlk = NULL;			/* 操作ブロック */
	m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */
	m_pcsbwVSplitBox = NULL;	/* 垂直分割ボックス */
	m_pcsbwHSplitBox = NULL;	/* 水平分割ボックス */
	m_pszAppName = "EditorClient";
	m_hInstance = NULL;
	m_hWnd = NULL;
	m_hwndVScrollBar = NULL;
	m_nVScrollRate = 1;			/* 垂直スクロールバーの縮尺 */
	m_hwndHScrollBar = NULL;
	m_hwndSizeBox = NULL;
	m_nCaretPosX = 0;			/* ビュー左端からのカーソル桁位置(０オリジン) */
	m_nCaretPosX_Prev = 0;		/* ビュー左端からのカーソル桁直前の位置(０オリジン) */
	m_nCaretPosY = 0;			/* ビュー上端からのカーソル行位置(０オリジン) */

	m_nCaretPosX_PHY = 0;		/* カーソル位置 改行単位行先頭からのバイト数(０開始) */
	m_nCaretPosY_PHY = 0;		/* カーソル位置 改行単位行の行番号(０開始) */


	m_nCaretWidth = 0;			/* キャレットの幅 */
	m_nCaretHeight = 0;			/* キャレットの高さ */
	m_bSelectingLock = FALSE;	/* 選択状態のロック */
	m_bBeginSelect = FALSE;		/* 範囲選択中 */
	m_bBeginBoxSelect = FALSE;	/* 矩形範囲選択中 */
	m_bBeginLineSelect = FALSE;	/* 行単位選択中 */
	m_bBeginWordSelect = FALSE;	/* 単語単位選択中 */
//	m_nSelectLineBgn = 0;		/* 範囲選択開始行(原点) */
//	m_nSelectColmBgn = 0;		/* 範囲選択開始桁(原点) */
//	m_nSelectLineFrom = 0;		/* 範囲選択開始行 */
//	m_nSelectColmFrom = 0;		/* 範囲選択開始桁 */
//	m_nSelectLineTo = 0;		/* 範囲選択終了行 */
//	m_nSelectColmTo = 0;		/* 範囲選択終了桁 */

//	m_nSelectLineBgn = -1;		/* 範囲選択開始行(原点) */
//	m_nSelectColmBgn = -1;		/* 範囲選択開始桁(原点) */
	m_nSelectLineBgnFrom = -1;	/* 範囲選択開始行(原点) */
	m_nSelectColmBgnFrom = -1;	/* 範囲選択開始桁(原点) */
	m_nSelectLineBgnTo = -1;	/* 範囲選択開始行(原点) */
	m_nSelectColmBgnTo = -1;	/* 範囲選択開始桁(原点) */

	m_nSelectLineFrom = -1;		/* 範囲選択開始行 */
	m_nSelectColmFrom = -1;		/* 範囲選択開始桁 */
	m_nSelectLineTo = -1;		/* 範囲選択終了行 */
	m_nSelectColmTo = -1;		/* 範囲選択終了桁 */

	m_nSelectLineFromOld = 0;	/* 範囲選択開始行 */
	m_nSelectColmFromOld = 0;	/* 範囲選択開始桁 */
	m_nSelectLineToOld = 0;		/* 範囲選択終了行 */
	m_nSelectColmToOld = 0;		/* 範囲選択終了桁 */
	m_nViewAlignLeft = 0;		/* 表示域の左端座標 */
	m_nViewAlignLeftCols = 0;	/* 行番号域の桁数 */
	m_nTopYohaku = m_pShareData->m_Common.m_nRulerBottomSpace; 	/* ルーラーとテキストの隙間 */
	m_nViewAlignTop = m_nTopYohaku;		/* 表示域の上端座標 */
	/* ルーラー表示 */
//	if( m_pShareData->m_Common.m_bRulerDisp ){
//	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		m_nViewAlignTop += m_pShareData->m_Common.m_nRulerHeight;	/* ルーラー高さ */
//	}
	m_nViewCx = 0;				/* 表示域の幅 */
	m_nViewCy = 0;				/* 表示域の高さ */
	m_nViewColNum = 0;			/* 表示域の桁数 */
	m_nViewRowNum = 0;			/* 表示域の行数 */
	m_nViewTopLine = 0;			/* 表示域の一番上の行 */
	m_nViewLeftCol = 0;			/* 表示域の一番左の桁 */
	m_hdcCompatDC = NULL;		/* 再描画用コンパチブルＤＣ */
	m_hbmpCompatBMP = NULL;		/* 再描画用メモリＢＭＰ */
	m_hbmpCompatBMPOld = NULL;	/* 再描画用メモリＢＭＰ(OLD) */
	m_nCharWidth = 10;			/* 半角文字の幅 */
	m_nCharHeight = 18;			/* 文字の高さ */
	/* フォント作成 */
	m_hFont_HAN = CreateFontIndirect( &(m_pShareData->m_Common.m_lf) );

	/* 太字フォント作成 */
	lf = m_pShareData->m_Common.m_lf;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT = CreateFontIndirect( &lf );

	/* 下線フォント作成 */
	lf = m_pShareData->m_Common.m_lf;
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* 太字下線フォント作成 */
	lf = m_pShareData->m_Common.m_lf;
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT_UL = CreateFontIndirect( &lf );





	lf = m_pShareData->m_Common.m_lf;
	lf.lfCharSet = SHIFTJIS_CHARSET;
	lf.lfOutPrecision = 1;
	strcpy( lf.lfFaceName, "ＭＳ ゴシック" );
	m_hFont_ZEN = CreateFontIndirect( &lf );
	m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
	m_bInMenuLoop = FALSE;				/* メニュー モーダル ループに入っています */
//	MYTRACE( "CEditView::CEditView()おわり\n" );
	m_bHokan = FALSE;

	m_hFontOld = NULL;

	//	Aug. 31, 2000 genta
	m_cHistory->SetMax( 30 );

	return;
}


CEditView::~CEditView()
{
	DeleteObject( m_hFont_HAN );
	DeleteObject( m_hFont_HAN_FAT );
	DeleteObject( m_hFont_HAN_UL );
	DeleteObject( m_hFont_HAN_FAT_UL );
	DeleteObject( m_hFont_ZEN );

	if( m_hWnd != NULL ){
		DestroyWindow( m_hWnd );
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

	/* キャレットの行桁位置表示用フォント */
	::DeleteObject( m_hFontCaretPosInfo );

	delete m_pcDropTarget;
	m_pcDropTarget = NULL;

	delete m_cHistory;
	return;
}



BOOL CEditView::Create(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	CEditDoc*	pcEditDoc,
	int			nMyIndex,
	BOOL		bShow
)
{
	WNDCLASS	wc;
	SCROLLINFO	si;
	HDC			hdc;
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;
	m_pcEditDoc = pcEditDoc;
	m_nMyIndex = nMyIndex;

	m_nTopYohaku = m_pShareData->m_Common.m_nRulerBottomSpace; 	/* ルーラーとテキストの隙間 */
	m_nViewAlignTop = m_nTopYohaku;								/* 表示域の上端座標 */
	/* ルーラー表示 */
//	if( m_pShareData->m_Common.m_bRulerDisp ){
	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		m_nViewAlignTop += m_pShareData->m_Common.m_nRulerHeight;	/* ルーラー高さ */
	}


	/* ウィンドウクラスの登録 */
	//	Apr. 27, 2000 genta
	//	サイズ変更時のちらつきを抑えるためCS_HREDRAW | CS_VREDRAW を外した
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= (WNDPROC)EditViewWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= sizeof( LONG );
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
	m_hWnd = ::CreateWindowEx(
		0
		/*| WS_EX_CLIENTEDGE */
		| WS_EX_STATICEDGE
		,	// extended window style

		m_pszAppName,			// pointer to registered class name
		m_pszAppName,			// pointer to window name
		0
		| WS_VISIBLE
		| WS_CHILD
		| WS_CLIPCHILDREN
		/*| WS_BORDER*/
		, // window style

		CW_USEDEFAULT,			// horizontal position of window
		0,						// vertical position of window
		CW_USEDEFAULT,			// window width
		0,						// window height
		hwndParent,				// handle to parent or owner window
		NULL,					// handle to menu or child-window identifier
		m_hInstance,			// handle to application instance
		(LPVOID)this			// pointer to window-creation data
	);
	if( NULL == m_hWnd ){
		return FALSE;
	}

//	CDropTarget::Register_DropTarget( m_hWnd );
	m_pcDropTarget->Register_DropTarget( m_hWnd );
//	::OleInitialize(NULL);
//	if( FAILED( ::RegisterDragDrop( m_hWnd, this ) ) ){
//		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
//			"::RegisterDragDrop()\n失敗"
//		);
//
//	}

	/* 辞書Tip表示ウィンドウ作成 */
	m_cTipWnd.Create( m_hInstance, m_hWnd/*m_pShareData->m_hwndTray*/ );

	/* 再描画用コンパチブルＤＣ */
	hdc = ::GetDC( m_hWnd );
	m_hdcCompatDC = ::CreateCompatibleDC( hdc );
	::ReleaseDC( m_hWnd, hdc );

	/* 垂直分割ボックス */
	m_pcsbwVSplitBox = new CSplitBoxWnd;
	m_pcsbwVSplitBox->Create( m_hInstance, m_hWnd, TRUE );
	/* 水平分割ボックス */
	m_pcsbwHSplitBox = new CSplitBoxWnd;
	m_pcsbwHSplitBox->Create( m_hInstance, m_hWnd, FALSE );


	/* スクロールバーの作成 */
	m_hwndVScrollBar = ::CreateWindowEx(
		0L,									/* no extended styles */
		"SCROLLBAR",						/* scroll bar control class */
		(LPSTR) NULL,						/* text for window title bar */
		WS_VISIBLE | WS_CHILD | SBS_VERT,	/* scroll bar styles */
		0,									/* horizontal position */
		0,									/* vertical position */
		200,								/* width of the scroll bar */
		CW_USEDEFAULT,						/* default height */
		m_hWnd,								/* handle of main window */
		(HMENU) NULL,						/* no menu for a scroll bar */
		m_hInstance,						/* instance owning this window */
		(LPVOID) NULL						/* pointer not needed */
	);
	si.cbSize = sizeof( si );
	si.fMask = SIF_ALL;
	si.nMin  = 0;
	si.nMax  = 29;
	si.nPage = 10;
	si.nPos  = 0;
	si.nTrackPos = 1;
	::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
	::ShowScrollBar( m_hwndVScrollBar, SB_CTL, TRUE );

	/* スクロールバーの作成 */
	m_hwndHScrollBar = NULL;
	if( m_pShareData->m_Common.m_bScrollBarHorz ){	/* 水平スクロールバーを使う */
		m_hwndHScrollBar = ::CreateWindowEx(
			0L,									/* no extended styles */
			"SCROLLBAR",						/* scroll bar control class */
			(LPSTR) NULL,						/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_HORZ,	/* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			m_hWnd,								/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL;
		si.nMin  = 0;
		si.nMax  = 29;
		si.nPage = 10;
		si.nPos  = 0;
		si.nTrackPos = 1;
		::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );
		::ShowScrollBar( m_hwndHScrollBar, SB_CTL, TRUE );
	}


	/* サイズボックス */
	if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 ){	/* ファンクションキー表示位置／0:上 1:下 */
		m_hwndSizeBox = ::CreateWindowEx(
			WS_EX_CONTROLPARENT/*0L*/, 			/* no extended styles */
			"SCROLLBAR",						/* scroll bar control class */
			(LPSTR) NULL,						/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			m_hWnd, 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}else{
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 								/* no extended styles */
			"STATIC",							/* scroll bar control class */
			(LPSTR) NULL,						/* text for window title bar */
			WS_VISIBLE | WS_CHILD/* | SBS_SIZEBOX | SBS_SIZEGRIP*/, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			m_hWnd, 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}

	SetFont();
	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	if( bShow ){
		ShowWindow( m_hWnd, SW_SHOW );
	}

	/* 親ウィンドウのタイトルを更新 */
	SetParentCaption();

//	/* 入力補完ウィンドウ作成 */
//	m_cHokanMgr.DoModeless( m_hInstance , m_hWnd, (LPARAM)this );

	/* キーボードの現在のリピート間隔を取得 */
	int nKeyBoardSpeed;
	SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0 );
//	nKeyBoardSpeed *= 2;
	/* タイマー起動 */
	if( 0 == ::SetTimer( m_hWnd, IDT_ROLLMOUSE, nKeyBoardSpeed, (TIMERPROC)EditViewTimerProc ) ){
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
			"CEditView::Create()\nタイマーが起動できません。\nシステムリソースが不足しているのかもしれません。"
		);
	}
	return TRUE;
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
	PAINTSTRUCT	ps;
//	int			nPos;
	int			nPosX;
	int			nPosY;
//	WORD		fwKeys;
//	short		zDelta;
//	short		xPos;
//	short		yPos;
//	int			i;
//	int			nScrollCode;
	switch ( uMsg ){
//-	case WM_DESTROYCLIPBOARD:
//-		MYTRACE( "CEditView WM_DESTROYCLIPBOARD\n" );
//-		return 0;
	case WM_MOUSEWHEEL:
		return OnMOUSEWHEEL( wParam, lParam );



	case WM_CREATE:
		::SetWindowLong( hwnd, 0, (LONG)this );

//		/* キーボードの現在のリピート間隔を取得 */
//		int	nKeyBoardSpeed;
//		SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0 );
//		/* タイマー起動 */
//		::SetTimer( hwnd, IDT_ROLLMOUSE, nKeyBoardSpeed, (TIMERPROC)EditViewTimerProc );

		return 0L;

	case WM_SIZE:
//		MYTRACE( "	WM_SIZE\n" );
		OnSize( LOWORD( lParam ), HIWORD( lParam ) );
		return 0L;



	case WM_SETFOCUS:
//		MYTRACE( "	WM_SETFOCUS m_nMyIndex=%d\n", m_nMyIndex );
		OnSetFocus();

		/* 親ウィンドウのタイトルを更新 */
		SetParentCaption();

		return 0L;
	case WM_KILLFOCUS:
		OnKillFocus();

//		/* 親ウィンドウのタイトルを更新 */
//		SetParentCaption( TRUE );

		return 0L;
	case WM_CHAR:
//		MYTRACE( "WM_CHAR\n" );
		HandleCommand( F_CHAR, TRUE, wParam, 0, 0, 0 );
		return 0L;

	case WM_IME_COMPOSITION:
		if( m_pShareData->m_Common.m_bIsINSMode /* 挿入モードか？ */
		 &&	lParam & GCS_RESULTSTR
		){
			HIMC hIMC;
			DWORD dwSize;
			HGLOBAL hstr;
			LPSTR lpstr;
			hIMC = ImmGetContext( hwnd );

			if( !hIMC ){
				return 0;
//				MyError( ERROR_NULLCONTEXT );
			}

			// Get the size of the result string.
			dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);

			// increase buffer size for NULL terminator,
			//   maybe it is in Unicode
			dwSize += sizeof( WCHAR );

			hstr = GlobalAlloc( GHND, dwSize );
			if( hstr == NULL ){
				return 0;
//				 MyError( ERROR_GLOBALALLOC );
			}

			lpstr = (LPSTR)GlobalLock( hstr );
			if( lpstr == NULL ){
				return 0;
//				 MyError( ERROR_GLOBALLOCK );
			}

			// Get the result strings that is generated by IME into lpstr.
			ImmGetCompositionString(hIMC, GCS_RESULTSTR, lpstr, dwSize);

//			MYTRACE( "lpstr=[%s]", lpstr );
			/* テキストを貼り付け */
			HandleCommand( F_INSTEXT, TRUE, (LPARAM)lpstr, TRUE, 0, 0 );

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
		if( FALSE == m_pShareData->m_Common.m_bIsINSMode ){ /* 上書きモードか？ */
			HandleCommand( F_IME_CHAR, TRUE, (WORD)( (((WORD)wParam&0x00ff)<<8) | (((WORD)wParam&0xff00)>>8) ), 0, 0, 0 );
		}
		return 0L;

//		if( NULL != m_pcOpeBlk ){	/* 操作ブロック */
//			return 1L;
//		}
//		/* コマンドの実行中 */
//		m_bCommandRunning = TRUE;
//
//		if( NULL != m_pcOpeBlk ){	/* 操作ブロック */
////			while( NULL != m_pcOpeBlk ){}
//			delete m_pcOpeBlk;
//		}
//		m_pcOpeBlk = new COpeBlk;
//
//		Command_IME_CHAR( (WORD)( (((WORD)wParam&0x00ff)<<8) | (((WORD)wParam&0xff00)>>8) ) ); /* ２バイト文字入力 */
//
//		/* アンドゥバッファの処理 */
//		if( 0 < m_pcOpeBlk->GetNum() ){	/* 操作の数を返す */
//			/* 操作の追加 */
//			m_pcEditDoc->m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
//		}else{
//			delete m_pcOpeBlk;
//		}
//		m_pcOpeBlk = NULL;
//		m_bCommandRunning = FALSE;
//        return 0L;

	case WM_KEYUP:
		/* キーリピート状態 */
		m_bPrevCommand = 0;
        return 0L;


//	case WM_MBUTTONDBLCLK:
	case WM_LBUTTONDBLCLK:


//		MYTRACE( " WM_LBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONDBLCLK( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

//	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
//	case WM_RBUTTONDOWN:
		::SetFocus( ::GetParent( m_hwndParent ) );

		if( m_nMyIndex != m_pcEditDoc->GetActivePane() ){
			/* アクティブなペインを設定 */
			m_pcEditDoc->SetActivePane( m_nMyIndex );
		}
//		MYTRACE( " WM_LBUTTONDOWN wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;
//	case WM_MBUTTONUP:
	case WM_LBUTTONUP:

//		MYTRACE( " WM_LBUTTONUP wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;
	case WM_MOUSEMOVE:
		OnMOUSEMOVE( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

	case WM_RBUTTONDBLCLK:
		::SetFocus( ::GetParent( m_hwndParent ) );
//		MYTRACE( " WM_RBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		return 0L;
//	case WM_RBUTTONDOWN:
//		MYTRACE( " WM_RBUTTONDOWN wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
//		OnRBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
//		if( m_nMyIndex != m_pcEditDoc->GetActivePane() ){
//			/* アクティブなペインを設定 */
//			m_pcEditDoc->SetActivePane( m_nMyIndex );
//		}
//		return 0L;
	case WM_RBUTTONUP:
//		MYTRACE( " WM_RBUTTONUP wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnRBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

	case WM_VSCROLL:
		::SetFocus( ::GetParent( m_hwndParent ) );

//		MYTRACE( "	WM_VSCROLL nPos=%d\n", GetScrollPos( m_hwndVScrollBar, SB_CTL ) );
		OnVScroll(
			(int) LOWORD( wParam ),
			((int) HIWORD( wParam )) * m_nVScrollRate,
			(HWND) lParam
		);

		if( m_nMyIndex != m_pcEditDoc->GetActivePane() ){
			/* アクティブなペインを設定 */
			m_pcEditDoc->SetActivePane( m_nMyIndex );
		}
		return 0L;

	case WM_HSCROLL:
		::SetFocus( ::GetParent( m_hwndParent ) );
//		MYTRACE( "	WM_HSCROLL nPos=%d\n", GetScrollPos( m_hwndHScrollBar, SB_CTL ) );
		OnHScroll(
			(int) LOWORD( wParam ),
			((int) HIWORD( wParam )),
			(HWND) lParam
		);

		if( m_nMyIndex != m_pcEditDoc->GetActivePane() ){
			/* アクティブなペインを設定 */
			m_pcEditDoc->SetActivePane( m_nMyIndex );
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
			m_pcEditDoc->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
		}
		return 0L;

	case WM_EXITMENULOOP:
		m_bInMenuLoop = FALSE;	/* メニュー モーダル ループに入っています */
		return 0L;


    case WM_PAINT:
		hdc = BeginPaint( hwnd, &ps );
		OnPaint( hdc, &ps, FALSE );
        EndPaint(hwnd, &ps);
        return 0L;

	case WM_CLOSE:
//		MYTRACE( "	WM_CLOSE\n" );
		DestroyWindow( hwnd );
		return 0L;
	case WM_DESTROY:
//		CDropTarget::Revoke_DropTarget();
		m_pcDropTarget->Revoke_DropTarget();
//		::RevokeDragDrop( m_hWnd );
//		::OleUninitialize();

		/* タイマー終了 */
		::KillTimer( m_hWnd, IDT_ROLLMOUSE );


//		MYTRACE( "	WM_DESTROY\n" );
		/*
		||子ウィンドウの破棄
		*/
		if( NULL != m_hwndHScrollBar ){
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


		m_hWnd = NULL;
		return 0L;

	case MYWM_DOSPLIT:
		nPosX = (int)wParam;
		nPosY = (int)lParam;
//		MYTRACE( "MYWM_DOSPLIT nPosX=%d nPosY=%d\n", nPosX, nPosY );
		::SendMessage( m_hwndParent, MYWM_DOSPLIT, wParam, lParam );
		return 0L;

	case MYWM_SETACTIVEPANE:
		m_pcEditDoc->SetActivePane( m_nMyIndex );
		::PostMessage( m_hwndParent, MYWM_SETACTIVEPANE, (WPARAM)m_nMyIndex, 0 );
		return 0L;


	default:
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}




void CEditView::OnMove( int x, int y, int nWidth, int nHeight )
{
	MoveWindow( m_hWnd, x, y, nWidth, nHeight, TRUE );
	return;
}




/* ウィンドウサイズの変更処理 */
void CEditView::OnSize( int cx, int cy )
{
	if( NULL == m_hWnd ){
		return;
	}
	if( cx == 0 && cy == 0 ){
		return;
	}
//#ifdef _DEBUG
//	/* デバッグモニタに出力 */
//	m_cShareData.TraceOut( "%s(%d): CEditView::OnSize( int cx, int cy ); cx=%d cy=%d\n", __FILE__, __LINE__, cx, cy );
//#endif

	int	nCxHScroll;
	int	nCyHScroll;
	int	nCxVScroll;
	int	nCyVScroll;
	int	nVSplitHeight;	/* 垂直分割ボックスの高さ */
	int	nHSplitWidth;	/* 水平分割ボックスの幅 */


	nVSplitHeight = 0;	/* 垂直分割ボックスの高さ */
	nHSplitWidth = 0;	/* 水平分割ボックスの幅 */

	HDC	hdc;
	nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );


	hdc = ::GetDC( m_hWnd );

	/* 垂直分割ボックス */
	if( NULL != m_pcsbwVSplitBox ){
		nVSplitHeight = 7;
		::MoveWindow( m_pcsbwVSplitBox->m_hWnd, cx - nCxVScroll , 0, nCxVScroll, nVSplitHeight, TRUE );
	}
	/* 水平分割ボックス */
	if( NULL != m_pcsbwHSplitBox ){
		nHSplitWidth = 7;
		::MoveWindow( m_pcsbwHSplitBox->m_hWnd,0, cy - nCyHScroll, nHSplitWidth, nCyHScroll, TRUE );
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
	}else{
	}


	m_nViewCx = cx - nCxVScroll - m_nViewAlignLeft;														/* 表示域の幅 */
	m_nViewCy = cy - ((NULL != m_hwndHScrollBar)?nCyHScroll:0) - m_nViewAlignTop;						/* 表示域の高さ */
	m_nViewColNum = m_nViewCx / ( m_nCharWidth  + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );	/* 表示域の桁数 */
	m_nViewRowNum = m_nViewCy / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );	/* 表示域の行数 */

//1999.12.1 仕様変更
//	/* ウィンドウリサイズ時にキャレット位置へスクロール */
//	MoveCursor( m_nCaretPosX, m_nCaretPosY, TRUE );

	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	/* 再描画用メモリＢＭＰ */
	if( m_hbmpCompatBMP != NULL ){
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );	/* 再描画用メモリＢＭＰ(OLD) */
		::DeleteObject( m_hbmpCompatBMP );
	}
	m_hbmpCompatBMP = ::CreateCompatibleBitmap( hdc, cx, cy );
	m_hbmpCompatBMPOld = (HBITMAP)::SelectObject( m_hdcCompatDC, m_hbmpCompatBMP );
	::ReleaseDC( m_hWnd, hdc );

	/* 親ウィンドウのタイトルを更新 */
	SetParentCaption();


//	/* 現在のウィンドウ幅で折り返し	*/
//	Command_WRAPWINDIWWIDTH();






	/* ウィンドウサイズに折り返し文字数を追従させるモード */
//
//	if( 10 > m_nViewColNum - 1 ){
//		::MessageBeep( MB_ICONHAND );
//		return;
//	}
//	m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize = m_nViewColNum - 1;
//
//	m_pcEditDoc->OnChangeSetting();	/* ビューに設定変更を反映させる */
//
//	/* 設定変更を反映させる */
//	m_cShareData.SendMessageToAllEditors(
//		MYWM_CHANGESETTING, (WPARAM)0, (LPARAM)0, ::GetParent( m_hwndParent )
//	);	/* 全編集ウィンドウへメッセージをポストする */
//
//	m_nViewLeftCol = 0;		/* 表示域の一番左の桁(0開始) */
//
//	/* フォーカス移動時の再描画 */
//	RedrawAll();
//	return;

	return;
}


/* キャレットの表示・更新 */
void CEditView::ShowEditCaret( void )
{
	const char*		pLine;
	int				nLineLen;
	int				nCaretWidth;
	int				nCaretHeight;
	int				nIdxFrom;
	int				nCharChars;
	HDC				hdc;
	const CLayout*	pcLayout;
//	HPEN			hPen, hPenOld;


//if( m_nMyIndex == 0 && m_nCaretPosX == 0 && m_nCaretPosY == 0 ){
//	MYTRACE( "ShowEditCaret() m_nMyIndex=%d m_nCaretWidth=%d\n", m_nMyIndex, m_nCaretWidth );
//}

/*
	   なんかフレームウィンドウがアクティブでないときに内部的にカーソル移動すると
	   カーソルがないのに、カーソルがあるということになってしまう
	   のでアクティブにしてもカーソルが出てこないときがある
	   フレームウィンドウがアクティブでないときは、カーソルがないことにする
*/
	if( ::GetActiveWindow() != ::GetParent( m_hwndParent ) ){
		m_nCaretWidth = 0;
//		MYTRACE( "アクティブでないのにカーソル作っちゃったから消しちゃった。\n" );
		return;
	}

	/* アクティブなペインを取得 */
	if( m_nMyIndex != m_pcEditDoc->GetActivePane() ){
		m_nCaretWidth = 0;
//if( m_nMyIndex == 0 && m_nCaretPosX == 0 && m_nCaretPosY == 0 ){
//	MYTRACE( "m_nMyIndex[%s] != m_pcEditDoc->GetActivePane()\n", m_nMyIndex, m_pcEditDoc->GetActivePane() );
//}
		return;
	}
	/* キャレットの幅、高さを決定 */
	if( 0 == m_pShareData->m_Common.GetCaretType() ){	/* カーソルのタイプ 0=win 1=dos */
		nCaretHeight = m_nCharHeight;					/* キャレットの高さ */
		if( m_pShareData->m_Common.m_bIsINSMode ){
			nCaretWidth = 2;
		}else{
			nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );
				if( nIdxFrom >= nLineLen ||
					pLine[nIdxFrom] == CR || pLine[nIdxFrom] == LF ||
					pLine[nIdxFrom] == TAB ){
					nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				}else{
					nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nIdxFrom] ) - &pLine[nIdxFrom];
					if( 0 < nCharChars ){
						nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) * nCharChars;
					}
				}
			}
		}
	}else
	if( 1 == m_pShareData->m_Common.GetCaretType() ){	/* カーソルのタイプ 0=win 1=dos */
		if( m_pShareData->m_Common.m_bIsINSMode ){
			nCaretHeight = m_nCharHeight / 2;			/* キャレットの高さ */
		}else{
			nCaretHeight = m_nCharHeight;				/* キャレットの高さ */
		}
		nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY, &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* 指定された桁に対応する行のデータ内の位置を調べる */
			nIdxFrom = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );
			if( nIdxFrom >= nLineLen ||
				pLine[nIdxFrom] == CR || pLine[nIdxFrom] == LF ||
				pLine[nIdxFrom] == TAB ){
				nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			}else{
				nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nIdxFrom] ) - &pLine[nIdxFrom];
				if( 0 < nCharChars ){
					nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) * nCharChars;
				}
			}
		}

	}
//	int		nDummy;
//	if( m_nMyIndex == 0 ){
//		nDummy = 0;
//	}else
//	if( m_nMyIndex == 1 ){
//		nDummy = 1;
//	}else
//	if( m_nMyIndex == 2 ){
//		nDummy = 2;
//	}else
//	if( m_nMyIndex == 3 ){
//		nDummy = 3;
//	}

	hdc = ::GetDC( m_hWnd );
	if( m_nCaretWidth == 0 ){	/* キャレットがなかった場合 */

//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && !IsTextSelecting() ){
//			/* カーソル行アンダーラインの描画 */
//			hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
//			hPenOld = (HPEN)::SelectObject( hdc, hPen );
//			m_nOldUnderLineY = m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight;
//			::MoveToEx(
//				hdc,
//				m_nViewAlignLeft,
//				m_nOldUnderLineY,
//				NULL
//			);
//			::LineTo(
//				hdc,
//				m_nViewCx + m_nViewAlignLeft,
//				m_nOldUnderLineY
//			);
//			::SelectObject( hdc, hPenOld );
//			::DeleteObject( hPen );
//		}

		/* キャレットの作成 */
		::CreateCaret( m_hWnd, (HBITMAP)NULL, nCaretWidth, nCaretHeight );
		/* キャレットの位置を調整 */
		::SetCaretPos(
			m_nViewAlignLeft + (m_nCaretPosX - m_nViewLeftCol) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),
			m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight - nCaretHeight
		);
		/* キャレットの表示 */
		::ShowCaret( m_hWnd );
	}else{
		if( m_nCaretWidth != nCaretWidth || m_nCaretHeight != nCaretHeight ){	/* キャレットはあるが、大きさが変わった場合 */


			/* 現在のキャレットを削除 */
			::DestroyCaret();

//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && -1 != m_nOldUnderLineY ){
//				/* カーソル行アンダーラインの消去 */
//				hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
//				hPenOld = (HPEN)::SelectObject( hdc, hPen );
//				::MoveToEx(
//					hdc,
//					m_nViewAlignLeft,
//					m_nOldUnderLineY,
//					NULL
//				);
//				::LineTo(
//					hdc,
//					m_nViewCx + m_nViewAlignLeft,
//					m_nOldUnderLineY
//				);
//				::SelectObject( hdc, hPenOld );
//				::DeleteObject( hPen );
//				m_nOldUnderLineY = -1;
//			}

//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && !IsTextSelecting() ){
//				/* カーソル行アンダーラインの描画 */
//				hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
//				hPenOld = (HPEN)::SelectObject( hdc, hPen );
//				m_nOldUnderLineY = m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight;
//				::MoveToEx(
//					hdc,
//					m_nViewAlignLeft,
//					m_nOldUnderLineY,
//					NULL
//				);
//				::LineTo(
//					hdc,
//					m_nViewCx + m_nViewAlignLeft,
//					m_nOldUnderLineY
//				);
//				::SelectObject( hdc, hPenOld );
//				::DeleteObject( hPen );
//			}

			/* キャレットの作成 */
			::CreateCaret( m_hWnd, (HBITMAP)NULL, nCaretWidth, nCaretHeight );
			/* キャレットの位置を調整 */
			::SetCaretPos(
				m_nViewAlignLeft + (m_nCaretPosX - m_nViewLeftCol) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),
				m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight - nCaretHeight
			);
			/* キャレットの表示 */
			::ShowCaret( m_hWnd );
		}else{
			/* キャレットはあるし、大きさも変わっていない場合 */
			/* キャレットを隠す */
			::HideCaret( m_hWnd );

//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && -1 != m_nOldUnderLineY ){
//				/* カーソル行アンダーラインの消去 */
//				hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
//				hPenOld = (HPEN)::SelectObject( hdc, hPen );
//				::MoveToEx(
//					hdc,
//					m_nViewAlignLeft,
//					m_nOldUnderLineY,
//					NULL
//				);
//				::LineTo(
//					hdc,
//					m_nViewCx + m_nViewAlignLeft,
//					m_nOldUnderLineY
//				);
//				::SelectObject( hdc, hPenOld );
//				::DeleteObject( hPen );
//				m_nOldUnderLineY = -1;
//			}

//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && !IsTextSelecting() ){
//				/* カーソル行アンダーラインの描画 */
//				hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
//				hPenOld = (HPEN)::SelectObject( hdc, hPen );
//				m_nOldUnderLineY = m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight;
//				::MoveToEx(
//					hdc,
//					m_nViewAlignLeft,
//					m_nOldUnderLineY,
//					NULL
//				);
//				::LineTo(
//					hdc,
//					m_nViewCx + m_nViewAlignLeft,
//					m_nOldUnderLineY
//				);
//				::SelectObject( hdc, hPenOld );
//				::DeleteObject( hPen );
//			}

			/* キャレットの位置を調整 */
			::SetCaretPos(
				m_nViewAlignLeft + (m_nCaretPosX - m_nViewLeftCol) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),
				m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight - nCaretHeight
			);
			/* キャレットの表示 */
			::ShowCaret( m_hWnd );
		}
	}
	m_nCaretWidth = nCaretWidth;
	m_nCaretHeight = nCaretHeight;	/* キャレットの高さ */
	SetIMECompFormPos();

	/* ルーラー描画 */
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );


	return;
}





/* 入力フォーカスを受け取ったときの処理 */
void CEditView::OnSetFocus( void )
{
//NG	/* 1999.11.15 */
//NG	::SetFocus( m_hwndParent );
//NG	::SetFocus( m_hWnd );


	ShowEditCaret();
	SetIMECompFormPos();
	SetIMECompFormFont();

	return;
}





/* 入力フォーカスを失ったときの処理 */
void CEditView::OnKillFocus( void )
{
//	if( m_nMyIndex == 0 && m_nCaretPosX == 0 && m_nCaretPosY == 0 ){
//		MYTRACE( "OnKillFocus()\n" );
//	}
	HDC	hdc;


	::DestroyCaret();
	m_nCaretWidth = 0;

	/* ルーラー描画 */
	hdc = ::GetDC( m_hWnd );
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );

	/* 辞書Tipが起動されている */
	if( 0 == m_dwTipTimer ){
		/* 辞書Tipを消す */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
	}


	if( m_bHokan ){
		m_pcEditDoc->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

//	if( m_bHokan ){
//		m_pcEditDoc->m_cHokanMgr.Hide();
//		m_bHokan = FALSE;
//	}

	return;
}





/* 垂直スクロールバーメッセージ処理 */
void CEditView::OnVScroll( int nScrollCode, int nPos, HWND hwndScrollBar )
{
//	int		i;
	switch( nScrollCode ){
	case SB_LINEDOWN:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( m_nViewTopLine + 1 );
//		}
		ScrollAtV( m_nViewTopLine + m_pShareData->m_Common.m_nRepeatedScrollLineNum );

		break;
	case SB_LINEUP:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( m_nViewTopLine - 1 );
//		}
		ScrollAtV( m_nViewTopLine - m_pShareData->m_Common.m_nRepeatedScrollLineNum );
		break;
	case SB_PAGEDOWN:
		ScrollAtV( m_nViewTopLine + m_nViewRowNum );
		break;
	case SB_PAGEUP:
		ScrollAtV( m_nViewTopLine - m_nViewRowNum );
		break;
	case SB_THUMBPOSITION:
		ScrollAtV( nPos );
		break;
	case SB_THUMBTRACK:
		ScrollAtV( nPos );
		break;
	default:
		break;
	}
	return;
}




/* 水平スクロールバーメッセージ処理 */
void CEditView::OnHScroll( int nScrollCode, int nPos, HWND hwndScrollBar )
{
//	int		i;
	switch( nScrollCode ){
	case SB_LINELEFT:
//		for( i = 0; i < 2; ++i ){
			ScrollAtH( m_nViewLeftCol - 4 );
//		}
		break;
	case SB_LINERIGHT:
//		for( i = 0; i < 2; ++i ){
			ScrollAtH( m_nViewLeftCol + 4 );
//		}
		break;
	case SB_PAGELEFT:
		ScrollAtH( m_nViewLeftCol - m_nViewColNum );
		break;
	case SB_PAGERIGHT:
		ScrollAtH( m_nViewLeftCol + m_nViewColNum );
		break;
	case SB_THUMBPOSITION:
		ScrollAtH( nPos );
//		MYTRACE( "nPos=%d\n", nPos );
		break;
	case SB_THUMBTRACK:
		ScrollAtH( nPos );
//		MYTRACE( "nPos=%d\n", nPos );
		break;
	}
	return;
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
//	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );
//	return;
//}

#if 0//////////////////////////////////////////////
/************************************************************************
 *
 * 	関数:	OpenDIB( LPSTR szFile )
 *
 * 	目的:	DIBファイルを開いてメモリDIBを作成します。また、BITMAPINFO、
 *	パレットデータ、ビットを含むメモリハンドルを作成します。
 *
 *	戻り値:	DIBを識別するハンドル
 *
 ************************************************************************/
HANDLE CEditView::OpenDIB ( LPCSTR szFile )
{
	unsigned			fh;
	BITMAPINFOHEADER	bi;
	LPBITMAPINFOHEADER	lpbi;
	DWORD				dwLen = 0;
	DWORD				dwBits;
	HANDLE				hdib;
	HANDLE				h;
	OFSTRUCT			of;

	/* ファイルを開いてDIB情報を読み取る */
	fh = OpenFile( szFile, &of, OF_READ );
	if( fh == -1 )
		return NULL;

	hdib = ReadDibBitmapInfo( fh );
	if( !hdib )
		return NULL;
	DibInfo( hdib, &bi );

	/* DIBの保持に必要なメモリ量を計算 */
	dwBits = bi.biSizeImage;
	dwLen  = bi.biSize + (DWORD)PaletteSize( &bi ) + dwBits;

	/* DIBを保持するビットマップ情報バッファサイズを増やす */
	h = GlobalReAlloc( hdib, dwLen, GHND );
	if( !h ){
		GlobalFree( hdib );
		hdib = NULL;
	}
	else
		hdib = h;
	/* ビットを読み取る */
	if( hdib ){
		lpbi = (LPBITMAPINFOHEADER)GlobalLock( hdib );
		lread( fh, (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize(lpbi), dwBits );
		GlobalUnlock( hdib );
	}
	_lclose( fh );
	return hdib;
}

/************************************************************************
 *
 *	関数:	ReadDibBitmapInfo( int fh )
 *
 *	目的:	DIB形式のファイルを読み取り、そのBITMAPINFOの
 *	グローバルハンドルを返します。
 *	この関数は以前の(BITMAPCOREHEADER)形式と新しい(BITMAPINFOHEADER)形式の
 * 	両方を処理できますが、返すのはつねに新しいBITMAPINFOです。
 *
 *	 戻り値: ファイル内のDIBのBITMAPINFOを識別するハンドル
 *
 ************************************************************************/
HANDLE CEditView::ReadDibBitmapInfo ( int fh )
{
	DWORD				off;
	HANDLE				hbi = NULL;
	int					size;
	int					i;
	WORD				nNumColors;

	RGBQUAD FAR			*pRgb;
	BITMAPINFOHEADER	bi;
	BITMAPCOREHEADER	bc;
	LPBITMAPINFOHEADER	lpbi;
	BITMAPFILEHEADER	bf;
	DWORD				dwWidth = 0;
	DWORD				dwHeight = 0;
	WORD				wPlanes, wBitCount;

	if( fh == -1 )
		return NULL;

	/* ファイルポインタをリセットし、ファイルヘッダーを読み取る */
	off = _llseek( fh, 0L, SEEK_CUR );
	if( sizeof( bf ) != _lread( fh, (LPSTR)&bf, sizeof( bf ) ) )
		return FALSE;

	/* RCヘッダーがあるか調べる */
	if( !ISDIB( bf.bfType ) ){
		bf.bfOffBits = 0L;
		_llseek( fh, off, SEEK_SET );
	}
	if( sizeof( bi ) != _lread( fh, (LPSTR)&bi, sizeof( bi ) ) )
		return FALSE;

	nNumColors = DibNumColors( &bi );

	/*
	 * 情報ブロックの内容(BITMAPINFOまたはBITMAPCORE)をチェックし、
	 * それに従って情報を取得する。BITMAPCOREHEADERならば、情報を
	 * BITMAPINFOHEADER形式のブロックに転送する。
	 */
	switch ( size = (int)bi.biSize ){
	  case sizeof( BITMAPINFOHEADER ):
		break;
	  case  sizeof( BITMAPCOREHEADER ):

		bc = *(BITMAPCOREHEADER*)&bi;

		dwWidth		= (DWORD)bc.bcWidth;
		dwHeight	= (DWORD)bc.bcHeight;
		wPlanes		= bc.bcPlanes;
		wBitCount	= bc.bcBitCount;

		bi.biSize		= sizeof( BITMAPINFOHEADER );
		bi.biWidth		= dwWidth;
		bi.biHeight		= dwHeight;
		bi.biPlanes		= wPlanes;
		bi.biBitCount	= wBitCount;

		bi.biCompression	= BI_RGB;
		bi.biSizeImage	 	= 0;
		bi.biXPelsPerMeter	= 0;
		bi.biYPelsPerMeter	= 0;
		bi.biClrUsed		= nNumColors;
		bi.biClrImportant       = nNumColors;
		_llseek( fh, (LONG)sizeof( BITMAPCOREHEADER ) - sizeof( BITMAPINFOHEADER ), SEEK_CUR );
		break;
	  default:
		/* DIBではない */
		return NULL;
	}
	/* 0ならばデフォルト値を設定 */
	if( bi.biSizeImage == 0 ){
		bi.biSizeImage = WIDTHBYTES( (DWORD)bi.biWidth * bi.biBitCount ) * bi.biHeight;
	}
	if( bi.biClrUsed == 0 )
		bi.biClrUsed = DibNumColors(&bi);
	/* BITMAPINFO構造体とカラーテーブルを割り当てる */
	hbi = GlobalAlloc( GHND, (LONG)bi.biSize + nNumColors * sizeof( RGBQUAD ) );
	if( !hbi )
		return NULL;
	lpbi = (LPBITMAPINFOHEADER)GlobalLock( hbi );
	*lpbi = bi;
	/* カラーテーブルを指すポインタを取得 */
	pRgb = (RGBQUAD FAR *)( (LPSTR)lpbi + bi.biSize );
	if (nNumColors){
		if( size == sizeof( BITMAPCOREHEADER ) ){
			/*
			 * 古いカラーテーブル(3バイトのRGBTRIPLE)を新しいカラーテーブル(4バイトのRGBQUAD)に変換
			 */
			_lread( fh, (LPSTR)pRgb, nNumColors * sizeof( RGBTRIPLE ) );
			for( i = nNumColors - 1; i >= 0; i-- ){
				RGBQUAD rgb;
				rgb.rgbRed		= ((RGBTRIPLE FAR *)pRgb)[i].rgbtRed;
				rgb.rgbBlue		= ((RGBTRIPLE FAR *)pRgb)[i].rgbtBlue;
				rgb.rgbGreen	= ((RGBTRIPLE FAR *)pRgb)[i].rgbtGreen;
				rgb.rgbReserved = (BYTE)0;
				pRgb[i]			= rgb;
			}
		}
		else
			_lread( fh, (LPSTR)pRgb, nNumColors * sizeof( RGBQUAD ) );
	}
	if( bf.bfOffBits != 0L )
		_llseek( fh, off + bf.bfOffBits, SEEK_SET );
	GlobalUnlock( hbi );
	return hbi;
}


/************************************************************************
 *
 *	関数:	DibInfo( HANDLE hbi, LPBITMAPINFOHEADER lpbi )
 *
 *	目的:	CF_DIB形式のメモリブロックに関連付けられているDIB情報を取得します。
 *
 *  戻り値:	TRUE	- 正常に終了した場合
 *			FALSE	- それ以外の場合
 *
 ************************************************************************/
BOOL CEditView::DibInfo( HANDLE hbi, LPBITMAPINFOHEADER lpbi )
{
	if( hbi ){
		*lpbi = *(LPBITMAPINFOHEADER)GlobalLock( hbi );

		/* デフォルトのメンバ設定 */
		if( lpbi->biSize != sizeof( BITMAPCOREHEADER ) ){
			if( lpbi->biSizeImage == 0L )
				lpbi->biSizeImage =
					WIDTHBYTES( lpbi->biWidth*lpbi->biBitCount ) * lpbi->biHeight;

			if( lpbi->biClrUsed == 0L )
				lpbi->biClrUsed = DibNumColors( lpbi );
		}
		GlobalUnlock( hbi );
		return TRUE;
	}
	return FALSE;
}

/********************************************************************************
 *
 *	関数:	PaletteSize( VOID FAR * pv )
 *
 *	目的:	パレットのバイト数を計算します。情報ブロックがBITMAPCOREHEADER型
 *			ならば、色数の3倍がパレットサイズになります。それ以外の場合は、
 *			色数の4倍がパレットサイズになります。
 *
 *	戻り値:	パレットのバイト数
 *
 *******************************************************************************/
WORD CEditView::PaletteSize ( VOID FAR * pv )
{
	LPBITMAPINFOHEADER lpbi;
	WORD		NumColors;
	lpbi		= (LPBITMAPINFOHEADER)pv;
	NumColors	= DibNumColors(lpbi);

	if( lpbi->biSize == sizeof( BITMAPCOREHEADER ) )
		return NumColors * sizeof( RGBTRIPLE );
	else
		return NumColors * sizeof( RGBQUAD );
}





/********************************************************************************
 *
 *	関数:	DibNumColors( VOID FAR * pv )
 *
 *	目的:	情報ブロックのBitCountメンバを参照して、DIBの色数を判断します。
 *
 *	戻り値:	DIBの色数
 *
 *******************************************************************************/
WORD CEditView::DibNumColors ( VOID FAR * pv)
{
	int					bits;
	LPBITMAPINFOHEADER lpbi;
	LPBITMAPCOREHEADER lpbc;
	lpbi = ( (LPBITMAPINFOHEADER)pv );
	lpbc = ( (LPBITMAPCOREHEADER)pv );
	/*
	 *	BITMAPINFO形式ヘッダーの場合、パレットのサイズはBITMAPCORE形式の
	 *	ヘッダーのbiClrUsedが示している。パレットのサイズは、ピクセル当たり
	 *	のビット数により異なる。
	 */
	if( lpbi->biSize != sizeof( BITMAPCOREHEADER ) ){
		if( lpbi->biClrUsed != 0 )
			return (WORD)lpbi->biClrUsed;
		bits = lpbi->biBitCount;
	}
	else
		bits = lpbc->bcBitCount;

	switch ( bits ){
		case 1:	return 2;
		case 4:	return 16;
		case 8:	return 256;
		/* 24ビットDIBにはカラーテーブルはない */
		default:	return 0;
	}
}

 /********** 64Kバイト以上の読み書きを行うプライベートルーチン *********/
/************************************************************************
 *
 *	関数:	lread( int fh, VOID FAR *pv, DWORD ul )
 *
 *	目的:	データをすべて読み取るまで32Kバイトずつデータを読み取ります。
 *
 *	戻り値:	0			- 正常に読み取れなかった場合
 *	読み取ったバイト数	- それ以外の場合
 *
 ************************************************************************/
DWORD CEditView::lread( int fh, void* pv, DWORD ul )
{
	DWORD	ulT = ul;
	BYTE	*hp = (BYTE *)pv;

#define	 MAXREAD_BYTES	32768	/* 読み取り処理時の読み取り可能なバイト数 */

	while( ul > (DWORD)MAXREAD_BYTES ){
		if( _lread( fh, (LPSTR)hp, (WORD)MAXREAD_BYTES ) != MAXREAD_BYTES )
			return 0;
		ul -= MAXREAD_BYTES;
		hp += MAXREAD_BYTES;
	}
	if( _lread(fh, (LPSTR)hp, (WORD)ul) != (WORD)ul )
		return 0;
	return ulT;
}


/************************************************************************
 * 関数:  PrintBitmap(int , int , int , int , LPCSTR );
 *
 *	目的:	指定したDIBファイルを読み込んで指定文字桁範囲に伸縮印刷します。
 *			印刷位置は現在のフォントサイズに影響されます。
 *
 *	戻り値:	なし
 *
 ************************************************************************/
void CEditView::PrintBitmap( HDC hdc, int x1, int y1, const char* szFile )
{
	HANDLE				hdib;
	BITMAPINFOHEADER	bi;
	LPBITMAPINFOHEADER	lpbi;
	LPSTR				pBuf;

	/* DIBファイルを開いてメモリDIBを作成 */
	hdib = OpenDIB ( szFile );
	if( hdib == NULL ){
//		MYTRACE( " OpenDIB()の実行に失敗  \n" );
		return;
	}
	DibInfo( hdib, &bi );

	/* DIBをプリンタDCに伸縮して転送 */
	lpbi = (LPBITMAPINFOHEADER)GlobalLock( hdib );
	if( !lpbi ){
		GlobalFree( (HGLOBAL)hdib );
		return ;
	}
	pBuf = (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize( lpbi );
	StretchDIBits ( hdc,
		x1,
		y1,
		bi.biWidth,
		bi.biHeight,
		0,
		0,
		bi.biWidth,
		bi.biHeight,
		pBuf,
		(LPBITMAPINFO)lpbi,
		DIB_RGB_COLORS,
		SRCCOPY
	);
	GlobalUnlock( hdib );
	GlobalFree( hdib );
	return;
}

#endif //////////////////////////////////////////////






/*
プリプロセッサ ディレクティブ

#define	#endif	#ifdef	#line
#elif	#error	#ifndef	#pragma
#else	#if	#include	#undef



Cキーワード

auto	double	int	struct
break	else	long	switch
case	enum	register	typedef
char	extern	return	union
const	float	short	unsigned
continue	for	signed	void
default		goto	sizeof	volatile
do	if	static	while



C++キーワード

asm	auto	bad_cast	bad_typeid
break	case	catch	char
class	const	const_cast	continue
default	delete	do	double
dynamic_cast	else	enum	except
extern	finally	float	for
friend	goto	if	inline
int	long	namespace	new
operator	private	protected	public
register	reinterpret_cast	return	short
signed	sizeof	static	static_cast
struct	switch	template	this
throw	try	type_info	typedef
typeid	union	unsigned	using
virtual	void	volatile	while
xalloc


C++演算子

::	スコープ解決	なし
::	グローバル	なし
[ ]	配列添字	左から右
( )	関数呼び出し	左から右
( )	変換	なし
.	メンバ選択 (オブジェクト)	左から右
->	メンバ選択 (ポインタ)	左から右
++	後置インクリメント	なし
--	後置デクリメント	なし
new	オブジェクト割り当て	なし
delete	オブジェクト解放	なし
delete[ ]	オブジェクト解放	なし
++	前置インクリメント	なし
--	前置デクリメント	なし
*	参照	なし
&	アドレス	なし
+	単項プラス	なし
-	算術否定 (単項)	なし
!	論理 NOT	なし
~	ビットごとの補数	なし
sizeof	オブジェクトのサイズ	なし
sizeof ( )	型のサイズ	なし
typeid( )	型名	なし
(type)	型キャスト (変換)	右から左
const_cast	型キャスト (変換)	なし
dynamic_cast	型キャスト (変換)	なし
reinterpret_cast	型キャスト (変換)	なし
static_cast	型キャスト (変換)	なし
.*	クラス メンバへの適用ポインタ (オブジェクト)	左から右
->*	ポインタを介した、クラス メンバへの逆参照ポインタ	左から右
*	乗算	左から右
/	除算	左から右
%	剰余 (モジュール)	左から右
+	加算	左から右
-	減算	左から右
<<	左シフト	左から右
>>	右シフト	左から右
<	小なり	左から右
>	大なり	左から右
<=	以下	左から右
>=	以上	左から右
==	等価	左から右
!=	不等価	左から右
&	ビットごとの AND	左から右
^	ビットごとの排他的 OR	左から右
|	ビットごとの OR	左から右
&&	論理 AND	左から右
||	論理 OR	左から右
e1?e2:e3	条件	右から左
=	代入	右から左
*=	乗算代入	右から左
/=	除算代入	右から左
%=	剰余代入	右から左
+=	加算代入	右から左
-=	減算代入	右から左
<<=	左シフト代入	右から左
>>=	右シフト代入	右から左
&=	ビットごとの AND 代入	右から左
|=	ビットごとの OR 代入	右から左
^=	ビットごとの排他的 OR 代入	右から左
, 	カンマ	左から右


*/


/* 2点を対角とする矩形を求める */
void CEditView::TwoPointToRect(
		RECT*	prcRect,
	int		nLineFrom,
	int		nColmFrom,
	int		nLineTo,
	int		nColmTo
)
{
	if( nLineFrom < nLineTo ){
		prcRect->top	= nLineFrom;
		prcRect->bottom	= nLineTo;
	}else{
		prcRect->top	= nLineTo;
		prcRect->bottom	= nLineFrom;
	}
	if( nColmFrom < nColmTo ){
		prcRect->left	= nColmFrom;
		prcRect->right	= nColmTo;
	}else{
		prcRect->left	= nColmTo;
		prcRect->right	= nColmFrom;
	}
	return;

}

#if 0//////////////////////////////////////////
/* デバッグ用 リージョン矩形のダンプ */
void CEditView::TraceRgn( HRGN hrgn )
{
//	unsigned int	i;
	char*			pBuf;
	int				nRgnDataSize;
	RGNDATA*		pRgnData;
	RECT*			pRect;
	nRgnDataSize = ::GetRegionData( hrgn, 0, NULL );
	pBuf = new char[nRgnDataSize];
	pRgnData = (RGNDATA*)pBuf;
	nRgnDataSize = ::GetRegionData( hrgn, nRgnDataSize, pRgnData );
	pRect = (RECT*)&pRgnData->Buffer[0];
//	if( 0 < pRgnData->rdh.nCount ){
//		m_cShareData.TraceOut( "---------\n" );;
//		for( i = 0; i < pRgnData->rdh.nCount; ++i ){
//			m_cShareData.TraceOut( "\t(%d, %d, %d, %d \n", pRect[i].left, pRect[i].right, pRect[i].top, pRect[i].bottom );;
//		}
//	}
	delete [] pBuf;
	return;
}
#endif //#if 0




/* 選択領域の描画 */
void CEditView::DrawSelectArea( void )
{
	/* カーソル行アンダーラインのOFF */
	CaretUnderLineOFF( TRUE );
	if( !m_bDrawSWITCH ){
		return;
	}

	int			nFromLine;
	int			nFromCol;
	int			nToLine;
	int			nToCol;
	HDC			hdc;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	int			nROP_Old;
	int			nLineNum;
	RECT		rcOld;
	RECT		rcNew;
	HRGN		hrgnOld = NULL;
	HRGN		hrgnNew = NULL;
	HRGN		hrgnDraw = NULL;

//	MYTRACE( "DrawSelectArea()  m_bBeginBoxSelect=%s\n", m_bBeginBoxSelect?"TRUE":"FALSE" );
	if( m_bBeginBoxSelect ){		/* 矩形範囲選択中 */
		hdc = ::GetDC( m_hWnd );
		hBrush = ::CreateSolidBrush( SELECTEDAREA_RGB );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
		nROP_Old = ::SetROP2( hdc, SELECTEDAREA_ROP2 );

		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcOld,
			m_nSelectLineFromOld,	/* 範囲選択開始行 */
			m_nSelectColmFromOld,	/* 範囲選択開始桁 */
			m_nSelectLineToOld,		/* 範囲選択終了行 */
			m_nSelectColmToOld		/* 範囲選択終了桁 */
		);
		if( rcOld.left	< m_nViewLeftCol ){
			rcOld.left = m_nViewLeftCol;
		}
		if( rcOld.right	< m_nViewLeftCol ){
			rcOld.right = m_nViewLeftCol;
		}
		if( rcOld.right > m_nViewLeftCol + m_nViewColNum + 1 ){
			rcOld.right = m_nViewLeftCol + m_nViewColNum + 1;
		}
		if( rcOld.top < m_nViewTopLine ){
			rcOld.top = m_nViewTopLine;
		}
		if( rcOld.bottom > m_nViewTopLine + m_nViewRowNum ){
			rcOld.bottom = m_nViewTopLine + m_nViewRowNum;
		}
		rcOld.left		= (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + rcOld.left  * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		rcOld.right		= (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + rcOld.right * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		rcOld.top		= ( rcOld.top - m_nViewTopLine ) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
		rcOld.bottom	= ( rcOld.bottom + 1 - m_nViewTopLine ) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
		hrgnOld = ::CreateRectRgnIndirect( &rcOld );

		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcNew,
			m_nSelectLineFrom,		/* 範囲選択開始行 */
			m_nSelectColmFrom,		/* 範囲選択開始桁 */
			m_nSelectLineTo,		/* 範囲選択終了行 */
			m_nSelectColmTo			/* 範囲選択終了桁 */
		);
		if( rcNew.left	< m_nViewLeftCol ){
			rcNew.left = m_nViewLeftCol;
		}
		if( rcNew.right	< m_nViewLeftCol ){
			rcNew.right = m_nViewLeftCol;
		}
		if( rcNew.right > m_nViewLeftCol + m_nViewColNum + 1 ){
			rcNew.right = m_nViewLeftCol + m_nViewColNum + 1;
		}
		if( rcNew.top < m_nViewTopLine ){
			rcNew.top = m_nViewTopLine;
		}
		if( rcNew.bottom > m_nViewTopLine + m_nViewRowNum ){
			rcNew.bottom = m_nViewTopLine + m_nViewRowNum;
		}
		rcNew.left		= (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + rcNew.left  * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		rcNew.right		= (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + rcNew.right * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		rcNew.top		= ( rcNew.top - m_nViewTopLine ) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
		rcNew.bottom	= ( rcNew.bottom + 1 - m_nViewTopLine ) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
		hrgnNew = ::CreateRectRgnIndirect( &rcNew );

		/* ::CombineRgn()の結果を受け取るために、適当なリージョンを作る */
		hrgnDraw = ::CreateRectRgnIndirect( &rcNew );

		if( rcNew.left <= rcNew.right ){
			/* 旧選択矩形と新選択矩形のリージョンを結合し､ 重なりあう部分だけを除去します */
			if( NULLREGION != ::CombineRgn( hrgnDraw, hrgnOld, hrgnNew, RGN_XOR ) ){
				::PaintRgn( hdc, hrgnDraw );
			}
		}else{
			hrgnDraw = hrgnOld;
			::PaintRgn( hdc, hrgnDraw );

		}

		//////////////////////////////////////////
		/* デバッグ用 リージョン矩形のダンプ */
//@@		TraceRgn( hrgnDraw );


		if( NULL != hrgnDraw ){
			::DeleteObject( hrgnDraw );
		}
		if( NULL != hrgnNew ){
			::DeleteObject( hrgnNew );
		}
		if( NULL != hrgnOld ){
			::DeleteObject( hrgnOld );
		}
		::SetROP2( hdc, nROP_Old );
		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( m_hWnd, hdc );
	}else{
		hdc = ::GetDC( m_hWnd );
		hBrush = ::CreateSolidBrush( SELECTEDAREA_RGB );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
		nROP_Old = ::SetROP2( hdc, SELECTEDAREA_ROP2 );

		/* 現在描画されている範囲と始点が同じ */
		if( m_nSelectLineFrom == m_nSelectLineFromOld &&
			m_nSelectColmFrom  == m_nSelectColmFromOld ){
			/* 範囲が後方に拡大された */
			if( m_nSelectLineTo > m_nSelectLineToOld ||
			    (m_nSelectLineTo == m_nSelectLineToOld &&
				m_nSelectColmTo > m_nSelectColmToOld ) ){
				nFromLine	= m_nSelectLineToOld;
				nFromCol	= m_nSelectColmToOld;
				nToLine		= m_nSelectLineTo;
				nToCol		= m_nSelectColmTo;
			}else{
				nFromLine	= m_nSelectLineTo;
				nFromCol	= m_nSelectColmTo;
				nToLine		= m_nSelectLineToOld;
				nToCol		= m_nSelectColmToOld;
			}
			for( nLineNum = nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
		}else
		if( m_nSelectLineTo == m_nSelectLineToOld &&
			m_nSelectColmTo  == m_nSelectColmToOld ){
			/* 範囲が前方に拡大された */
			if( m_nSelectLineFrom < m_nSelectLineFromOld ||
			    (m_nSelectLineFrom == m_nSelectLineFromOld &&
				m_nSelectColmFrom < m_nSelectColmFromOld ) ){
				nFromLine	= m_nSelectLineFrom;
				nFromCol	= m_nSelectColmFrom;
				nToLine		= m_nSelectLineFromOld;
				nToCol		= m_nSelectColmFromOld;
			}else{
				nFromLine	= m_nSelectLineFromOld;
				nFromCol	= m_nSelectColmFromOld;
				nToLine		= m_nSelectLineFrom;
				nToCol		= m_nSelectColmFrom;
			}
			for( nLineNum = nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
		}else{
			nFromLine		= m_nSelectLineFromOld;
			nFromCol		= m_nSelectColmFromOld;
			nToLine			= m_nSelectLineToOld;
			nToCol			= m_nSelectColmToOld;
			for( nLineNum	= nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
			nFromLine	= m_nSelectLineFrom;
			nFromCol	= m_nSelectColmFrom;
			nToLine		= m_nSelectLineTo;
			nToCol		= m_nSelectColmTo;
			for( nLineNum = nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
		}
		::SetROP2( hdc, nROP_Old );
		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( m_hWnd, hdc );
	}
	return;
}




/* 指定行の選択領域の描画 */
void CEditView::DrawSelectAreaLine(
		HDC hdc, int nLineNum, int nFromLine, int nFromCol, int nToLine, int nToCol
)
{
//	MYTRACE( "CEditView::DrawSelectAreaLine()\n" );
	HRGN			hrgnDraw;
	const char*		pLine;
	int				nLineLen;
	int				i;
	int				nCharChars;
	int				nPosX;
	RECT			rcClip;
	int				nSelectFrom;
	int				nSelectTo;
	const CLayout*	pcLayout;
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( nLineNum, &nLineLen, &pcLayout );
	if( NULL == pLine ){
		nLineLen = 0;
	}
	nPosX = 0;
	for( i = 0; i < nLineLen; ){
//		if( i == nLineLen - 1 && ( pLine[i] == '\n' || pLine[i] == '\r' ) ){
//		if( i >= nLineLen - pcLayout->m_cEol.GetLen() ){
		if( i >= nLineLen - (pcLayout->m_cEol.GetLen()?1:0 ) ){
			++nPosX;
			i = nLineLen;
			break;
		}
		if( pLine[i] == TAB ){
			nCharChars = m_pcEditDoc->GetDocumentAttribute().m_nTabSpace - ( nPosX % m_pcEditDoc->GetDocumentAttribute().m_nTabSpace );
			++i;
		}else{
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];

			if( 0 == nCharChars ){
				nCharChars = 1;
			}
			i+= nCharChars;
		}
		nPosX += nCharChars;
	}
	if( nFromLine == nToLine ){
			nSelectFrom = nFromCol;
			nSelectTo   = nToCol;
	}else{
		if( nLineNum == nFromLine ){
			nSelectFrom = nFromCol;
			nSelectTo   = nPosX;
		}else
		if( nLineNum == nToLine ){
			nSelectFrom = 0;
			nSelectTo   = nToCol;
		}else{
			nSelectFrom = 0;
			nSelectTo   = nPosX;
		}
	}
	if( nSelectFrom < m_nViewLeftCol ){
		nSelectFrom = m_nViewLeftCol;
	}
	if( nSelectTo < m_nViewLeftCol ){
		nSelectTo = m_nViewLeftCol;
	}
	rcClip.left		= (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + nSelectFrom * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	rcClip.right	= (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + nSelectTo   * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	rcClip.top		= ( nLineNum - m_nViewTopLine ) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
	rcClip.bottom	= rcClip.top + m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
	if( rcClip.right - rcClip.left > 3000 ){
		rcClip.right = rcClip.left + 3000;
	}
//	::Rectangle( hdc, rcClip.left, rcClip.top, rcClip.right + 1, rcClip.bottom + 1 );

	hrgnDraw = ::CreateRectRgn( rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
	::PaintRgn( hdc, hrgnDraw );
	::DeleteObject( hrgnDraw );


//	::Rectangle( hdc, rcClip.left, rcClip.top, rcClip.right + 1, rcClip.bottom + 1);
//	::FillRect( hdc, &rcClip, hBrushTextCol );

//	//	/* デバッグモニタに出力 */
//	m_cShareData.TraceOut( "DrawSelectAreaLine() rcClip.left=%d, rcClip.top=%d, rcClip.right=%d, rcClip.bottom=%d\n", rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );

	return;
}





/* テキストが選択されているか */
BOOL CEditView::IsTextSelected( void )
{
	if( m_nSelectLineFrom	== -1 ||
		m_nSelectLineTo		== -1 ||
		m_nSelectColmFrom	== -1 ||
		m_nSelectColmTo		== -1
	){
//	if( m_nSelectLineFrom == m_nSelectLineTo &&
//		m_nSelectColmFrom  == m_nSelectColmTo ){
		return FALSE;
	}
	return TRUE;
}


/* テキストの選択中か */
BOOL CEditView::IsTextSelecting( void )
{
	if( m_bBeginSelect ||
		IsTextSelected()
	){
//		MYTRACE( "m_bBeginSelect=%d IsTextSelected()=%d TRUE==IsTextSelecting()\n", m_bBeginSelect, IsTextSelected() );
		return TRUE;
	}
//	MYTRACE( "m_bBeginSelect=%d IsTextSelected()=%d FALSE==IsTextSelecting()\n", m_bBeginSelect, IsTextSelected() );
	return FALSE;
}


/* フォントの変更 */
void CEditView::SetFont( void )
{
	HDC			hdc;
	HFONT		hFontOld;
	TEXTMETRIC	tm;
	int			i;
	SIZE		sz;

	hdc = ::GetDC( m_hWnd );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
//	hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
	::GetTextMetrics( hdc, &tm );


// 1999.12.9
//	m_nCharWidth = tm.tmAveCharWidth - 1;
//	m_nCharHeight = tm.tmHeight + tm.tmExternalLeading;
	/* 文字の大きさを調べる */
// 2000.2.8
//	::GetTextExtentPoint32( hdc, "X", 1, &sz );
//	m_nCharHeight = sz.cy;
//	m_nCharWidth = sz.cx;
	::GetTextExtentPoint32( hdc, "大", 2, &sz );
	m_nCharHeight = sz.cy;
	m_nCharWidth = sz.cx / 2;


// 行の高さを2の倍数にする
//	if( ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) % 2 ){
//		++m_nCharHeight;
//	}

	m_nViewColNum = m_nViewCx / ( m_nCharWidth  + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );	/* 表示域の桁数 */
	m_nViewRowNum = m_nViewCy / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );	/* 表示域の行数 */
	/* 行番号表示に必要な幅を設定 */
	DetectWidthOfLineNumberArea( FALSE );
	/* 文字列描画用文字幅配列 */
	for( i = 0; i < ( sizeof(m_pnDx) / sizeof(m_pnDx[0]) ); ++i ){
		m_pnDx[i] = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	}
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( m_hWnd, hdc );
	::InvalidateRect( m_hWnd, NULL, TRUE );
	if( m_nCaretWidth == 0 ){	/* キャレットがなかった場合 */
	}else{
		OnKillFocus();
		OnSetFocus();
	}
	return;
}



/* 行番号表示に必要な桁数を計算 */
int CEditView::DetectWidthOfLineNumberArea_calculate( void )
{
	int			i;
	int			nAllLines;
	int			nWork;
	if( m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF ){	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		nAllLines = m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	}else{
		nAllLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	}
	if( 0 < nAllLines ){
		nWork = 100;
		for( i = 3; i < 12; ++i ){
			if( ( nWork - 1 ) / nAllLines >= 1 ){
				break;
			}
			nWork *= 10;
		}
	}else{
		i = 1;
	}
	return i;

}


/*
行番号表示に必要な幅を設定幅が変更された場合はTRUEを返す
*/
BOOL CEditView::DetectWidthOfLineNumberArea( BOOL bRedraw )
{
	int				i;
	PAINTSTRUCT		ps;
	HDC				hdc;
//	int				nAllLines;
//	int				nWork;
	int				m_nViewAlignLeftNew;
	int				nCxVScroll;
	RECT			rc;

	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_GYOU].m_bDisp ){
		/* 行番号表示に必要な桁数を計算 */
		i = DetectWidthOfLineNumberArea_calculate();
		m_nViewAlignLeftNew = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) * (i + 1);	/* 表示域の左端座標 */
		m_nViewAlignLeftCols = i + 1;
	}else{
		m_nViewAlignLeftNew = 8;
		m_nViewAlignLeftCols = 0;
	}
	if( m_nViewAlignLeftNew != m_nViewAlignLeft ){
		m_nViewAlignLeft = m_nViewAlignLeftNew;
		::GetClientRect( m_hWnd, &rc );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
		m_nViewCx = (rc.right - rc.left) - nCxVScroll - m_nViewAlignLeft;	/* 表示域の幅 */


		if( bRedraw ){
			/* 再描画 */
			hdc = ::GetDC( m_hWnd );
			ps.rcPaint.left = 0;
			ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
			ps.rcPaint.top = 0;
			ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			OnKillFocus();
			OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
			OnSetFocus();
			::ReleaseDC( m_hWnd, hdc );
		}
		return TRUE;
	}else{
		return FALSE;
	}
}





/* スクロールバーの状態を更新する */
void CEditView::AdjustScrollBars( void )
{
	if( !m_bDrawSWITCH ){
		return;
	}


	int			nAllLines;
	int			nVScrollRate;
	SCROLLINFO	si;
//	int			nNowPos;

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
		si.fMask = SIF_ALL;
		si.nMin  = 0;
		si.nMax  = nAllLines / nVScrollRate - 1;	/* 全行数 */
		si.nPage = m_nViewRowNum / nVScrollRate;	/* 表示域の行数 */
		si.nPos  = m_nViewTopLine / nVScrollRate;	/* 表示域の一番上の行(0開始) */
		si.nTrackPos = nVScrollRate;
		::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
		m_nVScrollRate = nVScrollRate;				/* 垂直スクロールバーの縮尺 */
	}
	if( NULL != m_hwndHScrollBar ){
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL;

//@@		::GetScrollInfo( m_hwndHScrollBar, SB_CTL, &si );
//@@		if( si.nMax == m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize - 1
//@@		 && si.nPage == (UINT)m_nViewColNum
//@@		 && si.nPos  == m_nViewLeftCol
//@@	   /*&& si.nTrackPos == 1*/ ){
//@@		}else{
			/* 水平スクロールバー */
			si.cbSize = sizeof( si );
			si.fMask = SIF_ALL;
			si.nMin  = 0;
			si.nMax  = m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize - 1;	/* 折り返し文字数 */
			si.nPage = m_nViewColNum;		/* 表示域の桁数 */
			si.nPos  = m_nViewLeftCol;		/* 表示域の一番左の桁(0開始) */
			si.nTrackPos = 1;
			::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );
//@@		}
	}

	return;
}





/*
||
|| 行桁指定によるカーソル移動
|| 必要に応じて縦/横スクロールもする
|| 垂直スクロールをした場合はその行数を返す（正／負）
||
*/
int CEditView::MoveCursor( int nWk_CaretPosX, int nWk_CaretPosY, BOOL bDraw, int nCaretMarginRate )
{
	/* スクロール処理 */
	int		nScrollRowNum = 0;
	int		nScrollColNum = 0;
	RECT	rcScrol;
	RECT	rcClip;
	RECT	rcClip2;
//	int		nIndextY = 8;
	int		nCaretMarginY;
	HDC		hdc;
//	HPEN	hPen, hPenOld;
	int		nScrollMarginRight;
	int		nScrollMarginLeft;

	if( 0 >= m_nViewColNum ){
		return 0;
	}
	hdc = ::GetDC( m_hWnd );

	/* カーソル行アンダーラインのOFF */
	CaretUnderLineOFF( bDraw );

	nCaretMarginY = m_nViewRowNum / nCaretMarginRate;
	if( m_bBeginSelect ){	/* 範囲選択中 */
		nCaretMarginY = 0;
	}else{
		if( 1 > nCaretMarginY ){
			nCaretMarginY = 1;
		}
	}
	if( nWk_CaretPosY > m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
		nWk_CaretPosY = m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1;
		if( nWk_CaretPosY < 0 ){
			nWk_CaretPosY = 0;
		}
	}

	/* 水平スクロール量（文字数）の算出 */
	nScrollColNum = 0;
	nScrollMarginRight = 4;
	nScrollMarginLeft = 4;
	if( m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize > m_nViewColNum &&
		nWk_CaretPosX > m_nViewLeftCol + m_nViewColNum - nScrollMarginRight ){
		nScrollColNum =
			( m_nViewLeftCol + m_nViewColNum - nScrollMarginRight ) - nWk_CaretPosX;
	}else
	if( 0 < m_nViewLeftCol &&
		nWk_CaretPosX < m_nViewLeftCol + nScrollMarginLeft
	){
		nScrollColNum = m_nViewLeftCol + nScrollMarginLeft - nWk_CaretPosX;
		if( 0 > m_nViewLeftCol - nScrollColNum ){
			nScrollColNum = m_nViewLeftCol;
		}

	}

	m_nViewLeftCol -= nScrollColNum;
//	if( 0 > m_nViewLeftCol ){
//		m_nViewLeftCol = 0;
//	}

//#ifdef _DEBUG
//	if( m_nMyIndex == 0 ){
//		MYTRACE( "★★m_nViewLeftCol=%d\n", m_nViewLeftCol );
//	}
//#endif

	/* 垂直スクロール量（行数）の算出 */
	if( nWk_CaretPosY < m_nViewTopLine + ( nCaretMarginY ) ){
		if( nWk_CaretPosY < ( nCaretMarginY ) ){
			nScrollRowNum = m_nViewTopLine;
		}else{
			nScrollRowNum = m_nViewTopLine + ( nCaretMarginY ) - nWk_CaretPosY;
		}
	}else
	if( nWk_CaretPosY >= m_nViewTopLine + m_nViewRowNum - ( nCaretMarginY + 2 ) ){
		if( nWk_CaretPosY > m_pcEditDoc->m_cLayoutMgr.GetLineCount() - ( nCaretMarginY + 2 ) ){
			if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 > m_nViewRowNum ){
				nScrollRowNum = m_nViewTopLine - ( m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 - m_nViewRowNum );
			}else{
				nScrollRowNum = 0;
			}
		}else{
			nScrollRowNum =
				m_nViewTopLine + m_nViewRowNum - ( nCaretMarginY ) - ( nWk_CaretPosY + 2 );
		}
	}
	if( bDraw ){
		/* スクロール */
		if( abs( nScrollColNum ) >= m_nViewColNum ||
			abs( nScrollRowNum ) >= m_nViewRowNum ){
			m_nViewTopLine -= nScrollRowNum;
			if( bDraw ){
				::InvalidateRect( m_hWnd, NULL, TRUE );
				/* スクロールバーの状態を更新する */
				AdjustScrollBars();
			}
		}else
		if( nScrollRowNum != 0 || nScrollColNum != 0 ){
			rcScrol.left = 0;
			rcScrol.right = m_nViewCx + m_nViewAlignLeft;
			rcScrol.top = m_nViewAlignTop;
			rcScrol.bottom = m_nViewCy + m_nViewAlignTop;
			if( nScrollRowNum > 0 ){
				rcScrol.bottom =
					m_nViewCy + m_nViewAlignTop -
					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
				m_nViewTopLine -= nScrollRowNum;
				rcClip.left = 0;
				rcClip.right = m_nViewCx + m_nViewAlignLeft;
				rcClip.top = m_nViewAlignTop;
				rcClip.bottom =
					m_nViewAlignTop + nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			}else
			if( nScrollRowNum < 0 ){
				rcScrol.top =
					m_nViewAlignTop - nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
				m_nViewTopLine -= nScrollRowNum;
				rcClip.left = 0;
				rcClip.right = m_nViewCx + m_nViewAlignLeft;
				rcClip.top =
					m_nViewCy + m_nViewAlignTop +
					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
				rcClip.bottom = m_nViewCy + m_nViewAlignTop;
			}
			if( nScrollColNum > 0 ){
				rcScrol.left = m_nViewAlignLeft;
				rcScrol.right =
					m_nViewCx + m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.left = m_nViewAlignLeft;
				rcClip2.right = m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.top = m_nViewAlignTop;
				rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
			}else
			if( nScrollColNum < 0 ){
				rcScrol.left = m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.left =
					m_nViewCx + m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.right = m_nViewCx + m_nViewAlignLeft;
				rcClip2.top = m_nViewAlignTop;
				rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
			}
			if( m_bDrawSWITCH ){
//				::ScrollWindow(
//					m_hWnd,		/* スクロールするウィンドウのハンドル */
//					nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),	/* 水平スクロール量 */
//					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),	/* 垂直スクロール量 */
//					&rcScrol,	/* スクロール長方形の構造体のアドレス */
//					NULL		/* クリッピング長方形の構造体のアドレス */
//				);
				::ScrollWindowEx(
					m_hWnd,
					nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),	/* 水平スクロール量 */
					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),	/* 垂直スクロール量 */
					&rcScrol,	/* スクロール長方形の構造体のアドレス */
					NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
				);

				if( nScrollRowNum != 0 ){
					::InvalidateRect( m_hWnd, &rcClip, TRUE );
					if( nScrollColNum != 0 ){
						rcClip.left = 0;
						rcClip.right = m_nViewAlignLeft;
						rcClip.top = 0;
						rcClip.bottom = m_nViewCy + m_nViewAlignTop;
						::InvalidateRect( m_hWnd, &rcClip, TRUE );
					}
				}
				if( nScrollColNum != 0 ){
					::InvalidateRect( m_hWnd, &rcClip2, TRUE );
	 			}
				::UpdateWindow( m_hWnd );
			}
			/* スクロールバーの状態を更新する */
			AdjustScrollBars();
		}
	}

	/* キャレット移動 */
	m_nCaretPosX = nWk_CaretPosX;
	m_nCaretPosY = nWk_CaretPosY;

	/* カーソル位置変換
	||  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	||  →物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nCaretPosX,
		m_nCaretPosY,
		&m_nCaretPosX_PHY,	/* カーソル位置 改行単位行先頭からのバイト数(０開始) */
		&m_nCaretPosY_PHY	/* カーソル位置 改行単位行の行番号(０開始) */
	);

	/* カーソル行アンダーラインのON */
	CaretUnderLineON( bDraw );
	if( bDraw ){
		/* キャレットの表示・更新 */
		ShowEditCaret();

		/* キャレットの行桁位置を表示する */
		DrawCaretPosInfo();
	}
	::ReleaseDC( m_hWnd, hdc );

//	/*
//	|| 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す
//	*/
//	CLayout*	pCLayout;
//	pCLayout = (CLayout*)m_pcEditDoc->m_cLayoutMgr.GetLineData( m_nCaretPosY );
//	if( NULL != pCLayout ){
//		pCLayout->DUMP();
//
//	}

//	/*
//	  カーソル位置変換
//	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
//	  → 物理位置(行頭からのバイト数、折り返し無し行位置)
//	*/
//	int		nX;
//	int		nY;
//	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//		m_nCaretPosX,
//		m_nCaretPosY,
//		&nX,
//		&nY
//	);
//	MYTRACE( "■nX=%d,nY=%d ", nX, nY );
//	/*
//	  カーソル位置変換
//	  物理位置(行頭からのバイト数、折り返し無し行位置)
//	  →レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
//	*/
//	int		nPosX2;
//	int		nPosY2;
//	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
//		nX,
//		nY,
//		&nPosX2,
//		&nPosY2
//	);
//	MYTRACE( "■nPosX2=%d,nPosY2=%d ", nPosX2, nPosY2 );
//	if( nPosX2 != m_nCaretPosX || nPosY2 != m_nCaretPosY ){
//		MYTRACE( "●●●エラー" );
//	}
//	MYTRACE( "\n" );

	return nScrollRowNum;


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
	RECT			rc;
	POINT			po;
	COMPOSITIONFORM	CompForm;
	HIMC			hIMC = ::ImmGetContext( m_hWnd );
	POINT			point;
	HWND			hwndFrame;
	hwndFrame = ::GetParent( m_hwndParent );

	::GetCaretPos( &point );
	CompForm.dwStyle = CFS_POINT;
	CompForm.ptCurrentPos.x = (long) point.x + 1;
	CompForm.ptCurrentPos.y = (long) point.y + 1 + m_nCaretHeight - m_nCharHeight;

	::GetWindowRect( m_hWnd, &rc );
	po.x = 0;
	po.y = 0;
	::ClientToScreen( hwndFrame, &po );
	CompForm.ptCurrentPos.x += ( rc.left - po.x );
	CompForm.ptCurrentPos.y += ( rc.top  - po.y );

	if ( hIMC ){
		::ImmSetCompositionWindow( hIMC, &CompForm );
	}
	::ImmReleaseContext( m_hWnd , hIMC );
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
	HIMC	hIMC = ::ImmGetContext( m_hWnd );
	if ( hIMC ){
		::ImmSetCompositionFont( hIMC, &(m_pShareData->m_Common.m_lf) );
	}
	::ImmReleaseContext( m_hWnd , hIMC );
	return;
}





/* マウス等による座標指定によるカーソル移動
|| 必要に応じて縦/横スクロールもする
|| 垂直スクロールをした場合はその行数を返す(正／負)
*/
int CEditView::MoveCursorToPoint( int xPos, int yPos )
{
	const char*		pLine;
	int				nLineLen;
	int				i;
	int				nCharChars;
	int				nNewX;
	int				nNewY;
	int				nPosX;
	int				nScrollRowNum = 0;
	const CLayout*	pcLayout;
	nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
	if( 0 > nNewY ){
		nNewY = 0;
	}
	/* カーソルがテキスト最下端行にあるか */
	if( nNewY >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
		nNewY = m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1;
		if( 0 > nNewY ){
			nNewY = 0;
		}
		nLineLen = 0;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( nNewY, &nLineLen, &pcLayout );
		if( NULL == pLine ){
			nNewX = nLineLen;
		}else
		/* 改行で終わっているか */
		if( EOL_NONE != pcLayout->m_cEol.GetLen() ){
//		if( pLine[ nLineLen - 1 ] == '\n' || pLine[ nLineLen - 1 ] == '\r' ){
			nNewX = 0;
			++nNewY;
		}else{
			nNewX = LineIndexToColmn( pLine, nLineLen, nLineLen );
		}
		nScrollRowNum = MoveCursor( nNewX, nNewY, TRUE, 1000 );
		m_nCaretPosX_Prev = m_nCaretPosX;
	}else
	/* カーソルがテキスト最上端行にあるか */
	if( nNewY < 0 ){
		nNewX = 0;
		nNewY = 0;
		nScrollRowNum = MoveCursor( nNewX, nNewY, TRUE, 1000 );
		m_nCaretPosX_Prev = m_nCaretPosX;
	}else{
		/* 移動先の行のデータを取得 */
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( nNewY, &nLineLen, &pcLayout );
		if( NULL == pLine ){
			nLineLen = 0;
		}
		nPosX = 0;
		nCharChars = 0;
		for( i = 0; i < nLineLen; ){
			nPosX += nCharChars;
//			if( i == nLineLen - 1 && ( pLine[i] == '\n' || pLine[i] == '\r' ) ){
//			if( i >= nLineLen - pcLayout->m_cEol.GetLen() ){
			if( i >= nLineLen - (pcLayout->m_cEol.GetLen()?1:0 ) ){
				i = nLineLen;
				break;
			}
			if( pLine[i] == TAB ){
				nCharChars = m_pcEditDoc->GetDocumentAttribute().m_nTabSpace - ( nPosX % m_pcEditDoc->GetDocumentAttribute().m_nTabSpace );
				if( nPosX + nCharChars > nNewX ){
					break;
				}
				++i;
			}else{
				nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
				if( 0 == nCharChars ){
					nCharChars = 1;
				}
				if( nPosX + nCharChars > nNewX ){
					break;
				}
				i+= nCharChars;
			}
		}
		if( i >= nLineLen ){
			/* フリーカーソルモードか */
			if( m_pShareData->m_Common.m_bIsFreeCursorMode
			  || ( m_bBeginSelect && m_bBeginBoxSelect )	/* マウス範囲選択中 && 矩形範囲選択中 */
			  || m_bDragMode /* OLE DropTarget */
			){
				if( nNewY + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() &&
					pLine[ nLineLen - 1 ] != '\n' && pLine[ nLineLen - 1 ] != '\r'
				){
					nPosX = LineIndexToColmn( pLine, nLineLen, nLineLen );
				}else{
					nPosX = nNewX;
					if( nPosX < 0 ){
						nPosX = 0;
					}else
					if( nPosX > m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize ){	/* 折り返し文字数 */
						nPosX = m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize;
					}
				}
			}
		}
		nScrollRowNum = MoveCursor( nPosX, nNewY, TRUE, 1000 );
		m_nCaretPosX_Prev = m_nCaretPosX;
	}
	return nScrollRowNum;
}
//_CARETMARGINRATE_CARETMARGINRATE_CARETMARGINRATE



/* マウス左ボタン押下 */
void CEditView::OnLBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{

	if( m_bHokan ){
		m_pcEditDoc->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

//	DWORD	nKeyBoardSpeed;
	int			nCaretPosY_Old;
	int			nUrlLine;	// URLの行(折り返し単位)
	int			nUrlIdxBgn;	// URLの位置(行頭からのバイト位置)
	int			nUrlLen;	// URLの長さ(バイト数)
	CMemory		cmemCurText;
	const char*	pLine;
	int			nLineLen;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	int			nIdx;
	int			nWork;

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() == 0 ){
		return;
	}
	if( m_nCaretWidth == 0 ){ //フォーカスがないとき
		return;
	}
	nCaretPosY_Old = m_nCaretPosY;

	/* 現在のマウスカーソル位置→レイアウト位置 */
	int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
//	MYTRACE( "OnLBUTTONDOWN() nNewX=%d nNewY=%d\n", nNewX, nNewY );

	if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop ){	/* OLEによるドラッグ & ドロップを使う */
		if( m_pShareData->m_Common.m_bUseOLE_DropSource ){		/* OLEによるドラッグ元にするか */
			/* 行選択エリアをドラッグした */
			if( xPos < m_nViewAlignLeft - ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) ){
				goto normal_action;
			}
			/* 指定カーソル位置が選択エリア内にあるか */
			if( 0 == IsCurrentPositionSelected(
				nNewX,		// カーソル位置X
				nNewY		// カーソル位置Y
				)
			){
				/* 選択範囲のデータを取得 */
				if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE ) ){
					DWORD dwEffects;
					m_bDragSource = TRUE;
					CDataObject data( cmemCurText.GetPtr( NULL ) );
					dwEffects = data.DragDrop( TRUE, DROPEFFECT_COPY | DROPEFFECT_MOVE );
					m_bDragSource = FALSE;
//					MYTRACE( "dwEffects=%d\n", dwEffects );
					if( 0 == dwEffects ){
						if( IsTextSelected() ){	/* テキストが選択されているか */
							/* 現在の選択範囲を非選択状態に戻す */
							DisableSelectArea( TRUE );
						}
					}
				}
				return;
			}
		}
	}

normal_action:;

	/* ALTキーが押されていたか */
	if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
		if( IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( TRUE );
		}
		if( yPos >= m_nViewAlignTop  && yPos < m_nViewAlignTop  + m_nViewCy ){
			if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
				MoveCursorToPoint( xPos, yPos );
			}else
			if( xPos < m_nViewAlignLeft ){
				MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
			}else{
				return;
			}
		}
		m_nMouseRollPosXOld = xPos;		/* マウス範囲選択前回位置(X座標) */
		m_nMouseRollPosYOld = yPos;		/* マウス範囲選択前回位置(Y座標) */
		/* 範囲選択開始 & マウスキャプチャー */
		m_bBeginSelect = TRUE;			/* 範囲選択中 */
		m_bBeginBoxSelect = TRUE;		/* 矩形範囲選択中でない */
		m_bBeginLineSelect = FALSE;		/* 行単位選択中 */
		m_bBeginWordSelect = FALSE;		/* 単語単位選択中 */

//		if( m_pShareData->m_Common.m_bFontIs_FIXED_PITCH ){	/* 現在のフォントは固定幅フォントである */
//			/* ALTキーが押されていたか */
//			if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
//				m_bBeginBoxSelect = TRUE;	/* 矩形範囲選択中 */
//			}
//		}
		::SetCapture( m_hWnd );
		::HideCaret( m_hWnd );
		/* 現在のカーソル位置から選択を開始する */
		BeginSelectArea( );
		if( xPos < m_nViewAlignLeft ){
			/* カーソル下移動 */
			Command_DOWN( TRUE, FALSE );
		}
	}else{
		/* カーソル移動 */
		if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
			if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
			}else
			if( xPos < m_nViewAlignLeft ){
			}else{
				return;
			}
		}else{
			return;
		}

		/* マウスのキャプチャなど */
		m_nMouseRollPosXOld = xPos;		/* マウス範囲選択前回位置(X座標) */
		m_nMouseRollPosYOld = yPos;		/* マウス範囲選択前回位置(Y座標) */
		/* 範囲選択開始 & マウスキャプチャー */
		m_bBeginSelect = TRUE;			/* 範囲選択中 */
//		m_bBeginBoxSelect = FALSE;		/* 矩形範囲選択中でない */
		m_bBeginLineSelect = FALSE;		/* 行単位選択中 */
		m_bBeginWordSelect = FALSE;		/* 単語単位選択中 */
		::SetCapture( m_hWnd );
		::HideCaret( m_hWnd );


		/* 選択開始処理 */
		/* SHIFTキーが押されていたか */
		if( (SHORT)0x8000 & ::GetKeyState( VK_SHIFT ) ){
			if( IsTextSelected() ){			/* テキストが選択されているか */
				if( m_bBeginBoxSelect ){	/* 矩形範囲選択中 */
					/* 現在の選択範囲を非選択状態に戻す */
					DisableSelectArea( TRUE );
					/* 現在のカーソル位置から選択を開始する */
					BeginSelectArea( );
				}else{
				}
			}else{
				/* 現在のカーソル位置から選択を開始する */
				BeginSelectArea( );
			}

			/* カーソル移動 */
			if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
				if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
					MoveCursorToPoint( xPos, yPos );
				}else
				if( xPos < m_nViewAlignLeft ){
					MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
				}
			}
		}else{
			if( IsTextSelected() ){	/* テキストが選択されているか */
				/* 現在の選択範囲を非選択状態に戻す */
				DisableSelectArea( TRUE );
			}
			/* カーソル移動 */
			if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
				if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
					MoveCursorToPoint( xPos, yPos );
				}else
				if( xPos < m_nViewAlignLeft ){
					MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
				}
			}
			/* 現在のカーソル位置から選択を開始する */
			BeginSelectArea( );
		}


		/******* この時点で必ず TRUE == IsTextSelected() の状態になる ****:*/
		if( !IsTextSelected() ){
			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
				"バグってる"
			);
			return;
		}

		int	nWorkRel;
		nWorkRel = IsCurrentPositionSelected(
			m_nCaretPosX,	// カーソル位置X
			m_nCaretPosY	// カーソル位置Y
		);
//		MYTRACE( "◆◆◆nWorkRel = %d\n", nWorkRel );


		/* 現在のカーソル位置によって選択範囲を変更 */
		ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );


		/* CTRLキーが押されていたか */
		if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
			m_bBeginWordSelect = TRUE;		/* 単語単位選択中 */
			if( !IsTextSelected() ){
				/* 現在位置の単語選択 */
				Command_SELECTWORD();
				m_nSelectLineBgnFrom = m_nSelectLineFrom;	/* 範囲選択開始行(原点) */
				m_nSelectColmBgnFrom = m_nSelectColmFrom;	/* 範囲選択開始桁(原点) */
				m_nSelectLineBgnTo = m_nSelectLineTo;		/* 範囲選択開始行(原点) */
				m_nSelectColmBgnTo = m_nSelectColmTo;		/* 範囲選択開始桁(原点) */
			}else{

				/* 選択領域描画 */
				DrawSelectArea();


				/* 指定された桁に対応する行のデータ内の位置を調べる */
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nSelectLineFrom, &nLineLen );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pLine, nLineLen, m_nSelectColmFrom );
					/* 現在位置の単語の範囲を調べる */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_nSelectLineFrom, nIdx, &nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
					){
						/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen );
						nColmFrom = LineIndexToColmn( pLine, nLineLen, nColmFrom );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen );
						nColmTo = LineIndexToColmn( pLine, nLineLen, nColmTo );


						nWork = IsCurrentPositionSelected(
							nColmFrom,	// カーソル位置X
							nLineFrom	// カーソル位置Y
						);
						if( -1 == nWork || 0 == nWork ){
							m_nSelectLineFrom = nLineFrom;
							m_nSelectColmFrom = nColmFrom;
							if( 1 == nWorkRel ){
								m_nSelectLineBgnFrom = nLineFrom;	/* 範囲選択開始行(原点) */
								m_nSelectColmBgnFrom = nColmFrom;	/* 範囲選択開始桁(原点) */
								m_nSelectLineBgnTo = nLineTo;		/* 範囲選択開始行(原点) */
								m_nSelectColmBgnTo = nColmTo;		/* 範囲選択開始桁(原点) */
							}
						}
	//					if( 1 == IsCurrentPositionSelected(
	//						nColmTo,	// カーソル位置X
	//						nLineTo		// カーソル位置Y
	//					) ){
	//						m_nSelectLineFrom = nLineTo;
	//						m_nSelectColmFrom = nColmTo;
	//					}
					}
				}
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nSelectLineTo, &nLineLen );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pLine, nLineLen, m_nSelectColmTo );
					/* 現在位置の単語の範囲を調べる */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_nSelectLineTo, nIdx,
						&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
					){
						/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen );
						nColmFrom = LineIndexToColmn( pLine, nLineLen, nColmFrom );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen );
						nColmTo = LineIndexToColmn( pLine, nLineLen, nColmTo );

						nWork = IsCurrentPositionSelected(
							nColmFrom,	// カーソル位置X
							nLineFrom	// カーソル位置Y
						);
						if( -1 == nWork || 0 == nWork ){
							m_nSelectLineTo = nLineFrom;
							m_nSelectColmTo = nColmFrom;
						}
						if( 1 == IsCurrentPositionSelected(
							nColmTo,	// カーソル位置X
							nLineTo		// カーソル位置Y
						) ){
							m_nSelectLineTo = nLineTo;
							m_nSelectColmTo = nColmTo;
						}
						if( -1 == nWorkRel || 0 == nWorkRel ){
							m_nSelectLineBgnFrom = nLineFrom;	/* 範囲選択開始行(原点) */
							m_nSelectColmBgnFrom = nColmFrom;	/* 範囲選択開始桁(原点) */
							m_nSelectLineBgnTo = nLineTo;		/* 範囲選択開始行(原点) */
							m_nSelectColmBgnTo = nColmTo;		/* 範囲選択開始桁(原点) */
						}
					}
				}

				if( 0 < nWorkRel ){

				}
				/* 選択領域描画 */
				DrawSelectArea();
			}
		}
		if( xPos < m_nViewAlignLeft ){
			/* 現在のカーソル位置から選択を開始する */
//			BeginSelectArea( );
			m_bBeginLineSelect = TRUE;

			/* カーソル下移動 */
			Command_DOWN( TRUE, FALSE );
			m_nSelectLineBgnTo = m_nSelectLineTo;	/* 範囲選択開始行(原点) */
			m_nSelectColmBgnTo = m_nSelectColmTo;	/* 範囲選択開始桁(原点) */
		}else{
//			/* 現在のカーソル位置から選択を開始する */
//			BeginSelectArea( );
//			m_bBeginLineSelect = FALSE;

			/* URLがクリックされたら選択するか */
			if( TRUE == m_pShareData->m_Common.m_bSelectClickedURL ){

				/* カーソル位置にURLが有る場合のその範囲を調べる */
				if( IsCurrentPositionURL(
					m_nCaretPosX,	// カーソル位置X
					m_nCaretPosY,	// カーソル位置Y
					&nUrlLine,		// URLの行(改行単位)
					&nUrlIdxBgn,	// URLの位置(行頭からのバイト位置)
					&nUrlLen,		// URLの長さ(バイト数)
					NULL			// URL受け取り先
				) ){

					/* 現在の選択範囲を非選択状態に戻す */
					DisableSelectArea( TRUE );

					/* 選択範囲の変更 */
//					m_nSelectLineBgn = nUrlLine;				/* 範囲選択開始行(原点) */
//					m_nSelectColmBgn = nUrlIdxBgn;				/* 範囲選択開始桁(原点) */
					m_nSelectLineBgnFrom = nUrlLine;			/* 範囲選択開始行(原点) */
					m_nSelectColmBgnFrom = nUrlIdxBgn;			/* 範囲選択開始桁(原点) */
					m_nSelectLineBgnTo = nUrlLine;				/* 範囲選択開始行(原点) */
					m_nSelectColmBgnTo = nUrlIdxBgn + nUrlLen;	/* 範囲選択開始桁(原点) */

					m_nSelectLineFrom =	nUrlLine;
					m_nSelectColmFrom = nUrlIdxBgn;
					m_nSelectLineTo = nUrlLine;
					m_nSelectColmTo = nUrlIdxBgn + nUrlLen;

					/*
					  カーソル位置変換
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					  →レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
					*/
					int	nX, nY;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( m_nSelectColmBgnFrom, m_nSelectLineBgnFrom, &nX, &nY );
					m_nSelectLineBgnFrom = nY;		/* 範囲選択開始行(原点) */
					m_nSelectColmBgnFrom = nX;		/* 範囲選択開始桁(原点) */
					m_nSelectLineFrom =	nY;
					m_nSelectColmFrom  = nX;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( m_nSelectColmTo, m_nSelectLineTo, &nX, &nY );
					m_nSelectLineTo = nY;
					m_nSelectColmTo = nX;
					m_nSelectLineBgnTo = nY;		/* 範囲選択開始行(原点) */
					m_nSelectColmBgnTo = nX;		/* 範囲選択開始桁(原点) */
					/* 選択領域描画 */
					DrawSelectArea();
				}
			}
		}
	}
//	/* キーボードの現在のリピート間隔を取得 */
//	SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0 );
//	nKeyBoardSpeed *= 3;
//	/* タイマー起動 */
//	::SetTimer( m_hWnd, IDT_ROLLMOUSE, nKeyBoardSpeed, (TIMERPROC)EditViewTimerProc );
	return;
}

/* 指定カーソル位置にURLが有る場合のその範囲を調べる */
/* 戻り値がTRUEの場合、*ppszURLは呼び出し側でdeleteすること */
BOOL CEditView::IsCurrentPositionURL(
		int		nCaretPosX,		// カーソル位置X
		int		nCaretPosY,		// カーソル位置Y
		int*	pnUrlLine,		// URLの行(改行単位)
		int*	pnUrlIdxBgn,	// URLの位置(行頭からのバイト位置)
		int*	pnUrlLen,		// URLの長さ(バイト数)
		char**	ppszURL			// URL受け取り先(関数内でnewする)
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CEditView::IsCurrentPositionURL" );
#endif
	const char*	pLine;
//	const char*	pLineWork;
	int			nLineLen;
	int			nX;
	int			nY;
//	char*		pURL;
	int			i;
//	BOOL		bFindURL;
//	int			nCharChars;
	int			nUrlLen;

	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		nCaretPosX,
		nCaretPosY,
		(int*)&nX,
		(int*)&nY
	);
	*pnUrlLine = nY;
	pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( nY, &nLineLen );

	i = nX - 200;
	if( i < 0 ){
		i = 0;
	}
	for( ; i <= nX && i < nLineLen && i < nX + 200; ){
	/* カーソル位置から前方に250バイトまでの範囲内で行頭に向かってサーチ */
		/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
		if( FALSE == IsURL( &pLine[i], nLineLen - i, &nUrlLen ) ){
			++i;
		}else{
			if( i <= nX && nX < i + nUrlLen ){
				/* URLを返す場合 */
				if( NULL != ppszURL ){
					*ppszURL = new char[nUrlLen + 1];
					memcpy( *ppszURL, &pLine[i], nUrlLen );
					(*ppszURL)[nUrlLen] = '\0';
					/* *ppszURLは呼び出し側でdeleteすること */
				}
				*pnUrlLen = nUrlLen;
				*pnUrlLine = nY;
				*pnUrlIdxBgn = i;
				return TRUE;
			}else{
				i += nUrlLen;
			}
		}
	}
	return FALSE;
}




/* マウス右ボタン押下 */
void CEditView::OnRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	/* 現在のマウスカーソル位置→レイアウト位置 */
	int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
	/* 指定カーソル位置が選択エリア内にあるか */
	if( 0 == IsCurrentPositionSelected(
		nNewX,		// カーソル位置X
		nNewY		// カーソル位置Y
		)
	){
		return;
	}
	OnLBUTTONDOWN( fwKeys, xPos , yPos );
	return;

	int			nIdx;
	int			nFuncID;
	nIdx = 0;
	/* Ctrl,ALT,キーが押されていたか */
	if( (SHORT)0x8000 & ::GetKeyState( VK_SHIFT ) ){
		nIdx |= _SHIFT;
	}
	if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
		nIdx |= _CTRL;
	}
	if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
		nIdx |= _ALT;
	}
	/* マウス右クリックに対応する機能コードはm_Common.m_pKeyNameArr[1]に入っている */
	nFuncID = m_pShareData->m_pKeyNameArr[1].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, 0 ),  (LPARAM)NULL );
	}
//	/* 右クリックメニュー */
//	Command_MENU_RBUTTON();
	return;
}

/* マウス右ボタン開放 */
void CEditView::OnRBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	if( m_bBeginSelect ){	/* 範囲選択中 */
		/* マウス左ボタン開放のメッセージ処理 */
		OnLBUTTONUP( fwKeys, xPos, yPos );
	}


	int		nIdx;
	int		nFuncID;
	nIdx = 0;
	/* Ctrl,ALT,キーが押されていたか */
	if( (SHORT)0x8000 & ::GetKeyState( VK_SHIFT ) ){
		nIdx |= _SHIFT;
	}
	if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
		nIdx |= _CTRL;
	}
	if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
		nIdx |= _ALT;
	}
	/* マウス右クリックに対応する機能コードはm_Common.m_pKeyNameArr[1]に入っている */
	nFuncID = m_pShareData->m_pKeyNameArr[1].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, 0 ),  (LPARAM)NULL );
	}
//	/* 右クリックメニュー */
//	Command_MENU_RBUTTON();
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
	CMemory		cmemCurText;
	CMemory*	pcmemRefText;
	char*		pszWork;
	int			nWorkLength;
	int			i;

	if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop ){	/* OLEによるドラッグ & ドロップを使う */
		if( m_bDragSource ){
			return;
		}
	}
	if( !m_bBeginSelect ){	/* 範囲選択中 */
		//	2001/06/14 asa-o 参照するデータの変更
//		if( m_pShareData->m_Common.m_bUseKeyWordHelp ){ /* キーワードヘルプを使用する */
		if( m_pcEditDoc->GetDocumentAttribute().m_bUseKeyWordHelp ){ /* キーワードヘルプを使用する */
			if( m_nCaretWidth > 0 ){ //フォーカスがあるとき
				/* ウィンドウ内にマウスカーソルがあるか？ */
				GetCursorPos( &po );
				GetWindowRect( m_hWnd, &rc );
				if( !PtInRect( &rc, po ) ){
					return;
				}
				/*  */
				if( m_bInMenuLoop == FALSE	&&	/* メニュー モーダル ループに入っていない */
					0 != m_dwTipTimer		&&	/* 辞書Tipを表示していない */
					300 < ::GetTickCount() - m_dwTipTimer	/* 一定時間以上、マウスが固定されている */
				){
					/* 選択範囲のデータを取得(複数行選択の場合は先頭の行のみ) */
					if( GetSelectedData( cmemCurText, TRUE, NULL, FALSE ) ){
						pszWork = cmemCurText.GetPtr( NULL );
						nWorkLength	= lstrlen( pszWork );
						for( i = 0; i < nWorkLength; ++i ){
							if( pszWork[i] == '\0' ||
								pszWork[i] == CR ||
								pszWork[i] == LF ){
								break;
							}
						}
						char*	pszBuf = new char[i + 1];
						memcpy( pszBuf, pszWork, i );
						pszBuf[i] = '\0';
						cmemCurText.SetData( pszBuf, i );
						delete [] pszBuf;

						/* 既に検索済みか */
						if( CMemory::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) ){
							/* 該当するキーがなかった */
							if( !m_cTipWnd.m_KeyWasHit ){
								goto end_of_search;
							}
						}else{
							m_cTipWnd.m_cKey = cmemCurText;
							/* 検索実行 */
							//	2001/06/14 asa-o 参照するデータの変更
//							if( m_cDicMgr.Search( cmemCurText.GetPtr( NULL ), &pcmemRefText, m_pShareData->m_Common.m_szKeyWordHelpFile ) ){
							if( m_cDicMgr.Search( cmemCurText.GetPtr( NULL ), &pcmemRefText, m_pcEditDoc->GetDocumentAttribute().m_szKeyWordHelpFile ) ){
								/* 該当するキーがある */
								m_cTipWnd.m_KeyWasHit = TRUE;
								pszWork = pcmemRefText->GetPtr( NULL );
//								m_cTipWnd.m_cInfo.SetData( pszWork, lstrlen( pszWork ) );
								m_cTipWnd.m_cInfo.SetDataSz( pszWork );
								delete pcmemRefText;
							}else{
								/* 該当するキーがなかった */
								m_cTipWnd.m_KeyWasHit = FALSE;
								goto end_of_search;
							}
						}
						m_dwTipTimer = 0;	/* 辞書Tipを表示している */
						m_poTipCurPos = po;	/* 現在のマウスカーソル位置 */

						/* 辞書Tipを表示 */
						m_cTipWnd.Show( po.x, po.y + m_nCharHeight, NULL );
					}
					end_of_search:;
				}
			}
		}
	}else{
		::GetCursorPos( &po );
		::GetWindowRect(m_hWnd, &rc );
		if( !PtInRect( &rc, po ) ){
			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
			return;
		}

		// 1999.12.18 クライアント領域内ではタイマー自動ドラッグ+ロールしない
		return;

//		rc.top += m_nViewAlignTop;
		RECT rc2;
		rc2 = rc;
		rc2.bottom = rc.top + m_nViewAlignTop + ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
		if( PtInRect( &rc2, po )
		 && 0 < m_nViewTopLine
		){
			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
			return;
		}
		rc2 = rc;
		rc2.top = rc.bottom - ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
		if( PtInRect( &rc2, po )
			&& m_pcEditDoc->m_cLayoutMgr.GetLineCount() > m_nViewTopLine + m_nViewRowNum
		){
			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
			return;
		}

//		rc.top += 48;
//		rc.bottom -= 48;
//		if( !PtInRect( &rc, po ) ){
//			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
//		}
	}
	return;
}





/* マウス移動のメッセージ処理 */
void CEditView::OnMOUSEMOVE( WPARAM fwKeys, int xPos , int yPos )
{
//#ifdef _DEBUG
//	gm_ProfileOutput = 1;
//	CRunningTimer cRunningTimer( (const char*)"CEditView::OnMOUSEMOVE" );
//#endif
	int			nScrollRowNum;
	POINT		po;
	const char*	pLine;
	int			nLineLen;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	int			nIdx;
	int			nWorkF;
	int			nWorkT;

	int			nSelectLineBgnFrom_Old;		/* 範囲選択開始行(原点) */
	int			nSelectColmBgnFrom_Old;		/* 範囲選択開始桁(原点) */
	int			nSelectLineBgnTo_Old;		/* 範囲選択開始行(原点) */
	int			nSelectColmBgnTo_Old;		/* 範囲選択開始桁(原点) */
	int			nSelectLineFrom_Old;
	int			nSelectColmFrom_Old;
	int			nSelectLineTo_Old;
	int			nSelectColmTo_Old;
	int			nSelectLineFrom;
	int			nSelectColmFrom;
	int			nSelectLineTo;
	int			nSelectColmTo;

	nSelectLineBgnFrom_Old	= m_nSelectLineBgnFrom;		/* 範囲選択開始行(原点) */
	nSelectColmBgnFrom_Old	= m_nSelectColmBgnFrom;		/* 範囲選択開始桁(原点) */
	nSelectLineBgnTo_Old	= m_nSelectLineBgnTo;		/* 範囲選択開始行(原点) */
	nSelectColmBgnTo_Old	= m_nSelectColmBgnTo;		/* 範囲選択開始桁(原点) */
	nSelectLineFrom_Old		= m_nSelectLineFrom;
	nSelectColmFrom_Old		= m_nSelectColmFrom;
	nSelectLineTo_Old		= m_nSelectLineTo;
	nSelectColmTo_Old		= m_nSelectColmTo;

	if( !m_bBeginSelect ){	/* 範囲選択中 */
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
		int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
		int			nUrlLine;	// URLの行(折り返し単位)
		int			nUrlIdxBgn;	// URLの位置(行頭からのバイト位置)
		int			nUrlLen;	// URLの長さ(バイト数)


		/* 選択テキストのドラッグ中か */
		if( m_bDragMode ){
			if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop ){	/* OLEによるドラッグ & ドロップを使う */
				/* 座標指定によるカーソル移動 */
				nScrollRowNum = MoveCursorToPoint( xPos , yPos );
			}
		}else{
			/* 行選択エリア? */
			if( xPos < m_nViewAlignLeft ){
				/* 矢印カーソル */
				::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}else

			if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop	/* OLEによるドラッグ & ドロップを使う */
			 && TRUE == m_pShareData->m_Common.m_bUseOLE_DropSource /* OLEによるドラッグ元にするか */
			 && 0 == IsCurrentPositionSelected(						/* 指定カーソル位置が選択エリア内にあるか */
				nNewX,	// カーソル位置X
				nNewY	// カーソル位置Y
				)
			){
				/* 矢印カーソル */
				::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}else
			/* カーソル位置にURLが有る場合 */
			if( IsCurrentPositionURL(
				nNewX,			// カーソル位置X
				nNewY,			// カーソル位置Y
				&nUrlLine,		// URLの行(改行単位)
				&nUrlIdxBgn,	// URLの位置(行頭からのバイト位置)
				&nUrlLen,		// URLの長さ(バイト数)
				NULL/*&pszURL*/	// URL受け取り先
			) ){
				/* 手カーソル */
				::SetCursor( ::LoadCursor( m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_HAND ) ) );
			}else{
				/* アイビーム */
				::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
			}
		}
		return;
	}
	::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
	if( m_bBeginBoxSelect ){	/* 矩形範囲選択中 */
		/* 座標指定によるカーソル移動 */
		nScrollRowNum = MoveCursorToPoint( xPos , yPos );
		/* 現在のカーソル位置によって選択範囲を変更 */
		ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
		m_nMouseRollPosXOld = xPos;	/* マウス範囲選択前回位置(X座標) */
		m_nMouseRollPosYOld = yPos;	/* マウス範囲選択前回位置(Y座標) */
	}else{
		/* 座標指定によるカーソル移動 */
		if( xPos < m_nViewAlignLeft && m_bBeginLineSelect ){
			nScrollRowNum = MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) , yPos + ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) );
		}else{
			nScrollRowNum = MoveCursorToPoint( xPos , yPos );
		}
		m_nMouseRollPosXOld = xPos;	/* マウス範囲選択前回位置(X座標) */
		m_nMouseRollPosYOld = yPos;	/* マウス範囲選択前回位置(Y座標) */

		/* CTRLキーが押されていたか */
//		if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
		if( !m_bBeginWordSelect ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
		}else{
//			/* 現在のカーソル位置によって選択範囲を変更 */
//			ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
			/* 現在のカーソル位置によって選択範囲を変更(テストのみ) */
			ChangeSelectAreaByCurrentCursorTEST(
				(int)m_nCaretPosX,
				(int)m_nCaretPosY,
				(int&)nSelectLineFrom,
				(int&)nSelectColmFrom,
				(int&)nSelectLineTo,
				(int&)nSelectColmTo
			);
			/* 選択範囲に変更なし */
			if( nSelectLineFrom_Old == nSelectLineFrom
			 && nSelectColmFrom_Old == nSelectColmFrom
			 && nSelectLineTo_Old == nSelectLineTo
			 && nSelectColmTo_Old == nSelectColmTo
			){
				ChangeSelectAreaByCurrentCursor(
					(int)m_nCaretPosX,
					(int)m_nCaretPosY
				);
				return;
			}
			if( NULL != ( pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen ) ) ){
				nIdx = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );
				/* 現在位置の単語の範囲を調べる */
				if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
					m_nCaretPosY, nIdx,
					&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
				){
					/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen );
					nColmFrom = LineIndexToColmn( pLine, nLineLen, nColmFrom );
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen );
					nColmTo = LineIndexToColmn( pLine, nLineLen, nColmTo );

					nWorkF = IsCurrentPositionSelectedTEST(
						nColmFrom,	// カーソル位置X
						nLineFrom,	// カーソル位置Y
						(int)nSelectLineFrom,
						(int)nSelectColmFrom,
						(int)nSelectLineTo,
						(int)nSelectColmTo
					);
					nWorkT = IsCurrentPositionSelectedTEST(
						nColmTo,	// カーソル位置X
						nLineTo,	// カーソル位置Y
						(int)nSelectLineFrom,
						(int)nSelectColmFrom,
						(int)nSelectLineTo,
						(int)nSelectColmTo
					);
//					MYTRACE( "nWorkF=%d nWorkT=%d\n", nWorkF, nWorkT );
					if( -1 == nWorkF/* || 0 == nWorkF*/ ){
						/* 現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( nColmFrom, nLineFrom );
					}else
					if( /*0 == nWorkT ||*/ 1 == nWorkT ){
						/* 現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( nColmTo, nLineTo );
					}else
					if( nSelectLineFrom_Old == nSelectLineFrom
					 && nSelectColmFrom_Old == nSelectColmFrom
					){
						/* 始点が無変更→前方に縮小された */
						/* 現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( nColmTo, nLineTo );
					}else
					if( nSelectLineTo_Old == nSelectLineTo
					 && nSelectColmTo_Old == nSelectColmTo
					){
						/* 終点が無変更→後方に縮小された */
						/* 現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( nColmFrom, nLineFrom );
					}
				}else{
					/* 現在のカーソル位置によって選択範囲を変更 */
					ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
				}
			}else{
				/* 現在のカーソル位置によって選択範囲を変更 */
				ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
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
//	MYTRACE( "CEditView::DispatchEvent() WM_MOUSEWHEEL fwKeys=%xh zDelta=%d xPos=%d yPos=%d \n", fwKeys, zDelta, xPos, yPos );
	if( 0 < zDelta ){
		nScrollCode = SB_LINEUP;
	}else{
		nScrollCode = SB_LINEDOWN;
	}




	/* マウスホイールによるスクロール行数をレジストリから取得 */
	nRollLineNum = 6;
	/* レジストリの存在チェック */
	HKEY hkReg;
	DWORD dwType;
	DWORD dwDataLen;	// size of value data
	char szValStr[1024];
	if( ERROR_SUCCESS == ::RegOpenKeyEx( HKEY_CURRENT_USER, "Control Panel\\desktop", 0, KEY_READ, &hkReg ) ){
		dwType = REG_SZ;
		dwDataLen = sizeof( szValStr ) - 1;
		if( ERROR_SUCCESS == ::RegQueryValueEx( hkReg, "WheelScrollLines", NULL, &dwType, (unsigned char *)szValStr, &dwDataLen ) ){
//			MYTRACE( "szValStr=[%s]\n", szValStr );
			nRollLineNum = ::atoi( szValStr );
		}
		::RegCloseKey( hkReg );
	}
	if( -1 == nRollLineNum ){/* 「1画面分スクロールする」 */
		nRollLineNum = m_nViewRowNum;	// 表示域の行数
	}else{
		if( nRollLineNum < 1 ){
			nRollLineNum = 1;
		}
		if( nRollLineNum > 10 ){
			nRollLineNum = 10;
		}
	}
	for( i = 0; i < nRollLineNum; ++i ){
//		::PostMessage( m_hWnd, WM_VSCROLL, MAKELONG( nScrollCode, 0 ), (WPARAM)m_hwndVScrollBar );
//		::SendMessage( m_hWnd, WM_VSCROLL, MAKELONG( nScrollCode, 0 ), (WPARAM)m_hwndVScrollBar );
		if( nScrollCode == SB_LINEUP ){
			ScrollAtV( m_nViewTopLine - 1 );
		}else{
			ScrollAtV( m_nViewTopLine + 1 );
		}
	}
	return 0;
}





/* 現在のカーソル位置から選択を開始する */
void CEditView::BeginSelectArea( void )
{
//	m_nSelectLineBgn = m_nCaretPosY;	/* 範囲選択開始行(原点) */
//	m_nSelectColmBgn = m_nCaretPosX;	/* 範囲選択開始桁(原点) */

	m_nSelectLineBgnFrom = m_nCaretPosY;/* 範囲選択開始行(原点) */
	m_nSelectColmBgnFrom = m_nCaretPosX;/* 範囲選択開始桁(原点) */
	m_nSelectLineBgnTo = m_nCaretPosY;	/* 範囲選択開始行(原点) */
	m_nSelectColmBgnTo = m_nCaretPosX;	/* 範囲選択開始桁(原点) */

	m_nSelectLineFrom = m_nCaretPosY;	/* 範囲選択開始行 */
	m_nSelectColmFrom = m_nCaretPosX;	/* 範囲選択開始桁 */
	m_nSelectLineTo = m_nCaretPosY;		/* 範囲選択終了行 */
	m_nSelectColmTo = m_nCaretPosX;		/* 範囲選択終了桁 */
	return;
}





/* 現在の選択範囲を非選択状態に戻す */
void CEditView::DisableSelectArea( BOOL bDraw )
{
	m_nSelectLineFromOld = m_nSelectLineFrom;	/* 範囲選択開始行 */
	m_nSelectColmFromOld = m_nSelectColmFrom;	/* 範囲選択開始桁 */
	m_nSelectLineToOld = m_nSelectLineTo;		/* 範囲選択終了行 */
	m_nSelectColmToOld = m_nSelectColmTo;		/* 範囲選択終了桁 */
//	m_nSelectLineFrom = 0;
//	m_nSelectColmFrom = 0;
//	m_nSelectLineTo = 0;
//	m_nSelectColmTo = 0;

	m_nSelectLineFrom	= -1;
	m_nSelectColmFrom	= -1;
	m_nSelectLineTo		= -1;
	m_nSelectColmTo		= -1;

	if( bDraw ){
		DrawSelectArea();
	}
	m_bSelectingLock     = FALSE;	/* 選択状態のロック */
	m_nSelectLineFromOld = 0;		/* 範囲選択開始行 */
	m_nSelectColmFromOld = 0; 		/* 範囲選択開始桁 */
	m_nSelectLineToOld = 0;			/* 範囲選択終了行 */
	m_nSelectColmToOld = 0;			/* 範囲選択終了桁 */
	m_bBeginBoxSelect = FALSE;		/* 矩形範囲選択中 */
	m_bBeginLineSelect = FALSE;		/* 行単位選択中 */
	m_bBeginWordSelect = FALSE;		/* 単語単位選択中 */

	//	From Here Dec. 6, 2000 genta
#if 0
	//	フリーカーソルでない場合には行末より右にある
	//	キャレットを適切な位置まで移動する．
	if( ! m_pShareData->m_Common.m_bIsFreeCursorMode ){
		int len, pos;
		const char *line;


		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &len );
		//	データがない＝EOFのみの行 i.e. Columnは常に0
		if( line == NULL && m_nCaretPosX > 0){
			MoveCursor( 0, m_nCaretPosY, bDraw );
		}
		else {
			pos = LineIndexToColmn( line, m_nCaretPosY, len );	//	行末の桁位置を計算
			if( m_nCaretPosX > pos ){
				MoveCursor( pos, m_nCaretPosY, bDraw );
			}
		}
//		char buf[30];
//		wsprintf( buf, "X[%d] Y[%d], len[%d], pos[%d]", m_nCaretPosX, m_nCaretPosY, len, pos );
//		::MessageBox( NULL, buf, "CEditView::DisableSelectArea", MB_OK );
	}
#endif
	//	To Here Dec. 6, 2000 genta

//	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp ){
//		/* カーソル行アンダーラインの描画 */
//		HDC		hdc;
//		HPEN	hPen;
//		HPEN	hPenOld;
//		hdc = ::GetDC( m_hWnd );
//		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
//		hPenOld = (HPEN)::SelectObject( hdc, hPen );
//		m_nOldUnderLineY = m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight;
//		::MoveToEx(
//			hdc,
//			m_nViewAlignLeft,
//			m_nOldUnderLineY,
//			NULL
//		);
//		::LineTo(
//			hdc,
//			m_nViewCx + m_nViewAlignLeft,
//			m_nOldUnderLineY
//		);
//		::SelectObject( hdc, hPenOld );
//		::DeleteObject( hPen );
//		::ReleaseDC( m_hWnd, hdc );
//	}


	/* カーソル行アンダーラインのON */
	CaretUnderLineON( bDraw );
	return;
}





/* 現在のカーソル位置によって選択範囲を変更 */
void CEditView::ChangeSelectAreaByCurrentCursor( int nCaretPosX, int nCaretPosY )
{
//	MYTRACE( "ChangeSelectAreaByCurrentCursor( %d, %d )\n", nCaretPosX, nCaretPosY );
//	int			nLineFrom;
//	int			nColmFrom;
//	int			nLineTo;
//	int			nColmTo;
//	const char*	pLine;
//	int			nLineLen;
//	int			nIdx;

	m_nSelectLineFromOld = m_nSelectLineFrom;	/* 範囲選択開始行 */
	m_nSelectColmFromOld = m_nSelectColmFrom; 	/* 範囲選択開始桁 */
	m_nSelectLineToOld = m_nSelectLineTo;		/* 範囲選択終了行 */
	m_nSelectColmToOld = m_nSelectColmTo;		/* 範囲選択終了桁 */

	if( m_nSelectLineBgnFrom == m_nSelectLineBgnTo /* 範囲選択開始行(原点) */
	 && m_nSelectColmBgnFrom == m_nSelectColmBgnTo ){
		if( nCaretPosY == m_nSelectLineBgnFrom
		 && nCaretPosX == m_nSelectColmBgnFrom ){
			/* 選択解除 */
			m_nSelectLineFrom = -1;
			m_nSelectColmFrom  = -1;
			m_nSelectLineTo = -1;
			m_nSelectColmTo = -1;
		}else
		if( nCaretPosY < m_nSelectLineBgnFrom
		 || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX < m_nSelectColmBgnFrom ) ){
			m_nSelectLineFrom = nCaretPosY;
			m_nSelectColmFrom = nCaretPosX;
			m_nSelectLineTo = m_nSelectLineBgnFrom;
			m_nSelectColmTo = m_nSelectColmBgnFrom;
		}else{
			m_nSelectLineFrom = m_nSelectLineBgnFrom;
			m_nSelectColmFrom = m_nSelectColmBgnFrom;
			m_nSelectLineTo = nCaretPosY;
			m_nSelectColmTo = nCaretPosX;
		}
	}else{
		/* 常時選択範囲の範囲内 */
		if( ( nCaretPosY > m_nSelectLineBgnFrom || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX >= m_nSelectColmBgnFrom ) )
		 && ( nCaretPosY < m_nSelectLineBgnTo || ( nCaretPosY == m_nSelectLineBgnTo && nCaretPosX < m_nSelectColmBgnTo ) )
		){
			m_nSelectLineFrom = m_nSelectLineBgnFrom;
			m_nSelectColmFrom = m_nSelectColmBgnFrom;
			m_nSelectLineTo = m_nSelectLineBgnTo;
			m_nSelectColmTo = m_nSelectColmBgnTo;
		}else
		if( !( nCaretPosY > m_nSelectLineBgnFrom || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX >= m_nSelectColmBgnFrom ) ) ){
			/* 常時選択範囲の前方向 */
			m_nSelectLineFrom = nCaretPosY;
			m_nSelectColmFrom  = nCaretPosX;
			m_nSelectLineTo = m_nSelectLineBgnTo;
			m_nSelectColmTo = m_nSelectColmBgnTo;
		}else{
			/* 常時選択範囲の後ろ方向 */
			m_nSelectLineFrom = m_nSelectLineBgnFrom;
			m_nSelectColmFrom = m_nSelectColmBgnFrom;
			m_nSelectLineTo = nCaretPosY;
			m_nSelectColmTo = nCaretPosX;
		}
	}
	/* 選択領域の描画 */
	DrawSelectArea();
	return;
}

/* 現在のカーソル位置によって選択範囲を変更(テストのみ) */
void CEditView::ChangeSelectAreaByCurrentCursorTEST(
	int		nCaretPosX,
	int		nCaretPosY,
	int&	nSelectLineFrom,
	int&	nSelectColmFrom,
	int&	nSelectLineTo,
	int&	nSelectColmTo
)
{
//	MYTRACE( "ChangeSelectAreaByCurrentCursor( %d, %d )\n", nCaretPosX, nCaretPosY );
//	int			nLineFrom;
//	int			nColmFrom;
//	int			nLineTo;
//	int			nColmTo;
//	const char*	pLine;
//	int			nLineLen;
//	int			nIdx;


	if( m_nSelectLineBgnFrom == m_nSelectLineBgnTo /* 範囲選択開始行(原点) */
	 && m_nSelectColmBgnFrom == m_nSelectColmBgnTo ){
		if( nCaretPosY == m_nSelectLineBgnFrom
		 && nCaretPosX == m_nSelectColmBgnFrom ){
			/* 選択解除 */
			nSelectLineFrom = -1;
			nSelectColmFrom  = -1;
			nSelectLineTo = -1;
			nSelectColmTo = -1;
		}else
		if( nCaretPosY < m_nSelectLineBgnFrom
		 || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX < m_nSelectColmBgnFrom ) ){
			nSelectLineFrom = nCaretPosY;
			nSelectColmFrom = nCaretPosX;
			nSelectLineTo = m_nSelectLineBgnFrom;
			nSelectColmTo = m_nSelectColmBgnFrom;
		}else{
			nSelectLineFrom = m_nSelectLineBgnFrom;
			nSelectColmFrom = m_nSelectColmBgnFrom;
			nSelectLineTo = nCaretPosY;
			nSelectColmTo = nCaretPosX;
		}
	}else{
		/* 常時選択範囲の範囲内 */
		if( ( nCaretPosY > m_nSelectLineBgnFrom || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX >= m_nSelectColmBgnFrom ) )
		 && ( nCaretPosY < m_nSelectLineBgnTo || ( nCaretPosY == m_nSelectLineBgnTo && nCaretPosX < m_nSelectColmBgnTo ) )
		){
			nSelectLineFrom = m_nSelectLineBgnFrom;
			nSelectColmFrom = m_nSelectColmBgnFrom;
			nSelectLineTo = m_nSelectLineBgnTo;
			nSelectColmTo = m_nSelectColmBgnTo;
		}else
		if( !( nCaretPosY > m_nSelectLineBgnFrom || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX >= m_nSelectColmBgnFrom ) ) ){
			/* 常時選択範囲の前方向 */
			nSelectLineFrom = nCaretPosY;
			nSelectColmFrom  = nCaretPosX;
			nSelectLineTo = m_nSelectLineBgnTo;
			nSelectColmTo = m_nSelectColmBgnTo;
		}else{
			/* 常時選択範囲の後ろ方向 */
			nSelectLineFrom = m_nSelectLineBgnFrom;
			nSelectColmFrom = m_nSelectColmBgnFrom;
			nSelectLineTo = nCaretPosY;
			nSelectColmTo = nCaretPosX;
		}
	}
	return;

}


/* マウス左ボタン開放のメッセージ処理 */
void CEditView::OnLBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
//	MYTRACE( "OnLBUTTONUP()\n" );
	CMemory		cmemBuf, cmemClip;

	/* 範囲選択終了 & マウスキャプチャーおわり */
	if( m_bBeginSelect ){	/* 範囲選択中 */
		/* マウス キャプチャを解放 */
		::ReleaseCapture();
		::ShowCaret( m_hWnd );

//		/* タイマー終了 */
//		::KillTimer( m_hWnd, IDT_ROLLMOUSE );
		m_bBeginSelect = FALSE;

//		if( !IsTextSelected() ){	/* テキストが選択されているか */
			if( m_nSelectLineFrom == m_nSelectLineTo &&
				m_nSelectColmFrom == m_nSelectColmTo
			){
				/* 現在の選択範囲を非選択状態に戻す */
				DisableSelectArea( TRUE );
			}
//		}
	}
	return;
}





/* マウス左ボタンダブルクリック */
void CEditView::OnLBUTTONDBLCLK( WPARAM fwKeys, int xPos , int yPos )
{
//	MYTRACE( "OnLBUTTONDBLCLK()\n" );

	int			nIdx;
	int			nFuncID;
	int			nUrlLine;	// URLの行(折り返し単位)
	int			nUrlIdxBgn;	// URLの位置(行頭からのバイト位置)
	int			nUrlLen;	// URLの長さ(バイト数)
	char*		pszURL;
	const char*	pszMailTo = "mailto:";

	/* カーソル位置にURLが有る場合のその範囲を調べる */
	if( IsCurrentPositionURL(
		m_nCaretPosX,	// カーソル位置X
		m_nCaretPosY,	// カーソル位置Y
		&nUrlLine,		// URLの行(改行単位)
		&nUrlIdxBgn,	// URLの位置(行頭からのバイト位置)
		&nUrlLen,		// URLの長さ(バイト数)
		&pszURL			// URL受け取り先
	) ){
		char*		pszWork = NULL;
		char*		pszOPEN;

		/* URLを開く */
	 	/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す */
		if( TRUE == IsMailAddress( pszURL, lstrlen( pszURL ), NULL ) ){
			pszWork = new char[ lstrlen( pszURL ) + lstrlen( pszMailTo ) + 1];
			strcpy( pszWork, pszMailTo );
			strcat( pszWork, pszURL );
			pszOPEN = pszWork;
		}else{
			pszOPEN = pszURL;
		}
		::ShellExecute( NULL, "open", pszOPEN, NULL, NULL, SW_SHOW );
		delete [] pszURL;
		if( NULL != pszWork ){
			delete [] pszWork;
		}
		return;
	}

	/* GREP出力モード かつ マウス左ボタンダブルクリックでタグジャンプ の場合 */
	if( m_pcEditDoc->m_bGrepMode && m_pShareData->m_Common.m_bGTJW_LDBLCLK ){
		/* タグジャンプ機能 */
		Command_TAGJUMP();
		return;
	}


	nIdx = 0;
	/* Ctrl,ALT,キーが押されていたか */
	if( (SHORT)0x8000 & ::GetKeyState( VK_SHIFT ) ){
		nIdx |= _SHIFT;
	}
	if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
		nIdx |= _CTRL;
	}
	if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
		nIdx |= _ALT;
	}
	/* マウス左クリックに対応する機能コードはm_Common.m_pKeyNameArr[0]に入っている */
	nFuncID = m_pShareData->m_pKeyNameArr[0].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
//		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, 0 ),  (LPARAM)NULL );
		::SendMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, 0 ),  (LPARAM)NULL );
	}

	/* ドラッグ選択開始 */
	m_nMouseRollPosXOld = xPos;			/* マウス範囲選択前回位置(X座標) */
	m_nMouseRollPosYOld = yPos;			/* マウス範囲選択前回位置(Y座標) */
	/* 範囲選択開始 & マウスキャプチャー */
	m_bBeginSelect = TRUE;				/* 範囲選択中 */
	m_bBeginBoxSelect = FALSE;			/* 矩形範囲選択中でない */
	m_bBeginLineSelect = FALSE;			/* 行単位選択中 */
	m_bBeginWordSelect = TRUE;			/* 単語単位選択中 */

	if( m_pShareData->m_Common.m_bFontIs_FIXED_PITCH ){	/* 現在のフォントは固定幅フォントである */
		/* ALTキーが押されていたか */
		if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
			m_bBeginBoxSelect = TRUE;	/* 矩形範囲選択中 */
		}
	}
	::SetCapture( m_hWnd );
	::HideCaret( m_hWnd );
	if( IsTextSelected() ){
		/* 常時選択範囲の範囲 */
		m_nSelectLineBgnTo = m_nSelectLineTo;
		m_nSelectColmBgnTo = m_nSelectColmTo;
	}else{
		/* 現在のカーソル位置から選択を開始する */
		BeginSelectArea( );
	}

	return;
}





/* カーソル上下移動処理 */
int CEditView::Cursor_UPDOWN( int nMoveLines, int bSelect )
{
	const char*		pLine;
	int				nLineLen;
	int				nPosX;
	int				nPosY = m_nCaretPosY;
	int				i;
	int				nCharChars;
	int				nLineCols;
	int				nScrollLines;
	const CLayout*	pcLayout;
	nScrollLines = 0;
	if( nMoveLines > 0 ){
		/* カーソルがテキスト最下端行にあるか */
		if( m_nCaretPosY + nMoveLines >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
			nMoveLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount() - m_nCaretPosY  - 1;
		}
		if( nMoveLines <= 0 ){
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				nLineCols = LineIndexToColmn( pLine, nLineLen, nLineLen );
				/* 改行で終わっているか */
				if( ( EOL_NONE != pcLayout->m_cEol.GetLen() )
//				if( ( pLine[ nLineLen - 1 ] == '\n' || pLine[ nLineLen - 1 ] == '\r' )
				 || nLineCols >= m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize
				){
					if( bSelect ){
						if( !IsTextSelected() ){	/* テキストが選択されているか */
							/* 現在のカーソル位置から選択を開始する */
							BeginSelectArea();
						}
					}else{
						if( IsTextSelected() ){	/* テキストが選択されているか */
							/* 現在の選択範囲を非選択状態に戻す */
							DisableSelectArea( TRUE );
						}
					}
					nPosX = 0;
					++nPosY;
					if( bSelect ){
						/* 現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( nPosX, nPosY );
					}
					nScrollLines = MoveCursor( nPosX, nPosY, TRUE );
				}
			}
			return nScrollLines;
		}
	}else{
		/* カーソルがテキスト最上端行にあるか */
		if( m_nCaretPosY + nMoveLines < 0 ){
			nMoveLines = - m_nCaretPosY;
		}
		if( nMoveLines >= 0 ){
			return nScrollLines;
		}
	}
	if( bSelect ){
		if( !IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在のカーソル位置から選択を開始する */
			BeginSelectArea();
		}
	}else{
		if( IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( TRUE );
		}
	}
	/* 次の行のデータを取得 */
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY + nMoveLines, &nLineLen, &pcLayout );
	if( NULL == pLine ){
		nLineLen = 0;
	}
	nPosX = 0;
	for( i = 0; i < nLineLen; ){
//		if( i == nLineLen - 1 && ( pLine[i] == '\n' || pLine[i] == '\r' ) ){
//		if( i >= nLineLen - pcLayout->m_cEol.GetLen() ){
		if( i >= nLineLen - (pcLayout->m_cEol.GetLen()?1:0 ) ){
			i = nLineLen;
			break;
		}
		if( pLine[i] == TAB ){
			nCharChars = m_pcEditDoc->GetDocumentAttribute().m_nTabSpace - ( nPosX % m_pcEditDoc->GetDocumentAttribute().m_nTabSpace );
			if( nPosX + nCharChars > m_nCaretPosX_Prev ){
				break;
			}
			++i;
		}else{
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
			if( nPosX + nCharChars > m_nCaretPosX_Prev ){
				break;
			}
			i+= nCharChars;
		}
		nPosX += nCharChars;
	}
	if( i >= nLineLen ){
		/* フリーカーソルモードか */
		if( m_pShareData->m_Common.m_bIsFreeCursorMode
		 || IsTextSelected() && m_bBeginBoxSelect	/* 矩形範囲選択中 */
		){
			if( m_nCaretPosY + nMoveLines + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount()  ){
				if( NULL != pLine ){
					if( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF ){
						nPosX = m_nCaretPosX_Prev;
					}
				}
			}else{
				nPosX = m_nCaretPosX_Prev;
			}
		}
	}
	nScrollLines = MoveCursor( nPosX, m_nCaretPosY + nMoveLines, TRUE );
	if( bSelect ){
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && !IsTextSelected() && -1 != m_nOldUnderLineY ){
//			HDC		hdc;
//			HPEN	hPen, hPenOld;
//			hdc = ::GetDC( m_hWnd );
//			/* カーソル行アンダーラインの消去 */
//			hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
//			hPenOld = (HPEN)::SelectObject( hdc, hPen );
//			::MoveToEx(
//				hdc,
//				m_nViewAlignLeft,
//				m_nOldUnderLineY,
//				NULL
//			);
//			::LineTo(
//				hdc,
//				m_nViewCx + m_nViewAlignLeft,
//				m_nOldUnderLineY
//			);
//			::SelectObject( hdc, hPenOld );
//			::DeleteObject( hPen );
//			m_nOldUnderLineY = -1;
//			::ReleaseDC( m_hWnd, hdc );
//		}
		/* 現在のカーソル位置によって選択範囲を変更 */
//		ChangeSelectAreaByCurrentCursor( nPosX, m_nCaretPosY + nMoveLines );
		ChangeSelectAreaByCurrentCursor( nPosX, m_nCaretPosY );
	}
	return nScrollLines;
}





/* 指定上端行位置へスクロール */
void CEditView::ScrollAtV( int nPos )
{
	int			nScrollRowNum;
	RECT		rcScrol;
	RECT		rcClip;
//	RECT		rcClip2;
//	PAINTSTRUCT ps;
//	HDC			hdc;
	if( nPos < 0 ){
		nPos = 0;
	}else
	if( (m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 )- m_nViewRowNum < nPos ){
		nPos = ( m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 ) - m_nViewRowNum;
		if( nPos < 0 ){
			nPos = 0;
		}
	}
	if( m_nViewTopLine == nPos ){
		return;
	}
	/* 垂直スクロール量（行数）の算出 */
	nScrollRowNum = m_nViewTopLine - nPos;

	/* スクロール */
	if( abs( nScrollRowNum ) >= m_nViewRowNum ){
		m_nViewTopLine = nPos;
//		if( bDraw ){
			::InvalidateRect( m_hWnd, NULL, TRUE );
//			/* スクロールバーの状態を更新する */
//			AdjustScrollBars();
//		}
	}else{
//	}else
//	if( nScrollRowNum != 0 || nScrollColNum != 0 ){
		rcScrol.left = 0;
		rcScrol.right = m_nViewCx + m_nViewAlignLeft;
		rcScrol.top = m_nViewAlignTop;
		rcScrol.bottom = m_nViewCy + m_nViewAlignTop;
		if( nScrollRowNum > 0 ){
			rcScrol.bottom =
				m_nViewCy + m_nViewAlignTop -
				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			m_nViewTopLine = nPos;
			rcClip.left = 0;
			rcClip.right = m_nViewCx + m_nViewAlignLeft;
			rcClip.top = m_nViewAlignTop;
			rcClip.bottom =
				m_nViewAlignTop + nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
		}else
		if( nScrollRowNum < 0 ){
			rcScrol.top =
				m_nViewAlignTop - nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			m_nViewTopLine = nPos;
			rcClip.left = 0;
			rcClip.right = m_nViewCx + m_nViewAlignLeft;
			rcClip.top =
				m_nViewCy + m_nViewAlignTop +
				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			rcClip.bottom = m_nViewCy + m_nViewAlignTop;
		}
//		if( nScrollColNum > 0 ){
//			rcScrol.left = m_nViewAlignLeft;
//			rcScrol.right =
//				m_nViewCx + m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
//			rcClip2.left = m_nViewAlignLeft;
//			rcClip2.right = m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
//			rcClip2.top = m_nViewAlignTop;
//			rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
//		}else
//		if( nScrollColNum < 0 ){
//			rcScrol.left = m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
//			rcClip2.left =
//				m_nViewCx + m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
//			rcClip2.right = m_nViewCx + m_nViewAlignLeft;
//			rcClip2.top = m_nViewAlignTop;
//			rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
//		}
		if( m_bDrawSWITCH ){
//			::ScrollWindow(
//				m_hWnd,	/* スクロールするウィンドウのハンドル */
//				0/*nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )*/,	/* 水平スクロール量 */
//				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),		/* 垂直スクロール量 */
//				&rcScrol,	/* スクロール長方形の構造体のアドレス */
//				NULL		/* クリッピング長方形の構造体のアドレス */
//			);
			::ScrollWindowEx(
				m_hWnd,
				0,	/* 水平スクロール量 */
				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),		/* 垂直スクロール量 */
				&rcScrol,	/* スクロール長方形の構造体のアドレス */
				NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
			);
			::InvalidateRect( m_hWnd, &rcClip, TRUE );
			::UpdateWindow( m_hWnd );
		}
	}

//	/* 再描画 */
//	hdc = ::GetDC( m_hWnd );
//	ps.rcPaint.left = 0;
//	ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
//	ps.rcPaint.top = 0;
//	ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//	OnKillFocus();
//	OnPaint( hdc, &ps, FALSE );	/* メモリＤＣを使用してちらつきのない再描画 */
//	OnSetFocus();
//	::ReleaseDC( m_hWnd, hdc );
	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	/* キャレットの表示・更新 */
	ShowEditCaret();

	return;
}




/* 指定左端桁位置へスクロール */
void CEditView::ScrollAtH( int nPos )
{
	int			nScrollColNum;
	RECT		rcScrol;
//	RECT		rcClip;
	RECT		rcClip2;
//	PAINTSTRUCT ps;
//	HDC			hdc;
	if( nPos < 0 ){
		nPos = 0;
	}else
	if( m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize - m_nViewRowNum < nPos ){
		nPos = m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize - m_nViewRowNum;
	}
	if( m_nViewLeftCol == nPos ){
		return;
	}
	/* 水平スクロール量（文字数）の算出 */
	nScrollColNum = m_nViewLeftCol - nPos;

//	m_nViewLeftCol = nPos;
//#ifdef _DEBUG
//		if( m_nMyIndex == 2 ){
//			MYTRACE( "%s(%d): m_nMyIndex == 2 m_nViewLeftCol = %d\n", __FILE__, __LINE__, m_nViewLeftCol );
//		}
//#endif
	/* スクロール */
	if( abs( nScrollColNum ) >= m_nViewColNum /*|| abs( nScrollRowNum ) >= m_nViewRowNum*/ ){
//		m_nViewTopLine -= nScrollRowNum;
		m_nViewLeftCol = nPos;
//		if( bDraw ){
			::InvalidateRect( m_hWnd, NULL, TRUE );
//			/* スクロールバーの状態を更新する */
//			AdjustScrollBars();
//		}
	}else{
//	}else
//	if( nScrollRowNum != 0 || nScrollColNum != 0 ){
		rcScrol.left = 0;
		rcScrol.right = m_nViewCx + m_nViewAlignLeft;
		rcScrol.top = m_nViewAlignTop;
		rcScrol.bottom = m_nViewCy + m_nViewAlignTop;
//		if( nScrollRowNum > 0 ){
//			rcScrol.bottom =
//				m_nViewCy + m_nViewAlignTop -
//				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//			m_nViewTopLine -= nScrollRowNum;
//			rcClip.left = 0;
//			rcClip.right = m_nViewCx + m_nViewAlignLeft;
//			rcClip.top = m_nViewAlignTop;
//			rcClip.bottom =
//				m_nViewAlignTop + nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//		}else
//		if( nScrollRowNum < 0 ){
//			rcScrol.top =
//				m_nViewAlignTop - nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//			m_nViewTopLine -= nScrollRowNum;
//			rcClip.left = 0;
//			rcClip.right = m_nViewCx + m_nViewAlignLeft;
//			rcClip.top =
//				m_nViewCy + m_nViewAlignTop +
//				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//			rcClip.bottom = m_nViewCy + m_nViewAlignTop;
//		}
		if( nScrollColNum > 0 ){
			rcScrol.left = m_nViewAlignLeft;
			rcScrol.right =
				m_nViewCx + m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.left = m_nViewAlignLeft;
			rcClip2.right = m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.top = m_nViewAlignTop;
			rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
		}else
		if( nScrollColNum < 0 ){
			rcScrol.left = m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.left =
				m_nViewCx + m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.right = m_nViewCx + m_nViewAlignLeft;
			rcClip2.top = m_nViewAlignTop;
			rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
		}
		m_nViewLeftCol = nPos;
		if( m_bDrawSWITCH ){
//			::ScrollWindow(
//				m_hWnd,	/* スクロールするウィンドウのハンドル */
//				nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),		/* 水平スクロール量 */
//				0/*nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight )*/,	/* 垂直スクロール量 */
//				&rcScrol,	/* スクロール長方形の構造体のアドレス */
//				NULL		/* クリッピング長方形の構造体のアドレス */
//			);
			::ScrollWindowEx(
				m_hWnd,
				nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),		/* 水平スクロール量 */
				0,	/* 垂直スクロール量 */
				&rcScrol,	/* スクロール長方形の構造体のアドレス */
				NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
			);
			::InvalidateRect( m_hWnd, &rcClip2, TRUE );
			::UpdateWindow( m_hWnd );
		}
	}
//	/* 再描画 */
//	hdc = ::GetDC( m_hWnd );
//	ps.rcPaint.left = 0;
//	ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
//	ps.rcPaint.top = 0;
//	ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//	OnKillFocus();
//	OnPaint( hdc, &ps, FALSE );	/* メモリＤＣを使用してちらつきのない再描画 */
//	OnSetFocus();
//	::ReleaseDC( m_hWnd, hdc );
	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	/* キャレットの表示・更新 */
	ShowEditCaret();



	return;
}

/* 選択範囲のデータを取得 */
/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
BOOL CEditView::GetSelectedData(
		CMemory&	cmemBuf,
		BOOL		bLineOnly,
		const char*	pszQuote,			/* 先頭に付ける引用符 */
		BOOL		bWithLineNumber,	/* 行番号を付与する */
//	Jul. 25, 2000 genta
		enumEOLType	neweol				//	コピー後の改行コード EOL_NONEはコード保存
)
{
	const char*		pLine;
	int				nLineLen;
	int				nLineNum;
	int				nIdxFrom;
	int				nIdxTo;
	RECT			rcSel;
	int				nRowNum;
	int				nLineNumCols;
	char*			pszLineNum;
	char*			pszSpaces = "                    ";
	const CLayout*	pcLayout;
	CEOL			appendEol( neweol );
	bool			addnl = false;

	/* 範囲選択がされていない */
	if( !IsTextSelected() ){
		return FALSE;
	}
	if( bWithLineNumber ){	/* 行番号を付与する */
		/* 行番号表示に必要な桁数を計算 */
		nLineNumCols = DetectWidthOfLineNumberArea_calculate();
		nLineNumCols += 1;
		pszLineNum = new char[nLineNumCols + 1];
	}


	if( m_bBeginBoxSelect ){	/* 矩形範囲選択中 */
		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,		/* 範囲選択開始行 */
			m_nSelectColmFrom,		/* 範囲選択開始桁 */
			m_nSelectLineTo,		/* 範囲選択終了行 */
			m_nSelectColmTo			/* 範囲選択終了桁 */
		);
//		cmemBuf.SetData( "", 0 );
		cmemBuf.SetDataSz( "" );
		nRowNum = 0;
		for( nLineNum = rcSel.top; nLineNum <= rcSel.bottom; ++nLineNum ){
//			if( nRowNum > 0 ){
//				cmemBuf.AppendSz( CRLF );
//				if( bLineOnly ){	/* 複数行選択の場合は先頭の行のみ */
//					break;
//				}
//			}
//			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( nLineNum, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom	= LineColmnToIndex( pLine, nLineLen, rcSel.left  );
				nIdxTo		= LineColmnToIndex( pLine, nLineLen, rcSel.right );
			}
			if( nIdxTo - nIdxFrom > 0 ){
				if( pLine[nIdxTo - 1] == '\n' || pLine[nIdxTo - 1] == '\r' ){
					cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 );
				}else{
					cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
				}
			}
			++nRowNum;
//			if( nRowNum > 0 ){
				cmemBuf.AppendSz( CRLF );
				if( bLineOnly ){	/* 複数行選択の場合は先頭の行のみ */
					break;
				}
//			}
		}
	}else{
		cmemBuf.SetDataSz( "" );
		for( nLineNum = m_nSelectLineFrom; nLineNum <= m_nSelectLineTo; ++nLineNum ){
//			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( nLineNum, &nLineLen, &pcLayout );
			if( NULL == pLine ){
				break;
			}
			if( nLineNum == m_nSelectLineFrom ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom = LineColmnToIndex( pLine, nLineLen, m_nSelectColmFrom );
			}else{
				nIdxFrom = 0;
			}
			if( nLineNum == m_nSelectLineTo ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxTo = LineColmnToIndex( pLine, nLineLen, m_nSelectColmTo );
//				if( EOL_NONE != pcLayout->m_cEol && nIdxTo >= nLineLen ){
//					nIdxTo = nLineLen + pcLayout->m_cEol.GetLen() - 1;
//				}
			}else{
				nIdxTo = nLineLen;// + (pcLayout->m_cEol.GetLen()?pcLayout->m_cEol.GetLen()-1:0) ;
			}
			if( nIdxTo - nIdxFrom == 0 ){
				continue;
			}

#if 0
			//	Jul. 25, 2000	genta
			//	改行処理変更のため削除
			/* 改行も処理するんかぃのぉ・・・？ */
			if( EOL_NONE != pcLayout->m_cEol && nIdxTo >= nLineLen ){
				nIdxTo = nLineLen - 1;
			}
#endif

			if( NULL != pszQuote && 0 < lstrlen( pszQuote ) ){	/* 先頭に付ける引用符 */
//				cmemBuf.Append( pszQuote, lstrlen( pszQuote ) );
				cmemBuf.AppendSz( pszQuote );
			}
			if( bWithLineNumber ){	/* 行番号を付与する */
				wsprintf( pszLineNum, " %d:" , nLineNum + 1 );
				cmemBuf.Append( pszSpaces, nLineNumCols - lstrlen( pszLineNum ) );
//				cmemBuf.Append( pszLineNum, lstrlen( pszLineNum ) );
				cmemBuf.AppendSz( pszLineNum );
			}


			if( EOL_NONE != pcLayout->m_cEol ){
//			if( pLine[nIdxTo - 1] == '\n' || pLine[nIdxTo - 1] == '\r' ){
//				cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 );
//				cmemBuf.AppendSz( CRLF );

				if( nIdxTo >= nLineLen ){
					cmemBuf.Append( &pLine[nIdxFrom], nLineLen - 1 - nIdxFrom );
					//	Jul. 25, 2000 genta
					cmemBuf.AppendSz( ( neweol == EOL_UNKNOWN ) ?
						(pcLayout->m_cEol).GetValue() :	//	コード保存
						appendEol.GetValue() );			//	新規改行コード
				}
				else {
					cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
				}
			}else{
				cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
				if( nIdxTo - nIdxFrom >= nLineLen ){
					if( m_pShareData->m_Common.m_bAddCRLFWhenCopy ||  /* 折り返し行に改行を付けてコピー */
						NULL != pszQuote || /* 先頭に付ける引用符 */
						bWithLineNumber 	/* 行番号を付与する */
					){
//						cmemBuf.Append( CRLF, lstrlen( CRLF ) );
						//	Jul. 25, 2000 genta
						cmemBuf.AppendSz(( neweol == EOL_UNKNOWN ) ?
							CRLF :						//	コード保存
							appendEol.GetValue() );		//	新規改行コード
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
	return TRUE;
}




/* 選択範囲内の全行をクリップボードにコピーする */
void CEditView::CopySelectedAllLines(
	const char*	pszQuote,		/* 先頭に付ける引用符 */
	BOOL		bWithLineNumber	/* 行番号を付与する */
)
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	int			nSelectLineFromOld;	/* 範囲選択開始行 */
	int			nSelectColFromOld; 	/* 範囲選択開始桁 */
	int			nSelectLineToOld;	/* 範囲選択終了行 */
	int			nSelectColToOld;	/* 範囲選択終了桁 */
	RECT		rcSel;
	CMemory		cmemBuf;
//	HGLOBAL		hgClip;
//	char*		pszClip;
//	const char*	pLine;
//	int			nLineLen;
	if( !IsTextSelected() ){	/* テキストが選択されているか */
		return;
	}
	/* 矩形範囲選択中か */
	if( m_bBeginBoxSelect ){
		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,					/* 範囲選択開始行 */
			m_nSelectColmFrom,					/* 範囲選択開始桁 */
			m_nSelectLineTo,					/* 範囲選択終了行 */
			m_nSelectColmTo						/* 範囲選択終了桁 */
		);
		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( TRUE );
//		/* 挿入データの先頭位置へカーソルを移動 */
//		MoveCursor( rcSel.left, rcSel.top, FALSE );
		m_nSelectLineFrom = rcSel.top;			/* 範囲選択開始行 */
		m_nSelectColmFrom = 0;					/* 範囲選択開始桁 */
		m_nSelectLineTo = rcSel.bottom + 1;		/* 範囲選択終了行 */
		m_nSelectColmTo = 0;					/* 範囲選択終了桁 */
		m_bBeginBoxSelect = FALSE;
	}else{
		nSelectLineFromOld = m_nSelectLineFrom;	/* 範囲選択開始行 */
		nSelectColFromOld = 0;					/* 範囲選択開始桁 */
		nSelectLineToOld = m_nSelectLineTo;		/* 範囲選択終了行 */
		if( m_nSelectColmTo > 0 ){
			++nSelectLineToOld;					/* 範囲選択終了行 */
		}
		nSelectColToOld = 0;					/* 範囲選択終了桁 */
		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( TRUE );
		m_nSelectLineFrom = nSelectLineFromOld;	/* 範囲選択開始行 */
		m_nSelectColmFrom = nSelectColFromOld; 	/* 範囲選択開始桁 */
		m_nSelectLineTo = nSelectLineToOld;		/* 範囲選択終了行 */
		m_nSelectColmTo = nSelectColToOld;		/* 範囲選択終了桁 */
	}
	/* 再描画 */
	//	::UpdateWindow();
	hdc = ::GetDC( m_hWnd );
	ps.rcPaint.left = 0;
	ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
	ps.rcPaint.top = m_nViewAlignTop;
	ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
	OnKillFocus();
	OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
	OnSetFocus();
	::ReleaseDC( m_hWnd, hdc );
	/* 選択範囲をクリップボードにコピー */
	/* 選択範囲のデータを取得 */
	/* 正常時はTRUE,範囲未選択の場合は終了する */
	if( FALSE == GetSelectedData(
		cmemBuf,
		FALSE,
		pszQuote, /* 引用符 */
		bWithLineNumber /* 行番号を付与する */
	) ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	/* クリップボードにデータを設定 */
	MySetClipboardData( cmemBuf.GetPtr( NULL ), cmemBuf.GetLength(), FALSE );


//	/* Windowsクリップボードにコピー */
//	hgClip = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, cmemBuf.GetLength() + 1 );
//	pszClip = (char*)::GlobalLock( hgClip );
//	memcpy( pszClip, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() + 1 );
//	::GlobalUnlock( hgClip );
//	::OpenClipboard( m_hWnd );
//	::EmptyClipboard();
//	::SetClipboardData( CF_OEMTEXT, hgClip );
//	::CloseClipboard();
	return;
}






/* 選択エリアのテキストを指定方法で変換 */
void CEditView::ConvSelectedArea( int nFuncCode )
{
	CMemory		cmemBuf;
	int			nNewLine;		/* 挿入された部分の次の位置の行 */
	int			nNewPos;		/* 挿入された部分の次の位置のデータ位置 */
	COpe*		pcOpe = NULL;
//	BOOL		bBoxSelected;
//	HDC			hdc;
//	PAINTSTRUCT	ps;
	RECT		rcSel;

	int			nPosX;
	int			nPosY;
	int			nIdxFrom;
	int			nIdxTo;
	int			nLineNum;
	int			nDelPos;
	int			nDelLen;
	int			nDelPosNext;
	int			nDelLenNext;
	const char*	pLine;
	int			nLineLen;
	const char*	pLine2;
	int			nLineLen2;
	int			i;
	CMemory*	pcMemDeleted;
	CWaitCursor cWaitCursor( m_hWnd );

	int			nSelectLineFromOld;				/* 範囲選択開始行 */
	int			nSelectColFromOld; 				/* 範囲選択開始桁 */
	int			nSelectLineToOld;				/* 範囲選択終了行 */
	int			nSelectColToOld;				/* 範囲選択終了桁 */
	BOOL		bBeginBoxSelectOld;

	/* テキストが選択されているか */
	if( !IsTextSelected() ){
		return;
	}

	nSelectLineFromOld	= m_nSelectLineFrom;	/* 範囲選択開始行 */
	nSelectColFromOld	= m_nSelectColmFrom;	/* 範囲選択開始桁 */
	nSelectLineToOld	= m_nSelectLineTo;		/* 範囲選択終了行 */
	nSelectColToOld		= m_nSelectColmTo;		/* 範囲選択終了桁 */
	bBeginBoxSelectOld	= m_bBeginBoxSelect;


	/* 矩形範囲選択中か */
	if( m_bBeginBoxSelect ){

		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,					/* 範囲選択開始行 */
			m_nSelectColmFrom,					/* 範囲選択開始桁 */
			m_nSelectLineTo,					/* 範囲選択終了行 */
			m_nSelectColmTo						/* 範囲選択終了桁 */
		);

		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( TRUE );

		nIdxFrom = 0;
		nIdxTo = 0;
		for( nLineNum = rcSel.bottom; nLineNum >= rcSel.top - 1; nLineNum-- ){
			nDelPosNext = nIdxFrom;
			nDelLenNext	= nIdxTo - nIdxFrom;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			if( NULL != pLine ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom	= LineColmnToIndex( pLine, nLineLen, rcSel.left );
				nIdxTo		= LineColmnToIndex( pLine, nLineLen, rcSel.right );

				for( i = nIdxFrom; i <= nIdxTo; ++i ){
					if( pLine[i] == CR || pLine[i] == LF ){
						nIdxTo = i;
						break;
					}
				}
			}else{
				nIdxFrom	= 0;
				nIdxTo		= 0;
			}
			nDelPos = nDelPosNext;
			nDelLen	= nDelLenNext;
			if( nLineNum < rcSel.bottom && 0 < nDelLen ){
				pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum + 1, &nLineLen2 );
				nPosX = LineIndexToColmn( pLine2, nLineLen2, nDelPos );
				nPosY =  nLineNum + 1;
				if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
					pcOpe = new COpe;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
						nPosX,
						nPosY,
						&pcOpe->m_nCaretPosX_PHY_Before,
						&pcOpe->m_nCaretPosY_PHY_Before
					);
			    }else{
			    	pcOpe = NULL;
			    }

				pcMemDeleted = new CMemory;
				/* 指定位置の指定長データ削除 */
				DeleteData2(
					nPosX/*rcSel.left*/,
					nPosY/*nLineNum + 1*/,
					nDelLen,
					pcMemDeleted,
					pcOpe,		/* 編集操作要素 COpe */
					FALSE,
					FALSE
				);
				cmemBuf.SetData( pcMemDeleted );
				if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
						nPosX,
						nPosY,
						&pcOpe->m_nCaretPosX_PHY_After,
						&pcOpe->m_nCaretPosY_PHY_After
					);
					/* 操作の追加 */
					m_pcOpeBlk->AppendOpe( pcOpe );
				}else{
					delete pcMemDeleted;
					pcMemDeleted = NULL;
				}
				/* 機能種別によるバッファの変換 */
				ConvMemory( &cmemBuf, nFuncCode );
				if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
					pcOpe = new COpe;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
						nPosX,
						nPosY,
						&pcOpe->m_nCaretPosX_PHY_Before,
						&pcOpe->m_nCaretPosY_PHY_Before
					);
				}
				/* 現在位置にデータを挿入 */
				InsertData_CEditView(
					nPosX,
					nPosY,
					cmemBuf.GetPtr( NULL ),
					cmemBuf.GetLength(),
					&nNewLine,
					&nNewPos,
					pcOpe,
					TRUE/*FALSE*/
				);
				/* カーソルを移動 */
				MoveCursor( nNewPos, nNewLine, FALSE );
				m_nCaretPosX_Prev = m_nCaretPosX;
				if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
					pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* 操作後のキャレット位置Ｘ */
					pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* 操作後のキャレット位置Ｙ */
					/* 操作の追加 */
					m_pcOpeBlk->AppendOpe( pcOpe );
				}
			}
		}
		/* 挿入データの先頭位置へカーソルを移動 */
		MoveCursor( rcSel.left, rcSel.top, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;

		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;									/* 操作種別 */
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* 操作前のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* 操作前のキャレット位置Ｙ */
			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* 操作後のキャレット位置Ｙ */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
	}else{
		/* 選択範囲のデータを取得 */
		/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
		GetSelectedData( cmemBuf, FALSE, NULL, FALSE );

		/* 機能種別によるバッファの変換 */
		ConvMemory( &cmemBuf, nFuncCode );

//		/* 選択エリアを削除 */
//		DeleteData( FALSE );

		/* データ置換 削除&挿入にも使える */
		ReplaceData_CEditView(
			m_nSelectLineFrom,		/* 範囲選択開始行 */
			m_nSelectColmFrom,		/* 範囲選択開始桁 */
			m_nSelectLineTo,		/* 範囲選択終了行 */
			m_nSelectColmTo,		/* 範囲選択終了桁 */
			NULL,					/* 削除されたデータのコピー(NULL可能) */
			cmemBuf.m_pData,		/* 挿入するデータ */
			cmemBuf.m_nDataLen,		/* 挿入するデータの長さ */
			TRUE/*bRedraw*/
		);
		return;


#if 0///////////////////////////////
		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* 操作前のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* 操作前のキャレット位置Ｙ */
		}
		/* 現在位置にデータを挿入 */
		InsertData_CEditView(
			m_nCaretPosX,
			m_nCaretPosY,
			cmemBuf.GetPtr( NULL ),
			cmemBuf.GetLength(),
			&nNewLine,
			&nNewPos,
			pcOpe,
			TRUE/*FALSE*/
		);

		/* カーソルを移動 */
		MoveCursor( nNewPos, nNewLine, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;

		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* 操作後のキャレット位置Ｙ */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
#endif ///////////////////////////////

	}
#if 0///////////////////////////////
	m_nSelectLineFrom	= nSelectLineFromOld;	/* 範囲選択開始行 */
	m_nSelectColmFrom	= nSelectColFromOld;	/* 範囲選択開始桁 */
	m_nSelectLineTo		= nSelectLineToOld;		/* 範囲選択終了行 */
	m_nSelectColmTo		= nSelectColToOld;		/* 範囲選択終了桁 */
	m_bBeginBoxSelect	= bBeginBoxSelectOld;

	m_pcEditDoc->m_bIsModified = TRUE;	/* 変更フラグ */
	SetParentCaption();					/* 親ウィンドウのタイトルを更新 */

	/* 再描画 */
	//	::UpdateWindow();
	hdc = ::GetDC( m_hWnd );
	ps.rcPaint.left		= 0;
	ps.rcPaint.right	= m_nViewAlignLeft + m_nViewCx;
	ps.rcPaint.top		= m_nViewAlignTop;
	ps.rcPaint.bottom	= m_nViewAlignTop + m_nViewCy;
	OnKillFocus();
	OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
	OnSetFocus();
	::ReleaseDC( m_hWnd, hdc );
#endif ///////////////////////////////


	return;
}


/* 機能種別によるバッファの変換 */
void CEditView::ConvMemory( CMemory* pCMemory, int nFuncCode )
{
	switch( nFuncCode ){
	case F_TOLOWER: pCMemory->ToLower(); break;						/* 英大文字→英小文字 */
	case F_TOUPPER: pCMemory->ToUpper(); break;						/* 英小文字→英大文字 */
	case F_TOHANKAKU: pCMemory->ToHankaku(); break;					/* 全角→半角 */
	case F_TOZENKAKUKATA: pCMemory->ToZenkaku( 0, 0 );			/* 1== ひらがな 0==カタカナ */ break;	/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	case F_TOZENKAKUHIRA: pCMemory->ToZenkaku( 1, 0 );			/* 1== ひらがな 0==カタカナ */ break;	/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	case F_HANKATATOZENKAKUKATA: pCMemory->ToZenkaku( 0, 1 );	/* 1== ひらがな 0==カタカナ */ break;	/* 半角カタカナ→全角カタカナ */
	case F_HANKATATOZENKAKUHIRA: pCMemory->ToZenkaku( 1, 1 );	/* 1== ひらがな 0==カタカナ */ break;	/* 半角カタカナ→全角ひらがな */
	case F_CODECNV_EMAIL:		pCMemory->JIStoSJIS(); break;		/* E-Mail(JIS→SJIS)コード変換 */
	case F_CODECNV_EUC2SJIS:	pCMemory->EUCToSJIS(); break;		/* EUC→SJISコード変換 */
	case F_CODECNV_UNICODE2SJIS:pCMemory->UnicodeToSJIS(); break;	/* Unicode→SJISコード変換 */
	case F_CODECNV_SJIS2JIS:	pCMemory->SJIStoJIS();break;		/* SJIS→JISコード変換 */
	case F_CODECNV_SJIS2EUC: 	pCMemory->SJISToEUC();break;		/* SJIS→EUCコード変換 */
	case F_CODECNV_UTF82SJIS:	pCMemory->UTF8ToSJIS();break;		/* UTF-8→SJISコード変換 */
	case F_CODECNV_UTF72SJIS:	pCMemory->UTF7ToSJIS();break;		/* UTF-7→SJISコード変換 */
	case F_CODECNV_SJIS2UTF7:	pCMemory->SJISToUTF7();break;		/* SJIS→UTF-7コード変換 */
	case F_CODECNV_SJIS2UTF8:	pCMemory->SJISToUTF8();break;		/* SJIS→UTF-8コード変換 */
	case F_CODECNV_AUTO2SJIS:	pCMemory->AUTOToSJIS();break;		/* 自動判別→SJISコード変換 */
	case F_TABTOSPACE:
		pCMemory->TABToSPACE(
			m_pcEditDoc->GetDocumentAttribute().m_nTabSpace
		);break;	/* TAB→空白 */
	case F_SPACETOTAB:	//#### Stonee, 2001/05/27
		pCMemory->SPACEToTAB(
			m_pcEditDoc->GetDocumentAttribute().m_nTabSpace
		);
		break;		/* 空白→TAB */
	}
	return;

}







/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */
int CEditView::LineColmnToIndex( const char* pLine, int nLineLen, int nColmn )
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CEditView::LineColmnToIndex" );
//#endif
	int		nPosX;
	int		i;
	int		nCharChars;
	nPosX = 0;
//	*pnLineAllColLen = 0;
	for( i = 0; i < nLineLen; ){
		if( pLine[i] == TAB ){
			nCharChars = m_pcEditDoc->GetDocumentAttribute().m_nTabSpace - ( nPosX % m_pcEditDoc->GetDocumentAttribute().m_nTabSpace );
			if( nPosX + nCharChars > nColmn ){
				break;
			}
			++i;
		}else{
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
			if( nPosX + nCharChars > nColmn ){
				break;
			}
			i+= nCharChars;
		}
		nPosX += nCharChars;
	}
//	if( i >= nLineLen  ){
//		*pnLineAllColLen = nPosX;
//	}
	return i;
}



/* 指定された桁に対応する行のデータ内の位置を調べる Ver0 */
/* 指定された桁より、行が短い場合はpnLineAllColLenに行全体の表示桁数を返す */
/* それ以外の場合はpnLineAllColLenに０をセットする */
int CEditView::LineColmnToIndex2( const char* pLine, int nLineLen, int nColmn, int* pnLineAllColLen )
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CEditView::LineColmnToIndex(0)" );
#endif
	int		nPosX;
	int		i;
	int		nCharChars;
//	int		bEOL = FALSE;
	nPosX = 0;
	*pnLineAllColLen = 0;
	for( i = 0; i < nLineLen; ){
//		if( bEOL ){
//			break;
//		}
//		if( pLine[i] == '\r' || pLine[i] == '\n' ){
//			bEOL = TRUE;
//		}
		if( pLine[i] == TAB ){
			nCharChars = m_pcEditDoc->GetDocumentAttribute().m_nTabSpace - ( nPosX % m_pcEditDoc->GetDocumentAttribute().m_nTabSpace );
			if( nPosX + nCharChars > nColmn ){
				break;
			}
			++i;
		}else{
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
			if( nPosX + nCharChars > nColmn ){
				break;
			}
			i+= nCharChars;
		}
		nPosX += nCharChars;
	}
	if( i >= nLineLen  ){
		*pnLineAllColLen = nPosX;
	}
	return i;
}





/*
||	指定された行のデータ内の位置に対応する桁の位置を調べる
||
*/
int CEditView::LineIndexToColmn( const char* pLine, int nLineLen, int nIndex )
{
	int		nPosX;
	int		i;
	int		nCharChars;
	nPosX = 0;
	for( i = 0; i < nLineLen; ){
		if( i >= nIndex ){
			break;
		}
		if( pLine[i] == TAB ){
			nCharChars = m_pcEditDoc->GetDocumentAttribute().m_nTabSpace - ( nPosX % m_pcEditDoc->GetDocumentAttribute().m_nTabSpace );
			++i;
		}else{
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
			i+= nCharChars;
		}
		nPosX += nCharChars;
	}
	return nPosX;
}





/* ポップアップメニュー(右クリック) */
int	CEditView::CreatePopUpMenu_R( void )
{
	int			nId;
//	HMENU		hMenuTop;
	HMENU		hMenu;
	POINT		po;
//	UINT		fuFlags;
//	int			cMenuItems;
//	int			nPos;
	RECT		rc;
	CMemory		cmemCurText;
	CMemory*	pcmemRefText;
	char*		pszWork;
	int			nWorkLength;
	int			i;
	int			nMenuIdx;
	char		szLabel[300];
	char		szLabel2[300];
	UINT		uFlags;
//	BOOL		bBool;


	CEditWnd*	pCEditWnd;
	pCEditWnd = ( CEditWnd* )::GetWindowLong( ::GetParent( m_hwndParent ), GWL_USERDATA );
	pCEditWnd->m_CMenuDrawer.ResetContents();

	/* 右クリックメニューの定義はカスタムメニュー配列の0番目 */
	nMenuIdx = 0;
//	if( nMenuIdx < 0 || MAX_CUSTOM_MENU	<= nMenuIdx ){
//		return 0;
//	}
//	if( 0 == m_pShareData->m_Common.m_nCustMenuItemNumArr[nMenuIdx] ){
//		return 0;
//	}



	hMenu = ::CreatePopupMenu();
	for( i = 0; i < m_pShareData->m_Common.m_nCustMenuItemNumArr[nMenuIdx]; ++i ){
		if( 0 == m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i] ){
			::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
		}else{
			::LoadString( m_hInstance, m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
			/* キー */
			if( '\0' == m_pShareData->m_Common.m_nCustMenuItemKeyArr[nMenuIdx][i] ){
				strcpy( szLabel2, szLabel );
			}else{
				wsprintf( szLabel2, "%s (&%c)", szLabel, m_pShareData->m_Common.m_nCustMenuItemKeyArr[nMenuIdx][i] );
			}
			/* 機能が利用可能か調べる */
			if( TRUE == CEditWnd::IsFuncEnable( m_pcEditDoc, m_pShareData, m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i] ) ){
				uFlags = MF_STRING | MF_ENABLED;
			}else{
				uFlags = MF_STRING | MF_DISABLED | MF_GRAYED;
			}
//			bBool = ::AppendMenu( hMenu, uFlags, m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel2 );
			pCEditWnd->m_CMenuDrawer.MyAppendMenu(
				hMenu, /*MF_BYPOSITION | MF_STRING*/uFlags,
				m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i] , szLabel2 );

		}
	}


	if( !m_bBeginSelect ){	/* 範囲選択中 */
		//	2001/06/14 asa-o 参照するデータの変更
//		if( m_pShareData->m_Common.m_bUseKeyWordHelp ){ /* キーワードヘルプを使用する */
		if( m_pcEditDoc->GetDocumentAttribute().m_bUseKeyWordHelp ){
			if( m_nCaretWidth > 0 ){					//フォーカスがあるとき
				/* ウィンドウ内にマウスカーソルがあるか？ */
				GetCursorPos( &po );
				GetWindowRect( m_hWnd, &rc );
				if( PtInRect( &rc, po ) ){
					if( m_bInMenuLoop == FALSE	//&&				/* メニュー モーダル ループに入っていない */
						//0 != m_dwTipTimer		&&					/* 辞書Tipを表示していない */
						//1000 < ::GetTickCount() - m_dwTipTimer	/* 一定時間以上、マウスが固定されている */
					){
						/* 選択範囲のデータを取得(複数行選択の場合は先頭の行のみ) */
						if( GetSelectedData( cmemCurText, TRUE, NULL, FALSE ) ){
							pszWork = cmemCurText.GetPtr( NULL );
							nWorkLength	= lstrlen( pszWork );
							for( i = 0; i < nWorkLength; ++i ){
								if( pszWork[i] == '\0' ||
									pszWork[i] == CR ||
									pszWork[i] == LF ){
									break;
								}
							}
							char*	pszBuf = new char[i + 1];
							memcpy( pszBuf, pszWork, i );
							pszBuf[i] = '\0';
							cmemCurText.SetData( pszBuf, i );
							delete [] pszBuf;


							/* 既に検索済みか */
							if( CMemory::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) ){
								/* 該当するキーがなかった */
								if( !m_cTipWnd.m_KeyWasHit ){
									goto end_of_search;
								}
							}else{
								m_cTipWnd.m_cKey = cmemCurText;
								/* 検索実行 */
								//	2001/06/14 asa-o 参照するデータの変更
	//							if( m_cDicMgr.Search( cmemCurText.GetPtr( NULL ), &pcmemRefText, m_pShareData->m_Common.m_szKeyWordHelpFile ) ){
								if( m_cDicMgr.Search( cmemCurText.GetPtr( NULL ), &pcmemRefText, m_pcEditDoc->GetDocumentAttribute().m_szKeyWordHelpFile ) ){
									/* 該当するキーがある */
									m_cTipWnd.m_KeyWasHit = TRUE;
									pszWork = pcmemRefText->GetPtr( NULL );
//									m_cTipWnd.m_cInfo.SetData( pszWork, lstrlen( pszWork ) );
									m_cTipWnd.m_cInfo.SetDataSz( pszWork );
									delete pcmemRefText;
								}else{
									/* 該当するキーがなかった */
									m_cTipWnd.m_KeyWasHit = FALSE;
									goto end_of_search;
								}
							}
							m_dwTipTimer = 0;	/* 辞書Tipを表示している */
							m_poTipCurPos = po;	/* 現在のマウスカーソル位置 */
	//						/* 辞書Tipを表示 */
	//						m_cTipWnd.Show( po.x, po.y + m_nCharHeight, NULL );
							pszWork = m_cTipWnd.m_cInfo.GetPtr(NULL);
							if( 80 < lstrlen( pszWork ) ){
								char*	pszShort = new char[80 + 1];
								memcpy( pszShort, pszWork, 80 );
								pszShort[80] = '\0';
								::InsertMenu( hMenu, 0, MF_BYPOSITION, IDM_COPYDICINFO, pszShort );
								delete [] pszShort;
							}else{
								::InsertMenu( hMenu, 0, MF_BYPOSITION, IDM_COPYDICINFO, pszWork );
							}
							::InsertMenu( hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
						}
						end_of_search:;
					}
				}
			}
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
		::GetParent( m_hwndParent )/*m_hWnd*/,
		NULL
	);
//	::DestroyMenu( hMenuTop );
	::DestroyMenu( hMenu );
//	MYTRACE( "nId=%d\n", nId );
	return nId;
}



/* 親ウィンドウのタイトルを更新 */
void CEditView::SetParentCaption( BOOL bKillFocus )
{
	m_pcEditDoc->SetParentCaption( bKillFocus );
	return;
}


/* キャレットの行桁位置を表示する */
void CEditView::DrawCaretPosInfo( void )
{
	if( !m_bDrawSWITCH ){
		return;
	}

	HDC				hdc;
	POINT			poFrame;
	POINT			po;
	RECT			rcFrame;
	HFONT			hFontOld;
	char			szText[64];
	HWND			hwndFrame;
	int				nStrLen;
	RECT			rc;
	unsigned char*	pLine;
	int				nLineLen;
	int				nIdxFrom;
	int				nCharChars;
	CEditWnd*		pCEditWnd;
	const CLayout*	pcLayout;
	char*			pCodeNameArr[] = {
		"SJIS",
		"JIS ",
		"EUC ",
		"Uni ",
		"UTF-8",
		"UTF-7"
	};
	char*			pCodeNameArr2[] = {
		"SJIS",
		"JIS ",
		"EUC ",
		"Unicode",
		"UTF-8",
		"UTF-7"
	};
	int	nCodeNameArrNum = sizeof( pCodeNameArr ) / sizeof( pCodeNameArr[0] );

	hwndFrame = ::GetParent( m_hwndParent );
	pCEditWnd = ( CEditWnd* )::GetWindowLong( hwndFrame, GWL_USERDATA );
	/* カーソル位置の文字コード */
//	pLine = (unsigned char*)m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen );
	pLine = (unsigned char*)m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY, &nLineLen, &pcLayout );

	//	May 12, 2000 genta
	//	改行コードの表示を追加
	//	From Here
	CEOL cNlType = m_pcEditDoc->GetNewLineCode();
	const char *nNlTypeName = cNlType.GetName();
	//	To Here


	/* ステータスバーに状態を書き出す */
	if( NULL == pCEditWnd->m_hwndStatusBar ){
		hdc = ::GetWindowDC( hwndFrame );
		poFrame.x = 0;
		poFrame.y = 0;
		::ClientToScreen( hwndFrame, &poFrame );
		::GetWindowRect( hwndFrame, &rcFrame );
		po.x = rcFrame.right - rcFrame.left;
		po.y = poFrame.y - rcFrame.top;
		hFontOld = (HFONT)::SelectObject( hdc, m_hFontCaretPosInfo );
		//	May 12, 2000 genta
		//	改行コードの表示を追加
		//	From Here
		if( NULL != pLine ){
			/* 指定された桁に対応する行のデータ内の位置を調べる */
			nIdxFrom = LineColmnToIndex( (const char *)pLine, nLineLen, m_nCaretPosX );
			if( nIdxFrom >= nLineLen ){
				/* szText */
//				wsprintf( szText, "%s(%s)       %6d 行、%d桁          ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, m_nCaretPosY + 1, m_nCaretPosX + 1 );
				wsprintf( szText, "%s(%s)       %6d：%d            ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, m_nCaretPosY + 1, m_nCaretPosX + 1 );	//Oct. 31, 2000 JEPRO //Oct. 31, 2000 JEPRO メニューバーでの表示桁を節約
			}else{
				if( nIdxFrom < nLineLen - (pcLayout->m_cEol.GetLen()?1:0) ){
					nCharChars = CMemory::MemCharNext( (char *)pLine, nLineLen, (char *)&pLine[nIdxFrom] ) - (char *)&pLine[nIdxFrom];
				}else{
					nCharChars = pcLayout->m_cEol.GetLen();
				}
				if( 1 == nCharChars ){
					/* szText */
//					wsprintf( szText, "%s(%s)   [%02x]%6d 行、%d桁          ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, pLine[nIdxFrom], m_nCaretPosY + 1, m_nCaretPosX + 1 );
					wsprintf( szText, "%s(%s)   [%02x]%6d：%d            ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, pLine[nIdxFrom], m_nCaretPosY + 1, m_nCaretPosX + 1 );//Oct. 31, 2000 JEPRO メニューバーでの表示桁を節約
				}else
				if( 2 == nCharChars ){
					/* szText */
//					wsprintf( szText, "%s(%s) [%02x%02x]%6d 行、%d桁          ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , m_nCaretPosY + 1, m_nCaretPosX + 1);
					wsprintf( szText, "%s(%s) [%02x%02x]%6d：%d            ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , m_nCaretPosY + 1, m_nCaretPosX + 1);//Oct. 31, 2000 JEPRO メニューバーでの表示桁を節約
				}else
				if( 4 == nCharChars ){
					/* szText */
//					wsprintf( szText, "%s(%s) [%02x%02x%02x%02x]%d 行、%d桁          ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , pLine[nIdxFrom + 2],  pLine[nIdxFrom + 3] , m_nCaretPosY + 1, m_nCaretPosX + 1);
					wsprintf( szText, "%s(%s) [%02x%02x%02x%02x]%d：%d            ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , pLine[nIdxFrom + 2],  pLine[nIdxFrom + 3] , m_nCaretPosY + 1, m_nCaretPosX + 1);//Oct. 31, 2000 JEPRO メニューバーでの表示桁を節約
				}else{
					/* szText */
//					wsprintf( szText, "%s(%s)       %6d 行、%d桁          ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, m_nCaretPosY + 1, m_nCaretPosX + 1 );
					wsprintf( szText, "%s(%s)       %6d：%d            ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, m_nCaretPosY + 1, m_nCaretPosX + 1 );//Oct. 31, 2000 JEPRO メニューバーでの表示桁を節約
				}
			}
		}else{
			/* szText */
//			wsprintf( szText, "%s(%s)       %6d 行、%d桁          ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, m_nCaretPosY + 1, m_nCaretPosX + 1 );
			wsprintf( szText, "%s(%s)       %6d：%d            ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, m_nCaretPosY + 1, m_nCaretPosX + 1 );//Oct. 31, 2000 JEPRO メニューバーでの表示桁を節約
		}
		//	To Here
		/* 文字列描画 */
//		nStrLen = 35;
		nStrLen = 30;	//Oct. 31, 2000 JEPRO メニューバーでの表示桁を節約
		rc.left = po.x - nStrLen * m_nCaretPosInfoCharWidth - 5;
		rc.right = rc.left + nStrLen * m_nCaretPosInfoCharWidth;
		rc.top = po.y - m_nCaretPosInfoCharHeight - 2;
		rc.bottom = rc.top + m_nCaretPosInfoCharHeight;
		::SetTextColor( hdc, ::GetSysColor( COLOR_MENUTEXT ) );
		::SetBkColor( hdc, ::GetSysColor( COLOR_MENU ) );
		::ExtTextOut(
			hdc,
			rc.left,
			rc.top,
			/*ETO_CLIPPED | */ ETO_OPAQUE,
			&rc,
			szText,
			nStrLen,
			m_pnCaretPosInfoDx
		);
		::SelectObject( hdc, hFontOld );
		::ReleaseDC( hwndFrame, hdc );
	}else{
		char	szText_1[64];
		char	szText_2[64];
//		char	szText_3[64];
//		char	szText_4[64];
		char	szText_5[64];
//		wsprintf( szText_1, "%8d 行 %5d 桁", m_nCaretPosY + 1, m_nCaretPosX + 1 );
		wsprintf( szText_1, "%6d 行 %5d 桁", m_nCaretPosY + 1, m_nCaretPosX + 1 );	//Oct. 30, 2000 JEPRO 千万行も要らん

		nCharChars = 0;
		if( NULL != pLine ){
			/* 指定された桁に対応する行のデータ内の位置を調べる */
			nIdxFrom = LineColmnToIndex( (const char *)pLine, nLineLen, m_nCaretPosX );
			if( nIdxFrom >= nLineLen ){
			}else{
				if( nIdxFrom < nLineLen - (pcLayout->m_cEol.GetLen()?1:0) ){
					nCharChars = CMemory::MemCharNext( (char *)pLine, nLineLen, (char *)&pLine[nIdxFrom] ) - (char *)&pLine[nIdxFrom];
				}else{
					nCharChars = pcLayout->m_cEol.GetLen();
				}
			}
		}

		if( 1 == nCharChars ){
			wsprintf( szText_2, "%02x  ", pLine[nIdxFrom] );
		}else
		if( 2 == nCharChars ){
			wsprintf( szText_2, "%02x%02x", pLine[nIdxFrom],  pLine[nIdxFrom + 1] );
		}else
		if( 4 == nCharChars ){
			wsprintf( szText_2, "%02x%02x%02x%02x", pLine[nIdxFrom],  pLine[nIdxFrom + 1], pLine[nIdxFrom + 2],  pLine[nIdxFrom + 3] );
		}else{
			wsprintf( szText_2, "    " );
		}

		if( m_pShareData->m_Common.m_bIsINSMode ){
			strcpy( szText_5, "挿入" );
		}else{
			strcpy( szText_5, "上書" );
		}
//		::GetClientRect( pCEditWnd->m_hwndStatusBar, &rc );
//		int			nStArr[7];
//		const char*	pszLabel[6] = { "", szText_1, szText_2, pCodeNameArr2[m_pcEditDoc->m_nCharCode], "REC", szText_5 };
//		int			nStArrNum = 6;
//		int			nAllWidth;
//		SIZE		sz;
//		HDC			hdc;
//		int			i;
//		TEXTMETRIC	tm;
//		nAllWidth = rc.right - rc.left;
//		hdc = ::GetDC( pCEditWnd->m_hwndStatusBar );
//		nStArr[nStArrNum - 1] = nAllWidth;
//		if( pCEditWnd->m_nWinSizeType != SIZE_MAXIMIZED ){	/* サイズ変更のタイプ */
//			nStArr[nStArrNum - 1] -= 16;
//		}
////		::GetTextMetrics( hdc, &tm );
//		for( i = nStArrNum - 1; i > 0; i-- ){
////			sz.cx = tm.tmMaxCharWidth * lstrlen( pszLabel[i] )  / 2;
//			::GetTextExtentPoint32( hdc, pszLabel[i], lstrlen( pszLabel[i] ), &sz );
//			nStArr[i - 1] = nStArr[i] - ( sz.cx + ::GetSystemMetrics( SM_CXEDGE ) );
//		}
//		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETPARTS, nStArrNum, (LPARAM) (LPINT)nStArr );
//		::ReleaseDC( pCEditWnd->m_hwndStatusBar, hdc );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)"" );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 1 | 0, (LPARAM) (LPINT)szText_1 );
		//	May 12, 2000 genta
		//	改行コードの表示を追加．後ろの番号を1つずつずらす
		//	From Here
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 2 | 0, (LPARAM) (LPINT)nNlTypeName );
		//	To Here
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 3 | 0, (LPARAM) (LPINT)szText_2 );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 4 | 0, (LPARAM)(LPINT)pCodeNameArr2[m_pcEditDoc->m_nCharCode] );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 5 | SBT_OWNERDRAW, (LPARAM) (LPINT)"" );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 6 | 0, (LPARAM) (LPINT)szText_5 );
	}

	return;
}





/* 設定変更を反映させる */
void CEditView::OnChangeSetting( void )
{
	RECT		rc;
	LOGFONT		lf;

	m_nTopYohaku = m_pShareData->m_Common.m_nRulerBottomSpace; 		/* ルーラーとテキストの隙間 */
	m_nViewAlignTop = m_nTopYohaku;									/* 表示域の上端座標 */
	/* ルーラー表示 */
//	if( m_pShareData->m_Common.m_bRulerDisp ){
	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		m_nViewAlignTop += m_pShareData->m_Common.m_nRulerHeight;	/* ルーラー高さ */
	}

	/* フォント作成 */
	::DeleteObject( m_hFont_HAN );
	m_hFont_HAN = CreateFontIndirect( &(m_pShareData->m_Common.m_lf) );

	/* 太字フォント作成 */
	::DeleteObject( m_hFont_HAN_FAT );
	lf = m_pShareData->m_Common.m_lf;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT = CreateFontIndirect( &lf );

	/* 下線フォント作成 */
	::DeleteObject( m_hFont_HAN_UL );
	lf = m_pShareData->m_Common.m_lf;
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* 太字下線フォント作成 */
	::DeleteObject( m_hFont_HAN_FAT_UL );
	lf = m_pShareData->m_Common.m_lf;
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT_UL = CreateFontIndirect( &lf );


	::DeleteObject( m_hFont_ZEN );
	lf = m_pShareData->m_Common.m_lf;
	lf.lfCharSet = SHIFTJIS_CHARSET;
	lf.lfOutPrecision = 1;
	strcpy( lf.lfFaceName, "ＭＳ ゴシック" );
	m_hFont_ZEN = CreateFontIndirect( &lf );

	/* フォントの変更 */
	SetFont();

	/* カーソル移動 */
	MoveCursor( m_nCaretPosX, m_nCaretPosY, TRUE );

	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	/* ウィンドウサイズの変更処理 */
	::GetClientRect( m_hWnd, &rc );
	OnSize( rc.right, rc.bottom );


	/* 再描画 */
	::InvalidateRect( m_hWnd, NULL, TRUE );

	return;
}




/* フォーカス移動時の再描画 */
void CEditView::RedrawAll( void )
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	/* 再描画 */
	hdc = ::GetDC( m_hWnd );

	OnKillFocus();

//	ps.rcPaint.left = 0;
//	ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
//	ps.rcPaint.top = 0;
//	ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
	::GetClientRect( m_hWnd, &ps.rcPaint );
//	ps.rcPaint.right -= ps.rcPaint.left;
//	ps.rcPaint.bottom -= ps.rcPaint.top;
//	ps.rcPaint.left = 0;
//	ps.rcPaint.top = 0;

	OnPaint( hdc, &ps, FALSE );	/* メモリＤＣを使用してちらつきのない再描画 */
	OnSetFocus();
	::ReleaseDC( m_hWnd, hdc );
	/* スクロールバーの状態を更新する */
	AdjustScrollBars();
	/* カーソル移動 */
	MoveCursor( m_nCaretPosX, m_nCaretPosY, TRUE );

	/* キャレットの行桁位置を表示する */
	DrawCaretPosInfo();

	/* 親ウィンドウのタイトルを更新 */
	SetParentCaption();

	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	return;
}



/* 自分の表示状態を他のビューにコピー */
void CEditView::CopyViewStatus( CEditView* pView )
{
	if( pView == NULL ){
		return;
	}
	if( pView == this ){
		return;
	}

	/* 入力状態 */
	pView->m_nCaretPosX 			= m_nCaretPosX;			/* ビュー左端からのカーソル桁位置（０開始）*/
	pView->m_nCaretPosX_Prev		= m_nCaretPosX_Prev;	/* ビュー左端からのカーソル桁位置（０オリジン）*/
	pView->m_nCaretPosY				= m_nCaretPosY;			/* ビュー上端からのカーソル行位置（０開始）*/
	pView->m_nCaretWidth			= m_nCaretWidth;		/* キャレットの幅 */
	pView->m_nCaretHeight			= m_nCaretHeight;		/* キャレットの高さ */

	/* キー状態 */
	pView->m_bSelectingLock			= m_bSelectingLock;		/* 選択状態のロック */
	pView->m_bBeginSelect			= m_bBeginSelect;		/* 範囲選択中 */
	pView->m_bBeginBoxSelect		= m_bBeginBoxSelect;	/* 矩形範囲選択中 */

//	pView->m_nSelectLineBgn			= m_nSelectLineBgn;		/* 範囲選択開始行(原点) */
//	pView->m_nSelectColmBgn			= m_nSelectColmBgn;		/* 範囲選択開始桁(原点) */
	pView->m_nSelectLineBgnFrom		= m_nSelectLineBgnFrom;	/* 範囲選択開始行(原点) */
	pView->m_nSelectColmBgnFrom		= m_nSelectColmBgnFrom;	/* 範囲選択開始桁(原点) */
	pView->m_nSelectLineBgnTo		= m_nSelectLineBgnTo;	/* 範囲選択開始行(原点) */
	pView->m_nSelectColmBgnTo		= m_nSelectColmBgnTo;	/* 範囲選択開始桁(原点) */

	pView->m_nSelectLineFrom		= m_nSelectLineFrom;	/* 範囲選択開始行 */
	pView->m_nSelectColmFrom		= m_nSelectColmFrom;	/* 範囲選択開始桁 */
	pView->m_nSelectLineTo			= m_nSelectLineTo;		/* 範囲選択終了行 */
	pView->m_nSelectColmTo			= m_nSelectColmTo;		/* 範囲選択終了桁 */
	pView->m_nSelectLineFromOld		= m_nSelectLineFromOld;	/* 範囲選択開始行 */
	pView->m_nSelectColmFromOld		= m_nSelectColmFromOld;	/* 範囲選択開始桁 */
	pView->m_nSelectLineToOld		= m_nSelectLineToOld;	/* 範囲選択終了行 */
	pView->m_nSelectColmToOld		= m_nSelectColmToOld;	/* 範囲選択終了桁 */
	pView->m_nMouseRollPosXOld		= m_nMouseRollPosXOld;	/* マウス範囲選択前回位置(X座標) */
	pView->m_nMouseRollPosYOld		= m_nMouseRollPosYOld;	/* マウス範囲選択前回位置(Y座標) */

	/* 画面情報 */
	pView->m_nViewAlignLeft			= m_nViewAlignLeft;		/* 表示域の左端座標 */
	pView->m_nViewAlignLeftCols		= m_nViewAlignLeftCols;	/* 行番号域の桁数 */
	pView->m_nViewAlignTop			= m_nViewAlignTop;		/* 表示域の上端座標 */
//	pView->m_nViewCx				= m_nViewCx;			/* 表示域の幅 */
//	pView->m_nViewCy				= m_nViewCy;			/* 表示域の高さ */
//	pView->m_nViewColNum			= m_nViewColNum;		/* 表示域の桁数 */
//	pView->m_nViewRowNum			= m_nViewRowNum;		/* 表示域の行数 */
	pView->m_nViewTopLine			= m_nViewTopLine;		/* 表示域の一番上の行(0開始) */
	pView->m_nViewLeftCol			= m_nViewLeftCol;		/* 表示域の一番左の桁(0開始) */

	/* 表示方法 */
	pView->m_nCharWidth				= m_nCharWidth;			/* 半角文字の幅 */
	pView->m_nCharHeight			= m_nCharHeight;		/* 文字の高さ */

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
			m_pcsbwVSplitBox->Create( m_hInstance, m_hWnd, TRUE );
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
			m_pcsbwHSplitBox->Create( m_hInstance, m_hWnd, FALSE );
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
			"SCROLLBAR",										/* scroll bar control class */
			(LPSTR) NULL,										/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,													/* horizontal position */
			0,													/* vertical position */
			200,												/* width of the scroll bar */
			CW_USEDEFAULT,										/* default height */
			m_hWnd,												/* handle of main window */
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
			"STATIC",												/* scroll bar control class */
			(LPSTR) NULL,											/* text for window title bar */
			WS_VISIBLE | WS_CHILD /*| SBS_SIZEBOX | SBS_SIZEGRIP*/, /* scroll bar styles */
			0,														/* horizontal position */
			0,														/* vertical position */
			200,													/* width of the scroll bar */
			CW_USEDEFAULT,											/* default height */
			m_hWnd,													/* handle of main window */
			(HMENU) NULL,											/* no menu for a scroll bar */
			m_hInstance,											/* instance owning this window */
			(LPVOID) NULL											/* pointer not needed */
		);
	}
	::ShowWindow( m_hwndSizeBox, SW_SHOW );

	::GetClientRect( m_hWnd, &rc );
	OnSize( rc.right, rc.bottom );

	return;
}





/* Grep実行 */
DWORD CEditView::DoGrep(
	CMemory*	pcmGrepKey,
	CMemory*	pcmGrepFile,
	CMemory*	pcmGrepFolder,
	BOOL		bGrepSubFolder,
	BOOL		bGrepLoHiCase,
	BOOL		bGrepRegularExp,
	BOOL		bKanjiCode_AutoDetect,
	BOOL		bGrepOutputLine,
	int			nGrepOutputStyle
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CEditView::DoGrep" );
#endif
	m_pcEditDoc->m_bGrepRunning = TRUE;


	int			nDummy;
	int			nHitCount = 0;
	char		szKey[_MAX_PATH];
	char		szFile[_MAX_PATH];
	char		szPath[_MAX_PATH];
//	int			nNewLine;		/* 挿入された部分の次の位置の行 */
//	int			nNewPos;		/* 挿入された部分の次の位置のデータ位置 */
//	COpe*		pcOpe = NULL;
	CDlgCancel	cDlgCancel;
	HWND		hwndCancel;
	char*		pszWork;
	HWND		hwndMainFrame;
	int			nCharChars;
	CJre		cJre;
	CMemory		cmemMessage;
	CMemory		cmemWork;
	int			nWork;
	int*		pnKey_CharCharsArr;
	pnKey_CharCharsArr = NULL;

	/*
	|| バッファサイズの調整
	*/
	cmemMessage.AllocBuffer( 64000 );



//	int*				pnKey_CharUsedArr;
//	pnKey_CharUsedArr		= NULL;
//	GrepParam*			pGrepParam;

//	CEditView*			pCEditView;
//	CMemory				cmGrepKey;
//	CMemory				cmGrepFile;
//	CMemory				cmGrepFolder;
//	BOOL				bGrepSubFolder;
//	BOOL				bGrepLoHiCase;
//	BOOL				bGrepRegularExp;
//	BOOL				bKanjiCode_AutoDetect;
//	BOOL				bGrepOutputLine;

//	pGrepParam				= (GrepParam*)dwGrepParam;

//	pCEditView				= (CEditView*)pGrepParam->pCEditView;
//	cmGrepKey				= *pGrepParam->pcmGrepKey;
//	cmGrepFile				= *pGrepParam->pcmGrepFile;
//	cmGrepFolder			= *pGrepParam->pcmGrepFolder;
//	bGrepSubFolder			= pGrepParam->bGrepSubFolder;
//	bGrepLoHiCase			= pGrepParam->bGrepLoHiCase;
//	bGrepRegularExp			= pGrepParam->bGrepRegularExp;
//	bKanjiCode_AutoDetect	= pGrepParam->bKanjiCode_AutoDetect;
//	bGrepOutputLine			= pGrepParam->bGrepOutputLine;

	m_bDoing_UndoRedo		= TRUE;


	/* アンドゥバッファの処理 */
	if( NULL != m_pcOpeBlk ){	/* 操作ブロック */
		while( NULL != m_pcOpeBlk ){}
//		delete m_pcOpeBlk;
//		m_pcOpeBlk = NULL;
	}
	m_pcOpeBlk = new COpeBlk;

	m_bCurSrchKeyMark = TRUE;								/* 検索文字列のマーク */
	strcpy( m_szCurSrchKey, pcmGrepKey->GetPtr( NULL ) );	/* 検索文字列 */
	m_bCurSrchRegularExp = bGrepRegularExp;					/* 検索／置換  1==正規表現 */
	m_bCurSrchLoHiCase = bGrepLoHiCase;						/* 検索／置換  1==英大文字小文字の区別 */
	/* 正規表現 */
	if( m_bCurSrchRegularExp ){
		/* CJreクラスの初期化 */
		m_CurSrch_CJre.Init();
		/* jre32.dllの存在チェック */
		if( FALSE == m_CurSrch_CJre.IsExist() ){
			::MessageBox( m_hWnd, "jre32.dllが見つかりません。\n正規表現を利用するにはjre32.dllが必要です。\n", "情報", MB_OK | MB_ICONEXCLAMATION );
			return 0;
		}

		/* 検索パターンのコンパイル */
		m_CurSrch_CJre.Compile( m_szCurSrchKey );
	}

//まだ m_bCurSrchWordOnly = m_pShareData->m_Common.m_bWordOnly;	/* 検索／置換  1==単語のみ検索 */
//	if( m_bCurSrchRegularExp ){
//		/* jre32.dllの存在チェック */
//		if( FALSE == m_CurSrch_CJre.IsExist() ){
//			m_bCurSrchKeyMark = FALSE;
//		}else{
//			/* 検索パターンのコンパイル */
//			if( !m_CurSrch_CJre.Compile( m_szCurSrchKey ) ){
//				m_bCurSrchKeyMark = FALSE;
//			}
//		}
//	}

//	cDlgCancel.Create( m_hInstance, m_hwndParent );
//	hwndCancel = cDlgCancel.Open( MAKEINTRESOURCE(IDD_GREPRUNNING) );
	hwndCancel = cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_GREPRUNNING );

	::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, 0, FALSE );

	pszWork = pcmGrepKey->GetPtr( NULL );
	strcpy( szKey, pszWork );

	strcpy( m_pcEditDoc->m_szGrepKey, szKey );
	m_pcEditDoc->m_bGrepMode = TRUE;

//	::SendMessage( ::GetParent( m_hwndParent ), WM_SETICON, ICON_BIG, (LPARAM)::LoadIcon( m_hInstance, IDI_QUESTION ) );
	HICON	hIcon;
//	hIcon = ::LoadIcon( NULL, IDI_QUESTION );
	hIcon = ::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_GREP ) );
	::SendMessage( ::GetParent( m_hwndParent ), WM_SETICON, ICON_SMALL,	(LPARAM)NULL );
	::SendMessage( ::GetParent( m_hwndParent ), WM_SETICON, ICON_SMALL,	(LPARAM)hIcon );
	::SendMessage( ::GetParent( m_hwndParent ), WM_SETICON, ICON_BIG,	(LPARAM)NULL );
	::SendMessage( ::GetParent( m_hwndParent ), WM_SETICON, ICON_BIG,	(LPARAM)hIcon );

	pszWork = pcmGrepFolder->GetPtr( NULL );
	strcpy( szPath, pszWork );
	nDummy = lstrlen( szPath );
	/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
	nCharChars = &szPath[nDummy] - CMemory::MemCharPrev( szPath, nDummy, &szPath[nDummy] );
	if( 1 == nCharChars && szPath[nDummy - 1] == '\\' ){
	}else{
		strcat( szPath, "\\" );
	}
	strcpy( szFile, pcmGrepFile->GetPtr( &nDummy ) );



	/* 最後にテキストを追加 */
	cmemMessage.AppendSz( "検索条件  " );
	if( 0 < lstrlen( szKey ) ){
		CMemory cmemWork2;
		cmemWork2.SetDataSz( szKey );
		if( m_pcEditDoc->GetDocumentAttribute().m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
			cmemWork2.Replace( "\\", "\\\\" );
			cmemWork2.Replace( "\'", "\\\'" );
			cmemWork2.Replace( "\"", "\\\"" );
		}else{
			cmemWork2.Replace( "\'", "\'\'" );
			cmemWork2.Replace( "\"", "\"\"" );
		}
		cmemWork.AppendSz( "\"" );
		cmemWork.Append( &cmemWork2 );
		cmemWork.AppendSz( "\"\r\n" );
	}else{
		cmemWork.AppendSz( "「ファイル検索」\r\n" );
	}
	cmemMessage += cmemWork;



	cmemMessage.AppendSz( "検索対象   " );
	cmemWork.SetDataSz( szFile );
	if( m_pcEditDoc->GetDocumentAttribute().m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemMessage += cmemWork;




	cmemMessage.AppendSz( "\r\n" );
	cmemMessage.AppendSz( "フォルダ   " );
	cmemWork.SetDataSz( szPath );
	if( m_pcEditDoc->GetDocumentAttribute().m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemMessage += cmemWork;
	cmemMessage.AppendSz( "\r\n" );

	if( bGrepSubFolder ){
		pszWork = "    (サブフォルダも検索)\r\n";
	}else{
		pszWork = "    (サブフォルダを検索しない)\r\n";
	}
	cmemMessage.AppendSz( pszWork );

	if( bGrepLoHiCase ){
		pszWork = "    (英大文字小文字を区別する)\r\n";
	}else{
		pszWork = "    (英大文字小文字を区別しない)\r\n";
	}
	cmemMessage.AppendSz( pszWork );

	if( bGrepRegularExp ){
		cmemMessage.AppendSz( "    (正規表現)\r\n" );
	}

	if( bKanjiCode_AutoDetect ){
		cmemMessage.AppendSz( "    (文字コードセットの自動判別)\r\n" );
	}

	if( bGrepOutputLine ){
	/* 該当行 */
		pszWork = "    (一致した行を出力)\r\n";
	}else{
		pszWork = "    (一致した箇所のみ出力)\r\n";
	}
	cmemMessage.AppendSz( pszWork );



	pszWork = "\r\n\r\n";
	cmemMessage.AppendSz( "\r\n\r\n" );
	pszWork = cmemMessage.GetPtr( &nWork );
	if( 0 < nWork ){
		Command_ADDTAIL( pszWork, nWork );
	}
	if( bGrepRegularExp ){
		/* CJreクラスの初期化 */
		cJre.Init();
		/* jre32.dllの存在チェック */
		if( FALSE == cJre.IsExist() ){
			::MessageBox( 0, "jre32.dllが見つかりません。\n正規表現を利用するにはjre32.dllが必要です。\n", "情報", MB_OK | MB_ICONEXCLAMATION );
			return 0;
		}
		/* 検索パターンのコンパイル */
		if( !cJre.Compile( szKey ) ){
			return 0;
		}
	}else{
		/* 検索条件の情報 */
		CDocLineMgr::CreateCharCharsArr(
			(const unsigned char *)szKey,
			lstrlen( szKey ),
			&pnKey_CharCharsArr
		);
//		/* 検索条件の情報(キー文字列の使用文字表)作成 */
//		CDocLineMgr::CreateCharUsedArr(
//			(const unsigned char *)szKey,
//			lstrlen( szKey ),
//			pnKey_CharCharsArr,
//			&pnKey_CharUsedArr
//		);

	}

	/* 表示処理ON/OFF */
	m_bDrawSWITCH = FALSE;



	if( -1 == DoGrepTree(
		&cDlgCancel, hwndCancel, szKey,
		pnKey_CharCharsArr,
//		pnKey_CharUsedArr,
		szFile, szPath, bGrepSubFolder, bGrepLoHiCase,
		bGrepRegularExp, bKanjiCode_AutoDetect,
		bGrepOutputLine, nGrepOutputStyle, &cJre, 0, &nHitCount
	) ){
		wsprintf( szPath, "中断しました。\r\n", nHitCount );
		Command_ADDTAIL( szPath, lstrlen( szPath ) );
	}
	wsprintf( szPath, "%d 個が検索されました。\r\n", nHitCount );
	Command_ADDTAIL( szPath, lstrlen( szPath ) );
	Command_GOFILEEND( FALSE );

#ifdef _DEBUG
	wsprintf( szPath, "処理時間: %dミリ秒\r\n", cRunningTimer.Read() );
	Command_ADDTAIL( szPath, lstrlen( szPath ) );
	Command_GOFILEEND( FALSE );
#endif

	cDlgCancel.CloseDialog( 0 );

	/* アクティブにする */
	hwndMainFrame = ::GetParent( m_hwndParent );
	/* アクティブにする */
	ActivateFrameWindow( hwndMainFrame );


	/* アンドゥバッファの処理 */
	if( NULL != m_pcOpeBlk ){
		if( 0 < m_pcOpeBlk->GetNum() ){	/* 操作の数を返す */
			/* 操作の追加 */
			m_pcEditDoc->m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
		}else{
			delete m_pcOpeBlk;
		}
		m_pcOpeBlk = NULL;
	}

	//	Apr. 13, 2001 genta
	//	Grep実行後はファイルを変更無しの状態にする．
	m_pcEditDoc->m_bIsModified = FALSE;
	
	m_pcEditDoc->m_bGrepRunning = FALSE;
	m_bDoing_UndoRedo = FALSE;

	if( NULL != pnKey_CharCharsArr ){
		delete [] pnKey_CharCharsArr;
		pnKey_CharCharsArr = NULL;
	}
//	if( NULL != pnKey_CharUsedArr ){
//		delete [] pnKey_CharUsedArr;
//		pnKey_CharUsedArr = NULL;
//	}

	/* 表示処理ON/OFF */
	m_bDrawSWITCH = TRUE;

	/* フォーカス移動時の再描画 */
	RedrawAll();

	return nHitCount;
}



/* Grep実行 */
int CEditView::DoGrepTree(
	CDlgCancel* pcDlgCancel,
	HWND		hwndCancel,
	const char*	pszKey,
	int*		pnKey_CharCharsArr,
//	int*		pnKey_CharUsedArr,
	const char*	pszFile,
	const char*	pszPath,
	BOOL		bGrepSubFolder,
	BOOL		bGrepLoHiCase,
	BOOL		bGrepRegularExp,
	BOOL		bKanjiCode_AutoDetect,
	BOOL		bGrepOutputLine,
	int			nGrepOutputStyle,
	CJre*		pCJre,
	int			nNest,
	int*		pnHitCount
)
{
	int				nPos;
	char			szFile[_MAX_PATH];
	char			szPath[_MAX_PATH];
	char			szPath2[_MAX_PATH];
//	char			szTab[64];
	int				nFileLen;
	char*			pszToken;
	HANDLE			hFind;
//	int				i;
	int				nRet;
	CMemory			cmemMessage;
	int				nHitCountOld;
	char*			pszWork;
	int				nWork;
	WIN32_FIND_DATA	w32fd;
	nHitCountOld = -100;
//	MSG msg;
//	BOOL ret;

	::SetDlgItemText( hwndCancel, IDC_STATIC_CURPATH, pszPath );

	strcpy( szFile, pszFile );
	nFileLen = lstrlen( szFile );
	if( TRUE == bGrepSubFolder ){
		strcpy( szPath, pszPath );
		strcat( szPath, "*.*" );
		hFind = ::FindFirstFile( szPath, &w32fd );
		if( INVALID_HANDLE_VALUE == hFind ){
		}else{
			do{
				/* 処理中のユーザー操作を可能にする */
				if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
					goto cancel_return;
				}
				/* 中断ボタン押下チェック */
				if( pcDlgCancel->IsCanceled() ){
					goto cancel_return;
				}
				if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
					0 != strcmp( w32fd.cFileName, "." ) &&
					0 != strcmp( w32fd.cFileName, ".." )
				){
//					szTab[0] = '\0';
//					for( i= 0; i < nNest; ++i ){
//						strcat( szTab, "\t" );
//					}
					strcpy( szPath2, pszPath );
					strcat( szPath2, w32fd.cFileName );
					strcat( szPath2, "\\" );
					if( -1 == DoGrepTree(
						pcDlgCancel, hwndCancel,
						pszKey,
						pnKey_CharCharsArr,
//						pnKey_CharUsedArr,
						pszFile, szPath2,
						bGrepSubFolder, bGrepLoHiCase,
						bGrepRegularExp, bKanjiCode_AutoDetect,
						bGrepOutputLine, nGrepOutputStyle, pCJre, nNest + 1, pnHitCount
					) ){
						goto cancel_return;
					}
				}
			}while( TRUE == ::FindNextFile( hFind, &w32fd ) );
			::FindClose( hFind );
		}
	}
	nPos = 0;
	pszToken = my_strtok( szFile, nFileLen, &nPos, " ;," );
	while( NULL != pszToken ){
		strcpy( szPath, pszPath );
		strcat( szPath, pszToken );
		hFind = ::FindFirstFile( szPath, &w32fd );
		if( INVALID_HANDLE_VALUE == hFind ){
			goto last_of_this_loop;
		}
		do{
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
				goto cancel_return;
			}
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				goto cancel_return;
			}
			if( (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ){
			}else{
//				::SetDlgItemText( hwndCancel, IDC_STATIC_CURPATH, szPath2 );
				::SetDlgItemText( hwndCancel, IDC_STATIC_CURFILE, w32fd.cFileName );

				wsprintf( szPath2, "%s%s", pszPath, w32fd.cFileName );
				/* ファイル内の検索 */
				nRet = DoGrepFile(
					pcDlgCancel, hwndCancel, pszKey,
					pnKey_CharCharsArr,
//					pnKey_CharUsedArr,
					pszFile, szPath2,
					bGrepSubFolder, bGrepLoHiCase,
					bGrepRegularExp, bKanjiCode_AutoDetect,
					bGrepOutputLine, nGrepOutputStyle,
					pCJre, nNest, pnHitCount, szPath2, cmemMessage
				);
//				::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
				if( *pnHitCount - nHitCountOld  >= 10 ){
					/* 結果出力 */
					pszWork = cmemMessage.GetPtr( &nWork );
					if( 0 < nWork ){
						Command_ADDTAIL( pszWork, nWork );
						Command_GOFILEEND( FALSE );
						/* 結果格納エリアをクリア */
						cmemMessage.SetDataSz( "" );
					}
					nHitCountOld = *pnHitCount;
				}
				if( -1 == nRet ){
					goto cancel_return;
				}
			}
		}while( TRUE == ::FindNextFile( hFind, &w32fd ) );
		::FindClose( hFind );
last_of_this_loop:;
		pszToken = my_strtok( szFile, nFileLen, &nPos, " ;," );
	}
	/* 結果出力 */
	pszWork = cmemMessage.GetPtr( &nWork );
	if( 0 < nWork ){
		Command_ADDTAIL( pszWork, nWork );
		Command_GOFILEEND( FALSE );
		/* 結果格納エリアをクリア */
		cmemMessage.SetDataSz( "" );
	}
	return 0;
cancel_return:;
	/* 結果出力 */
	pszWork = cmemMessage.GetPtr( &nWork );
	if( 0 < nWork ){
		Command_ADDTAIL( pszWork, nWork );
		Command_GOFILEEND( FALSE );
		/* 結果格納エリアをクリア */
		cmemMessage.SetDataSz( "" );
	}
	return -1;

}







/* Grep実行 */
int CEditView::DoGrepFile(
	CDlgCancel* pcDlgCancel,
	HWND		hwndCancel,
	const char*	pszKey,
	int*		pnKey_CharCharsArr,
//	int*		pnKey_CharUsedArr,
	const char*	pszFile,
	const char*	pszPath,
	BOOL		bGrepSubFolder,
	BOOL		bGrepLoHiCase,
	BOOL		bGrepRegularExp,
	BOOL		bKanjiCode_AutoDetect,
	BOOL		bGrepOutputLine,
	int			nGrepOutputStyle,
	CJre*		pCJre,
	int			nNest,
	int*		pnHitCount,
	const char*	pszFullPath,
	CMemory&	cmemMessage
)
{
	int		nHitCount;
	char	szLine[16000];
	char	szWork[16500];
	char	szWork0[_MAX_PATH + 100];
	HFILE	hFile;
	int		nLine;
//	CJre	cJre;
	char*	pszRes;
	CMemory	cmemBuf;
//	CMemory	cmemLine;
	int		nCharCode;
	char*	pCompareData;
	char*	pszCodeName;
	int		nFileLength;
	int		nReadLength;
	int		nReadSize;
	HGLOBAL	hgRead = NULL;;
	char*	pBuf;
	int		nEOF;
	int		nBgn;
	int		nPos;
	int		nColm;
	BOOL	bOutFileName;
	bOutFileName = FALSE;
//	CMemory	cmemMessage;
//	char*	pszWork;
//	int		nWork;
	int		nLineLen;
	const	char*	pLine;
//	int		nLineLen;
//	enumEOLType nEOLType;
	CEOL	cEol;
	int		nEolCodeLen;
	int		k;
//	int		nLineNum;






//	if( bGrepRegularExp ){
//		/* CJreクラスの初期化 */
//		cJre.Init();
//
//		/* jre32.dllの存在チェック */
//		if( FALSE == cJre.IsExist() ){
//			::MessageBox( 0, "jre32.dllが見つかりません。\n正規表現を利用するにはjre32.dllが必要です。\n", "情報", MB_OK | MB_ICONEXCLAMATION );
//			return -1;
//		}
//		/* 検索パターンのコンパイル */
//		if( !cJre.Compile( pszKey ) ){
//			return -1;
//		}
//	}

	nCharCode = 0;
	pszCodeName = "";
	if( bKanjiCode_AutoDetect ){
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
		nCharCode = CMemory::CheckKanjiCodeOfFile( pszFullPath );
		if( -1 == nCharCode ){
			wsprintf( szLine, "文字コードの判別処理でエラー [%s]\r\n", pszFullPath );
			Command_ADDTAIL( szLine, lstrlen( szLine ) );
			return 0;
		}
		pszCodeName = (char*)gm_pszCodeNameArr_3[nCharCode];
//		switch( nCharCode ){
//		case CODE_SJIS:	/* SJIS */
//			pszCodeName = "[SJIS]";
//			break;
//		case CODE_JIS:	/* JIS */
//			pszCodeName = "[JIS]";
//			break;
//		case CODE_EUC:	/* EUC */
//			pszCodeName = "[EUC]";
//			break;
//		case CODE_UNICODE:	/* Unicode */
//			pszCodeName = "[Unicode]";
//			break;
//		case CODE_UTF8:	/* UTF-8 */
//			pszCodeName = "[UTF-8]";
//			break;
//		case CODE_UTF7:	/* UTF-7 */
//			pszCodeName = "[UTF-7]";
//			break;
//		default:
//			wsprintf( szLine, "文字コードの判別がなんかヘンな結果。バグ。[%s]\r\n", pszFullPath );
//			Command_ADDTAIL( szLine, lstrlen( szLine ) );
//			return 0;
//		}
	}
	nHitCount = 0;
	nLine = 0;
	hFile = _lopen( pszPath, OF_READ );
	if( HFILE_ERROR == hFile ){
		wsprintf( szLine, "file open error [%s]\r\n", pszFullPath );
		Command_ADDTAIL( szLine, lstrlen( szLine ) );
		return 0;
	}

	/* ファイルサイズの取得 */
	nFileLength = _llseek( hFile, 0, FILE_END );
	_llseek( hFile, 0, FILE_BEGIN );

	hgRead = ::GlobalAlloc( GHND, nFileLength + 1 );
	if( NULL == hgRead ){
		wsprintf( szLine, "CEditView::DoGrepFile()\n[%s] メモリ確保に失敗しました。\n%dバイト \r\n", pszFullPath, nFileLength );
		Command_ADDTAIL( szLine, lstrlen( szLine ) );
		_lclose( hFile );
		return 0;
	}
	pBuf = (char*)::GlobalLock( hgRead );
	nEOF = TRUE;
	if( nCharCode == 3 ){ /* Unicode */
		nReadSize = _lread( hFile, pBuf, 2);
	}
	nReadLength = 0;
	nBgn = 0;
	nPos = 0;
	nReadSize = _lread( hFile, pBuf, nFileLength/*nReadBufSize*/);
	pBuf[nFileLength] = '\0';

	if( HFILE_ERROR == nReadSize ){
		wsprintf( szLine, "file read error %s\r\n", pszFullPath );
		Command_ADDTAIL( szLine, lstrlen( szLine ) );
		_lclose( hFile );
		return 0;
	}
	/* 処理中のユーザー操作を可能にする */
	if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
		return -1;
	}
	/* 中断ボタン押下チェック */
	if( pcDlgCancel->IsCanceled() ){
		return -1;
	}

	switch( nCharCode ){
	case CODE_EUC /* EUC */:
		cmemBuf.SetData( pBuf, nReadSize );
		/* EUC→SJISコード変換 */
		cmemBuf.EUCToSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_JIS /* JIS */:
		cmemBuf.SetData( pBuf, nReadSize );
		/* E-Mail(JIS→SJIS)コード変換 */
		cmemBuf.JIStoSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_UNICODE /* Unicode */:
		cmemBuf.SetData( pBuf, nReadSize );
		/* Unicode→SJISコード変換 */
		cmemBuf.UnicodeToSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_UTF8 /* UTF-8 */:
		cmemBuf.SetData( pBuf, nReadSize );
		/* UTF-8→SJISコード変換 */
		cmemBuf.UTF8ToSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_UTF7 /* UTF-7 */:
		cmemBuf.SetData( pBuf, nReadSize );
		/* UTF-7→SJISコード変換 */
		cmemBuf.UTF7ToSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_SJIS /* SJIS */:
	default:
		break;
	}


	int	nKeyKen = lstrlen( pszKey );

	/* 検索条件が長さゼロの場合はファイル名だけ返す */
	if( 0 == nKeyKen ){
		if( 1 == nGrepOutputStyle ){
		/* ノーマル */
			wsprintf( szWork0, "%s %s\r\n", pszFullPath, pszCodeName );
		}else{
		/* WZ風 */
			wsprintf( szWork0, "■\"%s\" %s\r\n", pszFullPath, pszCodeName );
		}
		cmemMessage.AppendSz( szWork0 );
		++(*pnHitCount);
		::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
		return 1;
	}
	wsprintf( szWork0, "■\"%s\" %s\r\n", pszFullPath, pszCodeName );


	nBgn = 0;
//	nPos = 0;
	nLine = 0;
	while( NULL != ( pLine = GetNextLine( pBuf, nReadSize, &nLineLen, &nBgn, &cEol ) ) ){
		nEolCodeLen = cEol.GetLen();
		++nLine;
//		AddLineStrX( pLine, nLineLen + nEolCodeLen, nEOLType );
		nReadLength += nLineLen + nEolCodeLen;
		nLineLen += nEolCodeLen;
		pCompareData = (char*)pLine;

		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
			return -1;
		}
		if( 0 == nLine % 64 ){
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				return -1;
			}
		}

		if( bGrepRegularExp ){
			if( NULL != ( pszRes = (char *)pCJre->GetMatchInfo( pCompareData, nLineLen, 0 ) )
			){
				nColm = pszRes - pCompareData + 1;


//				if( nLineLen > sizeof( szLine ) - 10 ){
//					nLineLen = sizeof( szLine ) - 10;
//				}
//				memcpy( szLine, pCompareData, nLineLen - 1 );
//				szLine[nLineLen - 1] = '\0';

				if( bGrepOutputLine ){
				/* 該当行 */
					k = nLineLen - nEolCodeLen;
					if( k > 1000 ){
						k = 1000;
					}
					pCompareData[k] = '\0';

					if( 1 == nGrepOutputStyle ){
					/* ノーマル */
						wsprintf( szWork, "%s(%d,%d)%s: %s\r\n", pszFullPath, nLine, nColm, pszCodeName, pCompareData );
					}else
					if( 2 == nGrepOutputStyle ){
					/* WZ風 */
						wsprintf( szWork, "・(%6d,%-5d): %s\r\n", nLine, nColm, pCompareData );
					}
				}else{
				/* 該当部分 */
					char* pszHit;
					pszHit = new char[pCJre->m_jreData.nLength + 1];
					memcpy( pszHit, pszRes, pCJre->m_jreData.nLength );
					pszHit[pCJre->m_jreData.nLength] = '\0';
					if( 1 == nGrepOutputStyle ){
					/* ノーマル */
						wsprintf( szWork, "%s(%d,%d)%s: %s\r\n", pszFullPath, nLine, nColm, pszCodeName, pszHit );
					}else
					if( 2 == nGrepOutputStyle ){
					/* WZ風 */
						wsprintf( szWork, "・(%6d,%-5d): %s\r\n", nLine, nColm, pszHit );
					}
					delete [] pszHit;
				}
				if( 2 == nGrepOutputStyle ){
				/* WZ風 */
					if( !bOutFileName ){
						cmemMessage.AppendSz( szWork0 );
						bOutFileName = TRUE;
					}
				}
				cmemMessage.AppendSz( szWork );
				++nHitCount;
				++(*pnHitCount);
				if( 0 == ( (*pnHitCount) % 16 ) ){
					::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
				}
			}
		}else{
			/* 文字列検索 */
			if( NULL != ( pszRes = (char *)CDocLineMgr::SearchString(
				(const unsigned char *)pCompareData, nLineLen,
				0,
				(const unsigned char *)pszKey, nKeyKen,
				pnKey_CharCharsArr,
//				pnKey_CharUsedArr,
				bGrepLoHiCase
			) ) ){
				nColm = pszRes - pCompareData + 1;

//				if( nLineLen > sizeof( szLine ) - 10 ){
//					nLineLen = sizeof( szLine ) - 10;
//				}
//				memcpy( szLine, pCompareData, nLineLen - 1 );
//				szLine[nLineLen - 1] = '\0';
//				memcpy( szLine, pCompareData, nLineLen - nEolCodeLen );
//				szLine[nLineLen - nEolCodeLen] = '\0';
//				pCompareData = szLine;

				if( bGrepOutputLine ){
				/* 該当行 */
					k = nLineLen - nEolCodeLen;
					if( k > 1000 ){
						k = 1000;
					}
					pCompareData[k] = '\0';
					if( 1 == nGrepOutputStyle ){
					/* ノーマル */
						wsprintf( szWork, "%s(%d,%d)%s: %s\r\n", pszFullPath, nLine, nColm, pszCodeName, pCompareData );
					}else
					if( 2 == nGrepOutputStyle ){
					/* WZ風 */
						wsprintf( szWork, "・(%6d,%-5d): %s\r\n", nLine, nColm, pCompareData );
					}
				}else{
				/* 該当部分 */
					char* pszHit;
					pszHit = new char[nKeyKen + 1];
					memcpy( pszHit, pszRes, nKeyKen );
					pszHit[nKeyKen] = '\0';
					if( 1 == nGrepOutputStyle ){
					/* ノーマル */
						wsprintf( szWork, "%s(%d,%d)%s: %s\r\n", pszFullPath, nLine, nColm, pszCodeName, pszHit );
					}else
					if( 2 == nGrepOutputStyle ){
					/* WZ風 */
						wsprintf( szWork, "・(%6d,%-5d): %s\r\n", nLine, nColm, pszHit );
					}
					delete [] pszHit;
				}
				if( 2 == nGrepOutputStyle ){
				/* WZ風 */
					if( !bOutFileName ){
						cmemMessage.AppendSz( szWork0 );
						bOutFileName = TRUE;
					}
				}
				cmemMessage.AppendSz( szWork );
				++nHitCount;
				++(*pnHitCount);
				//	May 22, 2000 genta
				// if( 0 == ( (*pnHitCount) % 16 ) ){
					::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
				// }
			}
		}
	}

	if( NULL != hgRead ){
		::GlobalUnlock( hgRead );
		::GlobalFree( hgRead );
	}
	_lclose( hFile );
	return nHitCount;
}


/*
	カーソル直前の単語を取得 単語の長さを返します
	単語区切り
*/
int CEditView::GetLeftWord( CMemory* pcmemWord, int nMaxWordLen )
{
	const char*	pLine;
	int			nLineLen;
	int			nIdx;
	int			nIdxTo;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	CMemory		cmemWord;
	int			nCurLine;
	int			nCharChars;

	nCurLine = m_nCaretPosY;
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCurLine, &nLineLen );
	if( NULL == pLine ){
//		return 0;
	}else{
		/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */
		nIdxTo = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );
	}
	if( 0 == nIdxTo || NULL == pLine ){
		if( nCurLine <= 0 ){
			return 0;
		}
		nCurLine--;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCurLine, &nLineLen );
		if( NULL == pLine ){
			return 0;
		}
		if( pLine[nLineLen - 1] == '\r' || pLine[nLineLen - 1] == '\n' ){
			return 0;
		}
		/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */
//		nIdxTo = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );

		nCharChars = &pLine[nLineLen] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nLineLen] );
		if( 0 == nCharChars ){
			return 0;
		}
		nIdxTo = nLineLen;
		nIdx = nIdxTo - nCharChars;

//		nIdx = nIdxTo = nLineLen - 1;
	}else{
		nCharChars = &pLine[nIdxTo] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nIdxTo] );
		if( 0 == nCharChars ){
			return 0;
		}
		nIdx = nIdxTo - nCharChars;
	}
	if( 1 == nCharChars ){
		if( pLine[nIdx] == SPACE || pLine[nIdx] == TAB ){
			return 0;
		}
	}
	if( 2 == nCharChars ){
		if( (unsigned char)pLine[nIdx	 ] == (unsigned char)0x81 &&
			(unsigned char)pLine[nIdx + 1] == (unsigned char)0x40
		){
			return 0;
		}
	}


//	nIdx = nIdxTo - 1;
//	if( 0 > nIdx ){
//		return 0;
//	}
//	if( pLine[nIdx] == SPACE ||
//		pLine[nIdx] == TAB ||
//		(
//			nIdx > 0 &&
//			pLine[nIdx] == 0x40 &&
//			(unsigned char)pLine[nIdx - 1] == (unsigned char)0x81
//		)
//	){
//		return 0;
//	}
//	while( 0 <= nIdx && ( pLine[nIdx] != SPACE && pLine[nIdx] != TAB ) ){
//		nIdx--;
//	}
//	++nIdx;
//	if( nIdxTo - nIdx > 0 ){
//		if( nMaxWordLen < nIdxTo - nIdx ){
//			return 0;
//		}else{
//			pcmemWord->SetData( &pLine[nIdx], nIdxTo - nIdx );
//			return nIdxTo - nIdx;
//		}
//	}else{
//		return 0;
//	}



	/* 現在位置の単語の範囲を調べる */
	if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
		nCurLine, nIdx,
		&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, &cmemWord, pcmemWord )
	){
		pcmemWord->Append( &pLine[nIdx], nCharChars );
//		MYTRACE( "==========\n" );
//		MYTRACE( "cmemWord=[%s]\n", cmemWord.GetPtr( NULL ) );
//		MYTRACE( "pcmemWord=[%s]\n", pcmemWord->GetPtr( NULL ) );

		return pcmemWord->GetLength();
	}else{
		return 0;
	}
}

/* 指定カーソル位置が選択エリア内にあるか
	【戻り値】
	-1	選択エリアより前方 or 無選択
	0	選択エリア内
	1	選択エリアより後方
*/
int CEditView::IsCurrentPositionSelected(
	int		nCaretPosX,		// カーソル位置X
	int		nCaretPosY		// カーソル位置Y
)
{
	if( FALSE == IsTextSelected() ){	/* テキストが選択されているか */
		return -1;
	}
	RECT	rcSel;
	POINT	po;


	/* 矩形範囲選択中か */
	if( m_bBeginBoxSelect ){
		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,		/* 範囲選択開始行 */
			m_nSelectColmFrom,		/* 範囲選択開始桁 */
			m_nSelectLineTo,		/* 範囲選択終了行 */
			m_nSelectColmTo			/* 範囲選択終了桁 */
		);
		++rcSel.bottom;
		po.x = nCaretPosX;
		po.y = nCaretPosY;
		if( m_bDragSource ){
			if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){ /* Ctrlキーが押されていたか */
				++rcSel.left;
			}else{
				++rcSel.right;
			}
		}
		if( PtInRect( &rcSel, po ) ){
			return 0;
		}
		if( rcSel.top > nCaretPosY ){
			return -1;
		}
		if( rcSel.bottom < nCaretPosY ){
			return 1;
		}
		if( rcSel.left > nCaretPosX ){
			return -1;
		}
		if( rcSel.right < nCaretPosX ){
			return 1;
		}
	}else{
		if( m_nSelectLineFrom > nCaretPosY ){
			return -1;
		}
		if( m_nSelectLineTo < nCaretPosY ){
			return 1;
		}
		if( m_nSelectLineFrom == nCaretPosY ){
			if( m_bDragSource ){
				if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){	/* Ctrlキーが押されていたか */
					if( m_nSelectColmFrom >= nCaretPosX ){
						return -1;
					}
				}else{
					if( m_nSelectColmFrom > nCaretPosX ){
						return -1;
					}
				}
			}else
			if( m_nSelectColmFrom > nCaretPosX ){
				return -1;
			}
		}
		if( m_nSelectLineTo == nCaretPosY ){
			if( m_bDragSource ){
				if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){	/* Ctrlキーが押されていたか */
					if( m_nSelectColmTo <= nCaretPosX ){
						return 1;
					}
				}else{
					if( m_nSelectColmTo < nCaretPosX ){
						return 1;
					}
				}
			}else
			if( m_nSelectColmTo <= nCaretPosX ){
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
int CEditView::IsCurrentPositionSelectedTEST(
	int		nCaretPosX,		// カーソル位置X
	int		nCaretPosY,		// カーソル位置Y
	int		nSelectLineFrom,
	int		nSelectColmFrom,
	int		nSelectLineTo,
	int		nSelectColmTo
)
{
	if( FALSE == IsTextSelected() ){	/* テキストが選択されているか */
		return -1;
	}
//	RECT	rcSel;
//	POINT	po;


	if( nSelectLineFrom > nCaretPosY ){
		return -1;
	}
	if( nSelectLineTo < nCaretPosY ){
		return 1;
	}
	if( nSelectLineFrom == nCaretPosY ){
		if( nSelectColmFrom > nCaretPosX ){
			return -1;
		}
	}
	if( nSelectLineTo == nCaretPosY ){
		if( nSelectColmTo <= nCaretPosX ){
			return 1;
		}
	}
	return 0;
}

/* クリップボードからデータを取得 */
BOOL CEditView::MyGetClipboardData( CMemory& cmemBuf, BOOL* pbColmnSelect )
{
	HGLOBAL		hglb;
	char*		lptstr;

	if( NULL != pbColmnSelect ){
		*pbColmnSelect = FALSE;
	}


	UINT uFormatSakuraClip;
	UINT uFormat;
	uFormatSakuraClip = ::RegisterClipboardFormat( "SAKURAClip" );
	if( !::IsClipboardFormatAvailable( CF_OEMTEXT )
	 && !::IsClipboardFormatAvailable( uFormatSakuraClip )
	){
		return FALSE;
	}
	if ( !::OpenClipboard( m_hWnd ) ){
		return FALSE;
	}

	char	szFormatName[128];

	if( NULL != pbColmnSelect ){
		/* 矩形選択のテキストデータがクリップボードにあるか */
		uFormat = 0;
		while( 0 != ( uFormat = ::EnumClipboardFormats( uFormat ) ) ){
			::GetClipboardFormatName( uFormat, szFormatName, sizeof(szFormatName) - 1 );
//			MYTRACE( "szFormatName=[%s]\n", szFormatName );
			if( 0 == lstrcmp( "MSDEVColumnSelect", szFormatName ) ){
				*pbColmnSelect = TRUE;
				break;
			}
		}
	}
	if( ::IsClipboardFormatAvailable( uFormatSakuraClip ) ){
		hglb = ::GetClipboardData( uFormatSakuraClip );
		if (hglb != NULL) {
			lptstr = (char*)::GlobalLock(hglb);
			cmemBuf.SetData( lptstr + sizeof(int), *((int*)lptstr) );
			::GlobalUnlock(hglb);
			::CloseClipboard();
			return TRUE;
		}
	}else{
		hglb = ::GetClipboardData( CF_OEMTEXT );
		if( hglb != NULL ){
			lptstr = (char*)::GlobalLock(hglb);
			cmemBuf.SetDataSz( lptstr );
			::GlobalUnlock(hglb);
			::CloseClipboard();
			return TRUE;
		}
	}
	::CloseClipboard();
	return FALSE;
}

/* クリップボードにデータを設定 */
BOOL CEditView::MySetClipboardData( const char* pszText, int nTextLen, BOOL bColmnSelect )
{
	HGLOBAL		hgClip;
	char*		pszClip;
	UINT		uFormat;
	/* Windowsクリップボードにコピー */
	if( FALSE == ::OpenClipboard( m_hWnd ) ){
		return FALSE;
	}
	::EmptyClipboard();

	/* テキスト形式のデータ */
	hgClip = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		lstrlen( pszText ) + 1
	);
	pszClip = (char*)::GlobalLock( hgClip );
	lstrcpy( pszClip, pszText );
	::GlobalUnlock( hgClip );
	::SetClipboardData( CF_OEMTEXT, hgClip );

	/* バイナリ形式のデータ
		(int) 「データ」の長さ
		「データ」
	*/
	UINT	uFormatSakuraClip;
	uFormatSakuraClip = ::RegisterClipboardFormat( "SAKURAClip" );
	if( 0 != uFormatSakuraClip ){
		hgClip = ::GlobalAlloc(
			GMEM_MOVEABLE | GMEM_DDESHARE,
			nTextLen + sizeof( int ) + 1
		);
		pszClip = (char*)::GlobalLock( hgClip );
		*((int*)pszClip) = nTextLen;
		memcpy( pszClip + sizeof( int ), pszText, nTextLen );
		::GlobalUnlock( hgClip );
		::SetClipboardData( uFormatSakuraClip, hgClip );
	}


	/* 矩形選択を示すダミーデータ */
	if( bColmnSelect ){
		uFormat = ::RegisterClipboardFormat( "MSDEVColumnSelect" );
		if( 0 != uFormat ){
			hgClip = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			pszClip = (char*)::GlobalLock( hgClip );
			pszClip[0] = '\0';
			::GlobalUnlock( hgClip );
			::SetClipboardData( uFormat, hgClip );
		}
	}
	::CloseClipboard();
	return TRUE;
}






STDMETHODIMP CEditView::DragEnter( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE( "CEditView::DragEnter()\n" );
#endif

	if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop ){	/* OLEによるドラッグ & ドロップを使う */
	}else{
		return E_INVALIDARG;
	}

	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;
	if( IsDataAvailable( pDataObject, CF_TEXT )
//	 && NULL != GetGlobalData(pDataObject, CF_TEXT)
	){
		/* 自分をアクティブペインにする */
		m_pcEditDoc->SetActivePane( m_nMyIndex );

		/* 選択テキストのドラッグ中か */
		m_bDragMode = TRUE;

	//	/* 編集ウィンドウオブジェクトからのアクティブ要求 */
	//	::PostMessage( m_pShareData->m_hwndTray, MYWM_ACTIVATE_ME, (WPARAM)::GetParent( m_hwndParent ),  0 );

		/* 入力フォーカスを受け取ったときの処理 */
		OnSetFocus();

	//	::ScreenToClient( m_hWnd_DropTarget, (LPPOINT)&pt );
	//	OnLBUTTONDOWN( dwKeyState, pt.x, pt.y );

		m_pcDropTarget->m_pDataObject = pDataObject;
		/* Ctrl,ALT,キーが押されていたか */
		if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
			*pdwEffect = DROPEFFECT_COPY;
		}else{
			*pdwEffect = DROPEFFECT_MOVE;
		}
//		/* アクティブにする */
//		ActivateFrameWindow( GetParent( m_hwndParent ) );
//		ActivateFrameWindow( m_hWnd_DropTarget/*GetParent( m_hwndParent )*/ );

//		::SetFocus( m_hWnd_DropTarget );
		::SetFocus( m_hWnd );
	}else{
		return E_INVALIDARG;
//		*pdwEffect = DROPEFFECT_NONE;

	}
//	/* アクティブにする */
//	ActivateFrameWindow( ::GetParent( m_hwndParent ) );
//	::PostMessage( ::GetParent( m_hwndParent ), WM_ACTIVATE, MAKELONG( 0, WA_ACTIVE ), 0 );
//	/*
//	||	処理中のユーザー操作を可能にする
	//	||	ブロッキングフック (メッセージ配送)
//	*/
//	BlockingHook();

	DragOver( dwKeyState, pt, pdwEffect );
	return S_OK;
}

STDMETHODIMP CEditView::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE( "CEditView::DragOver()\n" );
#endif
//	RECT	rc;
//	POINT	po;
//	po.x = pt.x;
//	po.y = pt.y;
//	::GetWindowRect(m_hWnd, &rc );
//	if( m_bDragSource && PtInRect( &rc, po ) ){
//	}else{
		/* マウス移動のメッセージ処理 */
//		::ScreenToClient( m_hWnd_DropTarget, (LPPOINT)&pt );
		::ScreenToClient( m_hWnd, (LPPOINT)&pt );
		OnMOUSEMOVE( dwKeyState, pt.x , pt.y );
//	}

//	MYTRACE( "m_nCaretPosX=%d, m_nCaretPosY=%d\n", m_nCaretPosX, m_nCaretPosY );

	if ( pdwEffect == NULL )
		return E_INVALIDARG;
//	::ScreenToClient( m_hWnd_DropTarget, (LPPOINT)&pt );
//	DWORD dwIndex = LOWORD( ::SendMessage( m_hWnd_DropTarget, EM_CHARFROMPOS, 0, MAKELPARAM( pt.x, pt.y ) ) );
//	if ( dwIndex != (WORD) -1 ){
//		::SendMessage( m_hWnd_DropTarget, EM_SETSEL, dwIndex, dwIndex );
//		::SendMessage( m_hWnd_DropTarget, EM_SCROLLCARET, 0, 0 );
//	}
	if( NULL == m_pcDropTarget->m_pDataObject ){
		*pdwEffect = DROPEFFECT_NONE;
	}else
	if( m_bDragSource
	 && 0 == IsCurrentPositionSelected( /* 指定カーソル位置が選択エリア内にあるか */
			m_nCaretPosX,				// カーソル位置X
			m_nCaretPosY				// カーソル位置Y
		)
	){
		*pdwEffect = DROPEFFECT_NONE;
	}else
	/* Ctrl,ALT,キーが押されていたか */
	if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL )
	 || FALSE == m_bDragSource
	){
		*pdwEffect = DROPEFFECT_COPY;
	}else{
		*pdwEffect = DROPEFFECT_MOVE;
	}
	return S_OK;
}



STDMETHODIMP CEditView::DragLeave( void )
{
#ifdef _DEBUG
	MYTRACE( "CEditView::DragLeave()\n" );
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
	m_bDragMode = FALSE;

	return S_OK;
}

STDMETHODIMP CEditView::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE( "CEditView::Drop()\n" );
#endif
	BOOL		bBoxSelected;
	BOOL		bMove;
	BOOL		bMoveToPrev;
	RECT		rcSel;
	int			nCaretPosX_Old;
	int			nCaretPosY_Old;
	CMemory		cmemBuf;
	CMemory		cmemClip;
	int			bBeginBoxSelect_Old;
//	int			nSelectLineBgn_Old;			/* 範囲選択開始行(原点) */
//	int			nSelectColBgn_Old;			/* 範囲選択開始桁(原点) */
	int			nSelectLineBgnFrom_Old;		/* 範囲選択開始行(原点) */
	int			nSelectColBgnFrom_Old;		/* 範囲選択開始桁(原点) */
	int			nSelectLineBgnTo_Old;		/* 範囲選択開始行(原点) */
	int			nSelectColBgnTo_Old;		/* 範囲選択開始桁(原点) */
	int			nSelectLineFrom_Old;
	int			nSelectColFrom_Old;
	int			nSelectLineTo_Old;
	int			nSelectColTo_Old;
//	MYTRACE( "CEditView::Drop()\n" );

	if( !m_bDragSource
	 && IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( TRUE );
	}
	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;
	*pdwEffect = DROPEFFECT_NONE;
	if( IsDataAvailable(pDataObject, CF_TEXT)){
#ifdef _DEBUG
		MYTRACE( "TRUE == IsDataAvailable()\n" );
#endif
		HGLOBAL		hData = GetGlobalData(pDataObject, CF_TEXT);
#ifdef _DEBUG
		MYTRACE( "%xh == GetGlobalData(pDataObject, CF_TEXT)\n", hData );
#endif
		if (hData == NULL){
			m_pcDropTarget->m_pDataObject = NULL;
			/* 選択テキストのドラッグ中か */
			m_bDragMode = FALSE;
			return E_INVALIDARG;
		}

		DWORD		nSize = 0;
		LPCTSTR lpszSource = (LPCTSTR) ::GlobalLock(hData);

//		MYTRACE( "lpszSource=[%s]\n", lpszSource );

		/* 移動かコピーか */
		if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL )
		 || FALSE == m_bDragSource
		){
			bMove = FALSE;
			*pdwEffect = DROPEFFECT_COPY;
		}else{
			bMove = TRUE;
			*pdwEffect = DROPEFFECT_MOVE;
		}

		if( m_bDragSource ){
			if( NULL != m_pcOpeBlk ){
			}else{
				m_pcOpeBlk = new COpeBlk;
			}
			bBoxSelected = m_bBeginBoxSelect;
			/* 選択範囲のデータを取得 */
//			GetSelectedData( cmemBuf, FALSE, NULL, FALSE );
//			cmemBuf.SetData( lpszSource, lstrlen( lpszSource ) );
			cmemBuf.SetDataSz( lpszSource );

			/* 移動の場合、位置関係を算出 */
			if( bMove ){
				if( bBoxSelected ){
					/* 2点を対角とする矩形を求める */
					TwoPointToRect(
						&rcSel,
						m_nSelectLineFrom,		/* 範囲選択開始行 */
						m_nSelectColmFrom,		/* 範囲選択開始桁 */
						m_nSelectLineTo,		/* 範囲選択終了行 */
						m_nSelectColmTo			/* 範囲選択終了桁 */
					);
					++rcSel.bottom;
					if( m_nCaretPosY >= rcSel.bottom ){
						bMoveToPrev = FALSE;
					}else
					if( m_nCaretPosY + rcSel.bottom - rcSel.top < rcSel.top ){
						bMoveToPrev = TRUE;
					}else
					if( m_nCaretPosX < rcSel.left ){
						bMoveToPrev = TRUE;
					}else{
						bMoveToPrev = FALSE;
					}
				}else{
					if( m_nSelectLineFrom > m_nCaretPosY ){
						bMoveToPrev = TRUE;
					}else
					if( m_nSelectLineFrom == m_nCaretPosY ){
						if( m_nSelectColmFrom > m_nCaretPosX ){
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
				DisableSelectArea( TRUE );
			}
			nCaretPosX_Old = m_nCaretPosX;
			nCaretPosY_Old = m_nCaretPosY;
			if( bMove ){
				if( bMoveToPrev ){
					/* 移動モード & 前に移動 */
					/* 選択エリアを削除 */
					DeleteData( TRUE );
					MoveCursor( nCaretPosX_Old, nCaretPosY_Old, TRUE );
				}else{
					bBeginBoxSelect_Old = m_bBeginBoxSelect;
//					nSelectLineBgn_Old	= m_nSelectLineBgn;			/* 範囲選択開始行(原点) */
//					nSelectColBgn_Old	= m_nSelectColmBgn;			/* 範囲選択開始桁(原点) */

					nSelectLineBgnFrom_Old	= m_nSelectLineBgnFrom;	/* 範囲選択開始行(原点) */
					nSelectColBgnFrom_Old	= m_nSelectColmBgnFrom;	/* 範囲選択開始桁(原点) */
					nSelectLineBgnTo_Old	= m_nSelectLineBgnTo;	/* 範囲選択開始行(原点) */
					nSelectColBgnTo_Old		= m_nSelectColmBgnTo;	/* 範囲選択開始桁(原点) */

					nSelectLineFrom_Old	= m_nSelectLineFrom;
					nSelectColFrom_Old	= m_nSelectColmFrom;
					nSelectLineTo_Old	= m_nSelectLineTo;
					nSelectColTo_Old	= m_nSelectColmTo;
					/* 現在の選択範囲を非選択状態に戻す */
					DisableSelectArea( TRUE );
				}
			}
			if( FALSE == bBoxSelected ){	/* 矩形範囲選択中 */
				Command_INSTEXT( TRUE, cmemBuf.GetPtr( NULL ), FALSE );
			}else{

				cmemClip.SetDataSz( "" );

				/* クリップボードからデータを取得 */
				BOOL	bBoxSelectOld;
				MyGetClipboardData( cmemClip, &bBoxSelectOld );

					/* クリップボードにデータを設定 */
					MySetClipboardData( cmemBuf.GetPtr( NULL ), cmemBuf.GetLength(), TRUE );

					/* 貼り付け（クリップボードから貼り付け）*/
					Command_PASTEBOX();

				/* クリップボードにデータを設定 */
				MySetClipboardData( cmemClip.GetPtr( NULL ), cmemClip.GetLength(), bBoxSelectOld );
			}
			if( bMove ){
				if( bMoveToPrev ){
				}else{
					/* 移動モード & 後ろに移動*/
					m_bBeginBoxSelect = bBeginBoxSelect_Old;
//					m_nSelectLineBgn = nSelectLineBgn_Old;			/* 範囲選択開始行(原点) */
//					m_nSelectColmBgn = nSelectColBgn_Old;			/* 範囲選択開始桁(原点) */
					m_nSelectLineBgnFrom = nSelectLineBgnFrom_Old;	/* 範囲選択開始行(原点) */
					m_nSelectColmBgnFrom = nSelectColBgnFrom_Old;	/* 範囲選択開始桁(原点) */
					m_nSelectLineBgnTo = nSelectLineBgnTo_Old;		/* 範囲選択開始行(原点) */
					m_nSelectColmBgnTo = nSelectColBgnTo_Old;		/* 範囲選択開始桁(原点) */

					m_nSelectLineFrom = nSelectLineFrom_Old;
					m_nSelectColmFrom = nSelectColFrom_Old;
					m_nSelectLineTo = nSelectLineTo_Old;
					m_nSelectColmTo = nSelectColTo_Old;

					/* 選択エリアを削除 */
					DeleteData( TRUE );
					MoveCursor( nCaretPosX_Old, nCaretPosY_Old, TRUE );
				}
			}
			/* アンドゥバッファの処理 */
			if( NULL != m_pcOpeBlk ){
				if( 0 < m_pcOpeBlk->GetNum() ){	/* 操作の数を返す */
					/* 操作の追加 */
					m_pcEditDoc->m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
				}else{
					delete m_pcOpeBlk;
				}
				m_pcOpeBlk = NULL;
			}
		}else{
			HandleCommand( F_INSTEXT, TRUE, (LPARAM)lpszSource, TRUE, 0, 0 );
		}
		::GlobalUnlock(hData);
	}else{
#ifdef _DEBUG
		MYTRACE( "FALSE == IsDataAvailable()\n" );
#endif
	}
	m_pcDropTarget->m_pDataObject = NULL;
//	::SetFocus(NULL);

	/* 選択テキストのドラッグ中か */
	m_bDragMode = FALSE;

	/* 編集ウィンドウオブジェクトからのアクティブ要求 */
	::SetFocus( ::GetParent( m_hwndParent ) );
	SetActiveWindow( m_hWnd );
//	::PostMessage( m_pShareData->m_hwndTray, MYWM_ACTIVATE_ME, (WPARAM)::GetParent( m_hwndParent ),  0 );

//	::ShowCaret( m_hWnd );
	return S_OK;
}





/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
void CEditView::GetCurrentTextForSearch( CMemory& cmemCurText )
{

	int				i;
	char			szTopic[_MAX_PATH];
//	CMemory			cmemCurText;
	const char*		pLine;
	int				nLineLen;
	int				nIdx;
	int				nLineFrom;
	int				nColmFrom;
	int				nLineTo;
	int				nColmTo;
	const CLayout*	pcLayout;

	cmemCurText.SetDataSz( "" );
	szTopic[0] = '\0';
	if( IsTextSelected() ){	/* テキストが選択されているか */
		/* 選択範囲のデータを取得 */
		if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE ) ){
			/* 検索文字列を現在位置の単語で初期化 */
			strncpy( szTopic, cmemCurText.GetPtr( NULL ), _MAX_PATH - 1 );
			szTopic[_MAX_PATH - 1] = '\0';
		}
	}else{
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY, &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* 指定された桁に対応する行のデータ内の位置を調べる */
			nIdx = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );

			/* 現在位置の単語の範囲を調べる */
			if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
				m_nCaretPosY, nIdx,
				&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
			){
				/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen );
				nColmFrom = LineIndexToColmn( pLine, nLineLen, nColmFrom );
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen );
				nColmTo = LineIndexToColmn( pLine, nLineLen, nColmTo );
				/* 選択範囲の変更 */
//				m_nSelectLineBgn = nLineFrom;		/* 範囲選択開始行(原点) */
//				m_nSelectColmBgn = nColmFrom;		/* 範囲選択開始桁(原点) */

				m_nSelectLineBgnFrom = nLineFrom;	/* 範囲選択開始行(原点) */
				m_nSelectColmBgnFrom = nColmFrom;	/* 範囲選択開始桁(原点) */
				m_nSelectLineBgnTo = nLineTo;		/* 範囲選択開始行(原点) */
				m_nSelectColmBgnTo = nColmTo;		/* 範囲選択開始桁(原点) */

				m_nSelectLineFrom = nLineFrom;
				m_nSelectColmFrom = nColmFrom;
				m_nSelectLineTo = nLineTo;
				m_nSelectColmTo = nColmTo;
				/* 選択範囲のデータを取得 */
				if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE ) ){
					/* 検索文字列を現在位置の単語で初期化 */
					strncpy( szTopic, cmemCurText.GetPtr( NULL ), MAX_PATH - 1 );
					szTopic[MAX_PATH - 1] = '\0';
				}
				/* 現在の選択範囲を非選択状態に戻す */
				DisableSelectArea( FALSE );
			}
		}
	}

	/* 検索文字列は改行まで */
	for( i = 0; i < (int)lstrlen( szTopic ); ++i ){
		if( szTopic[i] == CR || szTopic[i] == LF ){
			szTopic[i] = '\0';
			break;
		}
	}
//	cmemCurText.SetData( szTopic, lstrlen( szTopic ) );
	cmemCurText.SetDataSz( szTopic );
	return;

}



/* カーソル行アンダーラインのON */
void CEditView::CaretUnderLineON( BOOL bDraw )
{
	if( FALSE == m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp ){
		return;
	}

	if( IsTextSelected() ){	/* テキストが選択されているか */
		return;
	}
	m_nOldUnderLineY = m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight;
	if( -1 == m_nOldUnderLineY ){
		m_nOldUnderLineY = -2;
	}

	if( bDraw
	 && m_bDrawSWITCH
	 && m_nOldUnderLineY >=m_nViewAlignTop
	 && m_bDoing_UndoRedo == FALSE	/* アンドゥ・リドゥの実行中か */
	){
//		MYTRACE( "★カーソル行アンダーラインの描画\n" );
		/* ★カーソル行アンダーラインの描画 */
		HDC		hdc;
		HPEN	hPen, hPenOld;
		hdc = ::GetDC( m_hWnd );
		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx(
			hdc,
			m_nViewAlignLeft,
			m_nOldUnderLineY,
			NULL
		);
		::LineTo(
			hdc,
			m_nViewCx + m_nViewAlignLeft,
			m_nOldUnderLineY
		);
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
		::ReleaseDC( m_hWnd, hdc );
		hdc= NULL;
	}
	return;
}



/* カーソル行アンダーラインのOFF */
void CEditView::CaretUnderLineOFF( BOOL bDraw )
{
	if( FALSE == m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp ){
		return;
	}

//	if( IsTextSelected() ){	/* テキストが選択されているか */
//		m_nOldUnderLineY = -1;
//		return;
//	}

	if( -1 != m_nOldUnderLineY ){
		if( bDraw
		 && m_bDrawSWITCH
		 && m_nOldUnderLineY >=m_nViewAlignTop
		 && m_bDoing_UndoRedo == FALSE	/* アンドゥ・リドゥの実行中か */
		){
//			MYTRACE( "★カーソル行アンダーラインの消去\n" );
			/* カーソル行アンダーラインの消去 */
			HDC		hdc;
			HPEN	hPen, hPenOld;
			hdc = ::GetDC( m_hWnd );
			hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
			hPenOld = (HPEN)::SelectObject( hdc, hPen );
			::MoveToEx(
				hdc,
				m_nViewAlignLeft,
				m_nOldUnderLineY,
				NULL
			);
			::LineTo(
				hdc,
				m_nViewCx + m_nViewAlignLeft,
				m_nOldUnderLineY
			);
			::SelectObject( hdc, hPenOld );
			::DeleteObject( hPen );
			::ReleaseDC( m_hWnd, hdc );
		}
		m_nOldUnderLineY = -1;
	}
	return;
}


/* 現在、Enterなどで挿入する改行コードの種類を取得 */
CEOL CEditView::GetCurrentInsertEOL( void )
{
#if 0
	//	May 12, 2000 genta
	CDocLine* pFirstlineinfo = m_pcEditDoc->m_cDocLineMgr.GetLineInfo(0);
	if( pFirstlineinfo != NULL ){
		enumEOLType t = pFirstlineinfo->m_cEol;
		if( t != EOL_NONE && t != EOL_UNKNOWN )
			return t;
	}
	return EOL_CRLF;

	CEditDoc::FileReadにてコードを設定するようにしたので，ここは削除．
#endif
#if 0
	enumEOLType t = m_pcEditDoc->GetNewLineCode();
	char buf[30];
	wsprintf( buf, "EOL: %d", t );
	::MessageBox( NULL, buf, "End of Line", MB_OK );
	return t;
#else
	return m_pcEditDoc->GetNewLineCode();
#endif
}


// 子プロセスの標準出力をリダイレクトする
void CEditView::ExecCmd(const char* pszCmd, BOOL bGetStdout )
{
	HANDLE  hFile;
	char	szTempFile[_MAX_PATH+ 1];

	hFile = NULL;
	if( bGetStdout ){
		::GetTempPath( sizeof( szTempFile ) - 1, szTempFile );
		// テンポラリファイルのファイル名を作成します。
		::GetTempFileName(
		  szTempFile,	// pointer to directory name for temporary file
		  "skr",		// pointer to filename prefix
		  0,			// number used to create temporary filename
		  szTempFile	// pointer to buffer that receives the new filename
		);
//		MYTRACE( "szTempFile=[%s]\n", szTempFile );

		hFile = CreateFile(
			szTempFile,
			GENERIC_WRITE | GENERIC_READ,
			0,
			NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if ( hFile == INVALID_HANDLE_VALUE ){
			return;
		}
	}

	STARTUPINFO				StartupInfo;
	PROCESS_INFORMATION		ProcessInfo;
	char					szCmd[512];
	CDlgCancel				cDlgCancel;
	if( bGetStdout ){
		cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_EXECRUNNING );
	}

	GetStartupInfo(&StartupInfo);
	StartupInfo.dwFlags |= ( STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW );
	StartupInfo.wShowWindow = bGetStdout?/*SW_SHOW*/SW_HIDE:SW_SHOW;	/* 子プロセスのウィンドウ表示状態 */
	if( bGetStdout ){
		StartupInfo.hStdOutput	= hFile;
		StartupInfo.hStdError	= hFile;
	}
	lstrcpy(szCmd, "");
	lstrcat(szCmd, pszCmd);
	const char* pszOpt_C;
	if( bGetStdout ){
		pszOpt_C = "/C";
	}else{
		pszOpt_C = "/K";
	}
	memset( &ProcessInfo, 0, sizeof( PROCESS_INFORMATION ) );
	if( !CreateProcess( NULL, szCmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcessInfo ) ){
		wsprintf( szCmd, "command.com %s ", pszOpt_C );  // Win95/98 なら command.com
		lstrcat(szCmd, pszCmd);
		if( !CreateProcess( NULL, szCmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcessInfo ) ){
			wsprintf(szCmd, "cmd.exe %s ", pszOpt_C );  // Win NT なら cmd.exe
			lstrcat(szCmd, pszCmd);
			if( !CreateProcess( NULL, szCmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcessInfo ) ){
				goto end_of_func;
			}
		}
	}
	if( bGetStdout ){
		/* 最後にテキストを追加 */
//		Command_ADDTAIL( "\r\n", 2 );
//		Command_ADDTAIL( szCmd, strlen( szCmd ) );
//		Command_ADDTAIL( "\r\n", 2 );

		m_cShareData.TraceOut( "%s", "\r\n" );
		m_cShareData.TraceOut( "%s", szCmd );
		m_cShareData.TraceOut( "%s", "\r\n" );

	}
	if( bGetStdout ){
		::SetDlgItemText( cDlgCancel.m_hWnd, IDC_STATIC_CMD, szCmd );
	}

	DWORD	dwRes;
	DWORD	swFPOld;
	DWORD	swFPRead;
	DWORD	dwNumberOfBytesRead;
//	DWORD	dwNumberOfBytesWritten;
	swFPRead = 0;
	BOOL	bRes;
	char	szBuffer[1000];
	/* プロセスオブジェクトがシグナル状態になるまでループする */
	while(bGetStdout){
		/* プロセスオブジェクトの状態を調べる */
		dwRes = ::WaitForSingleObject(ProcessInfo.hProcess, 0 );
		// OutputDebugString( "WaitForSingleObject()\n" );

		if( bGetStdout ){
			/* ファイルポインタを移動します */
			swFPOld = SetFilePointer(
				hFile,						// handle of file
				swFPRead,					// number of bytes to move file pointer
				NULL,						// pointer to high-order word of distance to move
				FILE_BEGIN					// how to move
			);
			while(1){
				bRes = ReadFile(
					hFile,					// handle of file to read
					szBuffer,				// pointer to buffer that receives data
					sizeof( szBuffer ) - 1,	// number of bytes to read
					&dwNumberOfBytesRead,	// pointer to number of bytes read
					NULL					// pointer to structure for data
				);
				swFPRead += dwNumberOfBytesRead;
				szBuffer[dwNumberOfBytesRead] = '\0';
				if( 0 < dwNumberOfBytesRead ){
					/* 最後にテキストを追加 */
//					Command_ADDTAIL( szBuffer, dwNumberOfBytesRead );
//					/*ファイルの最後に移動 */
//					Command_GOFILEEND(FALSE);

					m_cShareData.TraceOut( "%s", szBuffer );

				}

				if( dwNumberOfBytesRead < sizeof( szBuffer ) - 1 ){
					break;
				}
			}

//			/* ファイルポインタを移動します */
//			SetFilePointer(
//				hFile,					// handle of file
//				0/*swFPOld*/,			// number of bytes to move file pointer
//				NULL,					// pointer to high-order word of distance to move
//				FILE_END/*FILE_BEGIN*/	// how to move
//			);
		}

		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( cDlgCancel.m_hWnd ) ){
			break;
		}
		/* 中断ボタン押下チェック */
		if( cDlgCancel.IsCanceled() ){
			//指定されたプロセスと、そのプロセスが持つすべてのスレッドを終了させます
			::TerminateProcess(
				ProcessInfo.hProcess,	// handle to the process
				0						// exit code for the process
			);
			/* 最後にテキストを追加 */
			const char*		pszText;
			pszText = "\r\n中断しました。\r\n";
//			Command_ADDTAIL( pszText, strlen( pszText ) );
//			/*ファイルの最後に移動 */
//			Command_GOFILEEND( FALSE );

			m_cShareData.TraceOut( "%s", pszText );

			break;
		}

		/* プロセスオブジェクトがシグナル状態 */
		if( WAIT_OBJECT_0 == dwRes ){
			break;
		}
	}
end_of_func:;
	if( bGetStdout ){
		if( NULL != ProcessInfo.hThread ){
			CloseHandle( ProcessInfo.hThread );
		}
		if( NULL != ProcessInfo.hProcess ){
			CloseHandle( ProcessInfo.hProcess );
		}
		cDlgCancel.CloseDialog( 0 );
	}
	if( NULL != hFile ){
		::CloseHandle( hFile );
		::DeleteFile( szTempFile );
	}

	return;
}


/*[EOF]*/
